#if defined USE_MAP

#include <stdlib.h>
#include <stdarg.h>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#include "MapDialog_spcific.cxx"
#include "MapDialogMkPDU.cxx"
#include "../status.h"
using namespace smsc::system;

static const bool SMS_SEGMENTATION = true;
static inline unsigned GetMOLockTimeout() {return 45;}

struct XMOMAPLocker {
  string imsi;
  unsigned long startTime;
  unsigned parts;
  unsigned ref;
};
typedef map<string,XMOMAPLocker> XMOMAP;

static Mutex x_map_lock;
typedef multimap<string,unsigned> X_MAP;
static X_MAP x_map;
static Mutex ussd_map_lock;
typedef map<long long,unsigned> USSD_MAP; 
static USSD_MAP ussd_map;
static XMOMAP x_momap;

static void ContinueImsiReq(MapDialog* dialog,const string& s_imsi,const string& s_msc);
static void PauseOnImsiReq(MapDialog* map);
static const string& SC_ADDRESS() { static string s("79029869999"); return s;}
static bool NeedNotifyHLR(MapDialog* dialog);
static void ResponseAlertSC(MapDialog* dialog);
static void SendErrToSmsc(unsigned dialogid,unsigned code);

static void AbortMapDialog(unsigned dialogid,unsigned ssn)
{
  Et96MapUAbortReq(ssn,dialogid,0,0,0,0);
}

string ImsiToString(const ET96MAP_IMSI_T* imsi)
{
  unsigned n = imsi->imsiLen;
  ostringstream ost;
  unsigned i = 0;
  for ( ;i < n; ++i ) { 
    unsigned x0 = ((unsigned int)imsi->imsi[i])&0x0f;
    unsigned x1 = (((unsigned int)imsi->imsi[i])>>4)&0x0f;
    if ( x0 <= 9 ) ost << x0;
    else break;
    if ( x1 <= 9 ) ost << x1;
    else break;
  }
  string result = ost.str();
  __trace2__("IMSI: %s",result.c_str());
  return result;
}

string MscToString(const ET96MAP_ADDRESS_T* msc)
{
  unsigned bytes = (msc->addressLength+1)/2;  
  ostringstream ost;
  for ( unsigned i=0; i<bytes; ++i) {
    unsigned x0 = ((unsigned int)msc->address[i])&0x0f;
    unsigned x1 = (((unsigned int)msc->address[i])>>4)&0x0f;
    if ( x0 <= 9 ) ost << x0;
    else break;
    if ( x1 <= 9 ) ost << x1;
    else break;
  }
  string result = ost.str();
  __trace2__("MSC: %s",result.c_str());
  return result;
}

static string FormatText(const char* format,...)
{
  auto_ptr<char> b(new char[1024]);
  memset(b.get(),0,1024);
  va_list arg;
  va_start(arg,format);
  vsnprintf(b.get(),1024-1,format,arg);
  va_end(arg);
  return string(b.get());
}

#define MAP_ERRORS_BASE 0x480
#define MAP_FALURE (/*MAP_ERRORS_BASE+34*/8)

struct MAPDIALOG_ERROR : public runtime_error
{
  unsigned code;
  MAPDIALOG_ERROR(unsigned code,const string& s) :
    runtime_error(s),code(code){}
  MAPDIALOG_ERROR(const string& s) :
    runtime_error(s),code(MAKE_ERRORCODE(CMD_ERR_TEMP,0)){}
};
struct MAPDIALOG_BAD_STATE : public MAPDIALOG_ERROR
{
  MAPDIALOG_BAD_STATE(const string& s) :
    MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_FATAL,0),s){}
};
struct MAPDIALOG_TEMP_ERROR : public MAPDIALOG_ERROR
{
  MAPDIALOG_TEMP_ERROR(const string& s,unsigned c=MAP_FALURE) :
    MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_TEMP,c),s){}
};
struct MAPDIALOG_FATAL_ERROR : public MAPDIALOG_ERROR
{
  MAPDIALOG_FATAL_ERROR(const string& s,unsigned c=MAP_FALURE) :
    MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_FATAL,c),s){}
};
struct MAPDIALOG_HEREISNO_ID : public MAPDIALOG_ERROR
{
  MAPDIALOG_HEREISNO_ID(const string& s,unsigned c=MAP_FALURE) :
    MAPDIALOG_ERROR(0,s){}
};

static void CloseMapDialog(unsigned dialogid,unsigned dialog_ssn){
  if ( dialogid == 0 ) return;
  USHORT_T res = Et96MapCloseReq (dialog_ssn,dialogid,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __trace2__("MAP::%s dialog 0x%x error, code 0x%hx",__FUNCTION__,dialogid,res);
  }else{
    __trace2__("MAP::%s dialog 0x%x closed",__FUNCTION__,dialogid);
  }
}

static void TryDestroyDialog(unsigned,bool send_error=false,unsigned err_code=0);
static string RouteToString(MapDialog*);

static void QueryHlrVersion(MapDialog*);
static void QueryMcsVersion(MapDialog*);

static ET96MAP_SS7_ADDR_T* GetScAddr()
{
  static ET96MAP_ADDRESS_T m_scAddr;
  static ET96MAP_SS7_ADDR_T scAddr;
  static bool initialized = false;
  if ( !initialized ) {
    mkMapAddress( &m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
    mkSS7GTAddress( &scAddr, &m_scAddr, 8 );
    initialized = true;
  }
  return &scAddr;
}

extern void MAPIO_TaskACVersionNotifier();
void MAPIO_QueryMscVersionInternal()
{
  USHORT_T result = 
    Et96MapGetACVersionReq(SSN,GetScAddr(),ET96MAP_SHORT_MSG_MT_RELAY);
  if ( result != ET96MAP_E_OK ) {
    throw runtime_error(FormatText("MAP::QueryMcsVersion: error 0x%x when GetAcVersion",result));
  }
}

static void StartDialogProcessing(MapDialog* dialog,const SmscCommand& cmd)
{
  __trace2__("MAP::%s: ",__FUNCTION__);
  dialog->dropChain = false;
  if ( !dialog->isQueryAbonentStatus ) {
    __trace2__("MAP:%s: Preapre SMSC command",__FUNCTION__);
    dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
    __trace2__("MAP::%s:DELIVERY_SM %s",__FUNCTION__,RouteToString(dialog).c_str());
    mkMapAddress( &dialog->m_msAddr, dialog->sms->getDestinationAddress().value, dialog->sms->getDestinationAddress().length );
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
  }else{
    __trace2__("MAP:%s: Preapre Abonent Status command",__FUNCTION__);
    AbonentStatus& as = dialog->QueryAbonentCommand->get_abonentStatus();
    __trace2__("MAP::%s: (%d.%d.%s)",__FUNCTION__,(unsigned)as.addr.type,(unsigned)as.addr.plan,as.addr.value);
    mkMapAddress( &dialog->m_msAddr, as.addr.value, as.addr.length );
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
  }
  __trace2__("MAP::%s: Query HLR AC version",__FUNCTION__);
  dialog->state = MAPST_WaitHlrVersion;
  QueryHlrVersion(dialog);
}

static void NotifyHLR(MapDialog* dialog);
static void MAPIO_PutCommand(const SmscCommand& cmd, MapDialog* dialog2=0 );

static void DropMapDialog_(unsigned dialogid){
  if ( dialogid == 0 ) return;
  DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid));
  if ( dialog.isnull() ) {
    __trace2__( ":MAP:%s: <ERROR> dialog(0x%x) is null",__FUNCTION__,dialogid);
    return;
  }
  __trace2__(":MAP:%s chain size %d , dropChain %d",__FUNCTION__,dialog->chain.size(),dialog->dropChain);
  
  if ( !dialog.isnull() ){
    {
      //MutexGuard(dialog->mutex);
      __trace2__("MAP::%s: 0x%x  (state CLOSED/ABORTED) /%d/",__FUNCTION__,dialog->dialogid_map,dialog->chain.size());
      unsigned __dialogid_map = dialog->dialogid_map;
      unsigned __dialogid_smsc = 0;
      if ( dialog->chain.size() == 0 ) {
        __trace2__("MAP::%s (state %d) associate:%p",__FUNCTION__,dialog->state,dialog->associate);
        if ( dialog->associate != 0 && dialog->state != MAPST_END )
        {
          //Et96MapPAbortInd(SSN,dialog->associate->dialogid_map,0,0,0);
          ContinueImsiReq(dialog->associate,"","");
        }
        else if ( NeedNotifyHLR(dialog.get()) )
        {
          try{
            dialog->Clean();
            NotifyHLR(dialog.get());          
            return; // do not drop dialog!
          }catch(exception& e){
            // drop dialog
            AbortMapDialog(dialog->dialogid_map,dialog->ssn);
            __trace2__("MAP::%s <exception> %s",__FUNCTION__,e.what());
          }
        }
        //MapDialogContainer::getInstance()->dropDialog(dialog->dialogid_map);
        //__trace2__("MAP::%s: 0x%x - closed and droped - ",__FUNCTION__,dialog->dialogid_map);
        //if ( dialog
        //return;
        goto dropDialogLabel;
      }
    }
    __trace2__("MAP::%s: - restart on next in chain - ",__FUNCTION__);
    if ( dialog->dropChain ) {
      //MutexGuard(dialog->mutex);
dropDialogLabel:
      MapDialogContainer::getInstance()->dropDialog(dialog->dialogid_map);
      for (;!dialog->chain.empty();dialog->chain.pop_front())
      {
        try{
          SmscCommand cmd = dialog->chain.front();
          SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,0));
        }catch(exception& e){
          __trace2__("MAP::%s: exception %s",__FUNCTION__,e.what());
        }catch(...){
        }
      }
      __trace2__("MAP::%s: 0x%x - closed and droped - ",__FUNCTION__,dialogid);
    }else{
      SmscCommand cmd = dialog->chain.front();
      dialog->chain.pop_front();
      dialog->Clean();
      MAPIO_PutCommand(cmd, dialog.get());
    }
  }
}

static void DropMapDialog(MapDialog* dialog){
  DropMapDialog_(dialog->dialogid_map);
}

static unsigned RemapDialog(MapDialog* dialog,unsigned ssn){
  return MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,ssn);
}

static void SendRescheduleToSmsc(unsigned dialogid)
{
  if ( dialogid == 0 ) return;
  __trace2__("Send RESCHEDULE NOW to SMSC");
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,0));
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __trace2__("Send RESCHEDULE NOW to SMSC OK");
}

static void SendAbonentStatusToSmsc(MapDialog* dialog,int status/*=AbonentStatus::UNKNOWN*/)
{
  __trace2__("MAP::%s: Send abonent status(%d) to SMSC ",__FUNCTION__,status);
  SmscCommand cmd = SmscCommand::makeQueryAbonentStatusResp(dialog->QueryAbonentCommand->get_abonentStatus(),status);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __trace2__("MAP::%s: Send abonent status to SMSC OK",__FUNCTION__);
}
                        
static void SendErrToSmsc(unsigned dialogid,unsigned code)
{
  if ( dialogid == 0 ) return;
  __trace2__("Send error 0x%x to SMSC",code);
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,code);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __trace2__("Send error to SMSC OK");
}

static void SendOkToSmsc(/*unsigned dialogid*/MapDialog* dialog)
{
  if ( dialog == 0 || dialog->dialogid_smsc == 0 ) {
    __trace2__("MAP::Send OK to SMSC: no smscid or no dialog exists");
    return;
  }
  __trace2__("MAP::Send OK to SMSC");
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialog->dialogid_smsc,0);
  Descriptor desc;
  desc.setImsi(dialog->s_imsi.length(),dialog->s_imsi.c_str());
  __trace2__("MAP::Send OK to SMSC: IMSI = %s",dialog->s_imsi.c_str());
  desc.setMsc(dialog->s_msc.length(),dialog->s_msc.c_str());
  __trace2__("MAP::Send OK to SMSC: MSC = %s",dialog->s_msc.c_str());
  cmd->get_resp()->setDescriptor(desc);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __trace2__("Send OK to SMSC done");
}

static void CheckLockedByMO(MapDialog* dialog)
{
  XMOMAP::iterator it = x_momap.find(dialog->s_imsi);
  if ( it != x_momap.end() )
  {
    __trace2__("MAP:UDHI:%s locked, recv(%d)",__FUNCTION__,it->second.parts);
    if ( it->second.startTime+GetMOLockTimeout() <= time(0) )
    {
      __trace2__("MAP:UDHI:%s time expired, unlocked",__FUNCTION__);
      x_momap.erase(it);
    }
    else{
      dialog->dropChain = true;
      __trace2__("MAP:UDHI:%s locked, reschedule NOW!",__FUNCTION__);
      throw MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,0),
                            "MAP:: Locked by MO: reschedule NOW!");
    }
  }
}

static void QueryHlrVersion(MapDialog* dialog)
{
  MutexGuard guard(x_map_lock);
  char text[32];
  SS7ToText(&dialog->mshlrAddr,text);
  string s_(text);
  __trace2__("MAP::QueryHlrVersion: [store %s]=0x%x",s_.c_str(),dialog->dialogid_map);
  x_map.insert(make_pair(s_,dialog->dialogid_map));
  USHORT_T result = 
    Et96MapGetACVersionReq(SSN,&dialog->mshlrAddr,ET96MAP_SHORT_MSG_GATEWAY_CONTEXT);
  __trace2__("MAP::QueryHlrVersion: done");
  if ( result != ET96MAP_E_OK ) {
    x_map.erase(s_);
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::QueryHlrVersion: error 0x%x when GetAcVersion",result),MAP_FALURE);
  }
}

static void QueryMcsVersion(MapDialog* dialog)
{
  MutexGuard guard(x_map_lock);
  char text[32];
  SS7ToText(&dialog->destMscAddr,text);
  string s_(text);
  __trace2__("MAP::QueryMcsVersion: [store %s]=0x%x",s_.c_str(),dialog->dialogid_map);
  x_map.insert(make_pair(s_,dialog->dialogid_map));
  USHORT_T result = 
    Et96MapGetACVersionReq(SSN,&dialog->destMscAddr,ET96MAP_SHORT_MSG_MT_RELAY);
  if ( result != ET96MAP_E_OK ) {
    x_map.erase(s_);
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::QueryMcsVersion: error 0x%x when GetAcVersion",result),MAP_FALURE);
  }
}

static inline 
void SetVersion(ET96MAP_APP_CNTX_T& ac,unsigned version){
  if ( version > 3 || version == 0 ) throw runtime_error(
    FormatText("MAP::%s: Opss, version = %d, why?",__FUNCTION__,version));
  switch(version){
  case 3:
  //  ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_3;
  //  break;
  case 2:
    ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;
    break;
  case 1:
    ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_1;
    break;
  }
}

static void SendRInfo(MapDialog* dialog)
{
  __trace2__("MAP::%s: MAP.did: 0x%x",__FUNCTION__,dialog->dialogid_map);
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
  SetVersion(appContext,dialog->version);
  //unsigned dialog_id = dialog->isMOreq?dialog->dialogid_req:dialog->dialogid_map;
  require ( dialog->ssn == SSN );
  unsigned dialog_id = dialog->dialogid_map;
  __trace2__("MAP::%s dialogid:0x%x ssn:%d",__FUNCTION__,dialog_id,dialog->ssn);
  USHORT_T result = Et96MapOpenReq(
    SSN, dialog_id, 
    &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendRInfo: Et96MapOpenReq error 0x%x",result),MAP_FALURE);
  }
  bool hiPrior = false;
  if ( !dialog->isQueryAbonentStatus && dialog->sms.get() != 0 && dialog->sms->getIntProperty(Tag::SMPP_PRIORITY) > 0 ) 
    hiPrior = true;
  if ( dialog->version == 2 ) {
    result = Et96MapV2SendRInfoForSmReq(dialog->ssn, dialog_id, 1, &dialog->m_msAddr, 
      hiPrior ? ET96MAP_ATTEMPT_DELIVERY : ET96MAP_DO_NOT_ATTEMPT_DELIVERY, 
      &dialog->m_scAddr );
  }else if ( dialog->version == 1 ) {
    result = Et96MapV1SendRInfoForSmReq(dialog->ssn, dialog_id, 1, &dialog->m_msAddr, 
      hiPrior ? ET96MAP_ATTEMPT_DELIVERY : ET96MAP_DO_NOT_ATTEMPT_DELIVERY, 
      &dialog->m_scAddr, 0, 0);
  }else throw runtime_error(
    FormatText("MAP::SendRInfo: incorrect dialog version %d",dialog->version));
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendRInfo: Et96MapVxSendRInfoForSmReq error 0x%x",result),MAP_FALURE);
  }
  __trace2__("MAP::SendRInfo: Et96MapVxSendRInfoForSmReq OK");
  result = Et96MapDelimiterReq(dialog->ssn, dialog_id, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendRInfo: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
  }
  __trace2__("MAP::SendRInfo: Et96MapOpenReq OK");
}

void ResponseMO(MapDialog* dialog,unsigned status)
{
  __trace2__("MAP::%s: MAP.did: 0x%x",__FUNCTION__,dialog->dialogid_map);
  ET96MAP_ERROR_FORW_SM_MO_T err ;
  memset(&err,0,sizeof(ET96MAP_ERROR_FORW_SM_MO_T));
  switch ( status )
  { 
  case Status::OK: break;  
  case Status::SYSERR:
    err.errorCode = 32;
    break;
  case Status::INVSRCADR:
  case Status::INVDSTADR:
  case Status::NOROUTE:
    err.errorCode = 5;
    break;
  case Status::INVSCHED:
    err.errorCode = 32;
    break;
  default:
    err.errorCode = 36;
    break;
  };
  __trace2__("MAP::%s err.errCode 0x%x (state %d) ",__FUNCTION__,err.errorCode,dialog->state);
  USHORT_T result; 
  if ( dialog->version == 2 ) {
    result = Et96MapV2ForwardSmMOResp(
      dialog->ssn,
      dialog->dialogid_map,
      dialog->invokeId,
      (status!=Status::OK)?&err:0);
  }else if ( dialog->version == 1 ) {
    result = Et96MapV1ForwardSmMOResp(
      dialog->ssn,
      dialog->dialogid_map,
      dialog->invokeId,
      (status!=Status::OK)?&err:0);
  }else throw runtime_error(
    FormatText("MAP::ResponseMO: incorrect dialog version %d",dialog->version));
  if ( result != ET96MAP_E_OK ) {
    __trace2__("MAP::ResponseMO: Et96MapVxForwardSmMOResp return error 0x%hx",result);
  }else{
    __trace2__("MAP::ResponseMO: Et96MapVxForwardSmMOResp OK");
  }
  unsigned INVALID = (unsigned)-1;
  if ( dialog->udhiRef != INVALID )
  {
    XMOMAPLocker* locker;
    __trace2__("MAP:UDHI:%s: find locker with imsi %s, ref %x",__FUNCTION__,dialog->s_imsi.c_str(),dialog->udhiRef);
    XMOMAP::iterator it = x_momap.find(dialog->s_imsi);
    if ( it == x_momap.end() )
    {
      __trace2__("MAP:UDHI:%s: create locker",__FUNCTION__);
      XMOMAPLocker lockerX;
      lockerX.imsi = dialog->s_imsi;
      lockerX.ref = INVALID;
      lockerX.parts = INVALID;
      locker = &x_momap[dialog->s_imsi];
      *locker = lockerX;
      //locker = x_momap[dialog->s_imsi];
    }
    else locker = &it->second;
    if ( locker->ref == dialog->udhiRef )
    {
      __trace2__("MAP:UDHI:%s: update locker part %d/%d",__FUNCTION__,dialog->udhiMsgNum,dialog->udhiMsgCount);
      ++locker->parts;
      if ( locker->parts >= dialog->udhiMsgCount ){
        x_momap.erase(dialog->s_imsi);
        __trace2__("MAP:UDHI:%s: unlock part ",__FUNCTION__);
      }else
        locker->startTime = time(0);
    }
    else
    {
      __trace2__("MAP:%s: update locker part %d/%d",__FUNCTION__,dialog->udhiMsgNum,dialog->udhiMsgCount);
      locker->ref = dialog->udhiRef;
      locker->parts = 1;
      locker->startTime = time(0);
      //locker->imsi 
    }
  }
}

static void AttachSmsToDialog(MapDialog* dialog,ET96MAP_SM_RP_UI_T *ud,ET96MAP_SM_RP_OA_T *srcAddr)
{
  __trace2__("MAP::%s: MAP.did: 0x%x",__FUNCTION__,dialog->dialogid_map);
  auto_ptr<SMS> _sms ( new SMS() );
  SMS& sms = *_sms.get();
  Address src_addr;
  Address dest_addr;
  SMS_SUMBMIT_FORMAT_HEADER* ssfh = (SMS_SUMBMIT_FORMAT_HEADER*)ud->signalInfo;
  __trace2__("MAP:: MR(8) = 0x%x",ssfh->mr);
  __trace2__("MAP:: MSG_TYPE_IND(2) = 0x%x",ssfh->mg_type_ind);
  __trace2__("MAP:: MSG_VPF(2) = 0x%x",ssfh->tp_vp);
  __trace2__("MAP:: MSG_UDHI(1) = %d",ssfh->udhi);
  __trace2__("MAP:: MSG_REJECT_DUPL(1) = %d",ssfh->reject_dupl);
  __trace2__("MAP:: MSG_REPLY_PATH(1) = %d",ssfh->reply_path);
  __trace2__("MAP:: MSG_SRR(1) = %d",ssfh->srr);
  MAP_SMS_ADDRESS* msa = (MAP_SMS_ADDRESS*)(ud->signalInfo+2);
  __trace2__("MAP:: MSA tonpi.va(8):0x%x, ton(3):0x%x, npi(4):0x%x, len(8):%d",
             msa->tonpi,msa->st.ton,msa->st.npi,msa->len);
  unsigned msa_len = msa->len/2+msa->len%2+2;
  unsigned char protocol_id = *(unsigned char*)(ud->signalInfo+2+msa_len);
  __trace2__("MAP:: protocol_id = 0x%x",protocol_id);
  unsigned char user_data_coding = *(unsigned char*)(ud->signalInfo+2+msa_len+1);
  __trace2__("MAP:: user_data_encoding = 0x%x",user_data_coding);
  unsigned char user_data_len = *(unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2);
  __trace2__("MAP:: user_data_len = %d",user_data_len);
  unsigned char* user_data = (unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2+1);
  unsigned encoding = 0;
  if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
       (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
  {
    if ( user_data_coding&(1<<5) ){
      __trace2__("MAP:: required compression");
      throw runtime_error("MAP:: required compression");
    }
    encoding = user_data_coding&0x0c;
    if ( (user_data_coding & 0xc0) == 0x40 )
      sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x03);
  }
  else if ( (user_data_coding & 0xf0) == 0xc0 ) // 1100xxxx
  {
    encoding = MAP_OCTET7BIT_ENCODING;
    sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x3);
    sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                       (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
  }
  else if ( (user_data_coding & 0xf0) == 0xd0 ) // 1101xxxx
  {
    encoding = MAP_OCTET7BIT_ENCODING;
    sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
    sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                       (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
  }
  else if ( (user_data_coding & 0xf0) == 0xe0 ) // 1110xxxx
  {
    encoding = MAP_UCS2_ENCODING;
    sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
    sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                       (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
  }
  else if ( (user_data_coding & 0xf0) == 0xf0 ) // 1111xxxx
  {
    if ( user_data_coding & 0x4 ) encoding = MAP_8BIT_ENCODING;
    else encoding = MAP_OCTET7BIT_ENCODING;
    sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,user_data_coding&0x3);
  }
  else{
    __trace2__("MAP:: unknown coding scheme 0x%x",user_data_coding);
    throw runtime_error("unknown coding scheme");
  }
  {
    if (  encoding == MAP_OCTET7BIT_ENCODING ){
      if ( ssfh->udhi){
        MicroString ms;
        auto_ptr<unsigned char> b(new unsigned char[255*2]);
        unsigned udh_len = ((unsigned)*user_data)&0x0ff;
        __trace2__("MAP:: ud_length 0x%x",user_data_len);
        __trace2__("MAP:: udh_len 0x%x",udh_len);
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = user_data_len-x/7;
        __trace2__("MAP:: text symbols 0x%x",symbols);
        __trace2__("MAP:: text bit offset 0x%x",x-(udh_len+1)*8);
        Convert7BitToSMSC7Bit(user_data+udh_len+1,symbols,&ms,x-(udh_len+1)*8);
        memcpy(b.get(),user_data,udh_len+1);
        memcpy(b.get()+udh_len+1,ms.bytes,ms.len);
        sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,(char*)b.get(),udh_len+1+symbols);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,udh_len+1+symbols);
      }else{
        MicroString ms;
        Convert7BitToSMSC7Bit(user_data,user_data_len,&ms,0);
        sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,ms.bytes,ms.len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
      }
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)MAP_SMSC7BIT_ENCODING);
    }
    else{
      sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,(const char*)user_data,user_data_len);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)encoding);
    }
  }
  {
    unsigned INVALID = (unsigned)-1;
    if ( ssfh->udhi )
    {
      unsigned ref = INVALID;
      unsigned msgNum = INVALID;
      unsigned msgCount = INVALID;

      unsigned len = ((unsigned)*user_data)&0x0ff;
      __trace2__(":MAP:UDHI: udh length %d",len);
      unsigned char* udh = user_data+1;
      unsigned ptr = 0;
      for (; ptr+2 < len; ptr+=2 )
      {
        unsigned elLength = udh[ptr+1];
        __trace2__(":MAP:UDHI: ptr %d, tag %d, len %d",ptr,udh[ptr],elLength);
        if ( udh[ptr] == 0 || udh[ptr] == 8)
        {
          if ( udh[ptr] == 0 ) {
            ref = udh[ptr+2];
            msgCount = udh[ptr+3];
            msgNum   = udh[ptr+4];
            __trace2__(":MAP:UDHI: tag 0, len %d",elLength);
            break;
          }else{
            //ref = ntohs(*(unsigned short*)(udh+ptr+2));
            unsigned short tmpVal;
            memcpy(&tmpVal,udh+ptr+2,2);
            ref = ntohs(tmpVal);
            msgCount = udh[ptr+4];
            msgNum   = udh[ptr+5];
            __trace2__(":MAP:UDHI: tag 8, len %d",elLength);
            break;
          }
        }
        else ptr+=elLength;
      }
      dialog->udhiRef = ref;
      dialog->udhiMsgNum = msgNum;
      dialog->udhiMsgCount = msgCount;
      __trace2__(":MAP:UDHI: ref %x, msgNum %d, msgCont %d ",ref,msgNum,msgCount);
    }
    else
    {
      dialog->udhiRef = INVALID;
      dialog->udhiMsgNum = INVALID;
      dialog->udhiMsgCount = INVALID;
    }
  }
  unsigned esm_class = 0;
  esm_class |= (ssfh->udhi?0x40:0);
  esm_class |= (ssfh->reply_path?0x80:0);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
  sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
  sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
  sms.setMessageReference(ssfh->mr);
  if ( ssfh->srr ) sms.setIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST,1);
  ConvAddrMSISDN2Smc(srcAddr,&src_addr);
  sms.setOriginatingAddress(src_addr);
  ConvAddrMap2Smc(msa,&dest_addr);
  sms.setDestinationAddress(dest_addr);
  dialog->sms = _sms;
}

static void SendSubmitCommand(MapDialog* dialog)
{
  __trace2__("MAP::%s: MAP.did: 0x%x",__FUNCTION__,dialog->dialogid_map);
  if ( dialog->sms.get() == 0 )
    throw runtime_error("MAP::hereis no SMS for submiting");
  Descriptor desc;
  desc.setImsi(dialog->s_imsi.length(),dialog->s_imsi.c_str());
  __trace2__("MAP::Send OK to SMSC: IMSI = %s",dialog->s_imsi.c_str());
  desc.setMsc(dialog->s_msc.length(),dialog->s_msc.c_str());
  __trace2__("MAP::Send OK to SMSC: MSC = %s",dialog->s_msc.c_str());
  dialog->sms->setOriginatingDescriptor(desc);
  if ( dialog->isUSSD ) {
    Address src_addr;
    ConvAddrMap2Smc((const MAP_SMS_ADDRESS*)&dialog->m_msAddr,&src_addr);
    dialog->sms->setOriginatingAddress(src_addr);
    istringstream(src_addr.value)>>dialog->ussdSequence;
    __trace2__("MAP::%s:USSD request %s",__FUNCTION__,RouteToString(dialog).c_str());
  }
  MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
  SmscCommand cmd = SmscCommand::makeSumbmitSm(
    *dialog->sms.get(),((uint32_t)dialog->dialogid_map)&0xffff);
  proxy->putIncomingCommand(cmd);
}

static const unsigned DIALOGID_BEFORE_CREATING = 0x10000;

static void TryDestroyDialog(unsigned dialogid,bool send_error = false,unsigned err_code=0)
{
  {
    __trace2__("MAP::TryDestroyDialog: dialog 0x%x , reason error",dialogid);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid));
    if ( dialog.isnull() ) {
      __trace2__("MAP::TryDestroyDialog: has no dialog 0x%x",dialogid);
      return;
    }
    __trace2__("MAP::%s: 0x%x  (state %d/TRY__DESTROY)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    if ( send_error )
    {
      dialog->dropChain = true;
      try{
        if ( dialog->isQueryAbonentStatus ){
          int status;
          status = AbonentStatus::UNKNOWNVALUE;
          SendAbonentStatusToSmsc(dialog.get(),status);
        }else{
          SendErrToSmsc(dialog->dialogid_smsc,err_code);
        }
      }catch(...){
        __trace2__("MAP::%s: catched exception when send error response to smsc",__FUNCTION__);
      }
    }
    __trace2__("MAP::%s: dialod state %d",__FUNCTION__,dialog->state);
    if ( send_error ){
      __trace2__("MAP::ABORT: dialog 0x%x",dialog->dialogid_map);
      AbortMapDialog(dialog->dialogid_map,dialog->ssn);
    }else{
      switch(dialog->state){
      case MAPST_ABORTED: 
        __trace2__("MAP::ABORT: dialog 0x%x",dialog->dialogid_map);
        AbortMapDialog(dialog->dialogid_map,dialog->ssn);
        break;
      default:
        CloseMapDialog(dialog->dialogid_map,dialog->ssn);
      }
    }
  }
  DropMapDialog_(dialogid);
}

static string RouteToString(MapDialog* dialog)
{
  static string skiped("<unknown> -> <unknown>");
#if !defined DIABLE_TRACING
  if ( dialog->sms.get() == 0 ) return skiped;
  auto_ptr<char> b(new char[1024]);
  memset(b.get(),0,1024);
  snprintf(b.get(),1023,"%d.%d.%s -> %d.%d.%s",
    dialog->sms->getOriginatingAddress().getTypeOfNumber(),
    dialog->sms->getOriginatingAddress().getNumberingPlan(),
    dialog->sms->getOriginatingAddress().value,
    dialog->sms->getDestinationAddress().getTypeOfNumber(),
    dialog->sms->getDestinationAddress().getNumberingPlan(),
    dialog->sms->getDestinationAddress().value);
  return string(b.get());
#else
  return skiped;
#endif
}

#define MAP_TRY  try {
#define MAP_CATCH(__dialogid_map,__dialogid_smsc)     \
}catch(MAPDIALOG_HEREISNO_ID& x){\
  __trace2__("#NOID#MAP::%s# MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,__dialogid_map,__dialogid_smsc);\
  __trace2__("   <exception>:%s",x.what());\
}catch(MAPDIALOG_ERROR& err){\
  __trace2__("#ERR#MAP::%s# MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,__dialogid_map,__dialogid_smsc);\
  __trace2__("   <exception>:%s",err.what());\
  TryDestroyDialog(__dialogid_map,true,err.code);\
}catch(exception& e){\
  __trace2__("#except#MAP::%s# MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,__dialogid_map,__dialogid_smsc);\
  __trace2__("   <exception>:%s",e.what());\
  TryDestroyDialog(__dialogid_map,true,MAKE_ERRORCODE(CMD_ERR_FATAL,0));\
}

static bool SendSms(MapDialog* dialog){
  __trace2__("MAP::%s: MAP.did: 0x%x",__FUNCTION__,dialog->dialogid_map);
  CheckLockedByMO(dialog);

  bool mms = dialog->chain.size() != 0;
  //if ( dialog->version < 2 ) mms = false;
  mms = false;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_MT_RELAY;
  SetVersion(appContext,dialog->version);
  USHORT_T result;
  bool segmentation = false;

  if ( mms )  { __trace2__("MAP::%s: MMS flag is set",__FUNCTION__); }
  __trace2__("MAP::%s: chain size is %d",__FUNCTION__,dialog->chain.size());

  if ( !dialog->mms ) {
    result = Et96MapOpenReq(SSN,dialog->dialogid_map,&appContext,&dialog->destMscAddr,&dialog->scAddr,0,0,0);
    if ( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::SendSms: Et96MapOpenReq error 0x%x",result),MAP_FALURE);
  }
  
  dialog->smRpOa.typeOfAddress = ET96MAP_ADDRTYPE_SCADDR;
  dialog->smRpOa.addrLen = (dialog->m_scAddr.addressLength+1)/2+1;
  dialog->smRpOa.addr[0] = dialog->m_scAddr.typeOfAddress;
  memcpy( dialog->smRpOa.addr+1, dialog->m_scAddr.address, (dialog->m_scAddr.addressLength+1)/2 );

  ET96MAP_SM_RP_UI_T* ui;
  dialog->auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(ui=new ET96MAP_SM_RP_UI_T);
  mkDeliverPDU(dialog->sms.get(),ui,mms);
  if ( ui->signalInfoLen > 98 || (mms && !dialog->mms) ) {
    __trace2__("MAP::SendSMSCToMT: Et96MapDelimiterReq");
    result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
    segmentation = true;
  }else{
    __trace2__("MAP::SendSMSCToMT: Et96MapVxForwardSmMTReq");
    if ( dialog->version == 2 ) {
      result = Et96MapV2ForwardSmMTReq( dialog->ssn, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), FALSE);
    }else if ( dialog->version == 1 ){
      result = Et96MapV1ForwardSmMT_MOReq( dialog->ssn, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get());
    }else throw runtime_error(
      FormatText("MAP::SendSMSCToMT: incorrect dialog version %d",dialog->version));
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT: Et96MapVxForwardSmMTReq error 0x%x",result),MAP_FALURE);
    __trace2__("MAP::SendSMSCToMT: Et96MapVxForwardSmMTReq OK");
    result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
    if( result != ET96MAP_E_OK ) 
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
  }
  if ( segmentation == SMS_SEGMENTATION )
    dialog->state = MAPST_WaitSpecOpenConf;
  else if ( !dialog->mms )
    dialog->state = MAPST_WaitOpenConf;
  else // mms continue
    dialog->state = MAPST_WaitSmsConf;
  dialog->mms = mms;
  return segmentation;
}

static void SendNextMMS(MapDialog* dialog)
{
  __trace2__("MAP::%s: 0x%x  (state %d/NEXTMMS)",__FUNCTION__,dialog->dialogid_map,dialog->state);
  if ( dialog->chain.size() == 0 ) 
    throw runtime_error("MAP::SendNextMMS: has no messages to send");
  SmscCommand cmd = dialog->chain.front();
  dialog->chain.pop_front();
  //dialog->
  dialog->dialogid_smsc = cmd->get_dialogId();
  dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
  SendSms(dialog);
}

static void SendSegmentedSms(MapDialog* dialog)
{
  USHORT_T result;
  CheckLockedByMO(dialog);
  __trace2__("MAP::SendSegmentedSms: Et96MapVxForwardSmMTReq");
  if ( dialog->version == 2 ) {
    result = Et96MapV2ForwardSmMTReq( dialog->ssn, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), dialog->mms);
  }else if ( dialog->version == 1 ) {
    result = Et96MapV1ForwardSmMT_MOReq( dialog->ssn, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get());
  }else throw runtime_error(
    FormatText("MAP::SendSegmentedSms: incorrect dialog version %d",dialog->version));
  if( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSegmentedSms: Et96MapVxForwardSmMTReq error 0x%x",result),MAP_FALURE);
  __trace2__("MAP::SendSegmentedSms: Et96MapVxForwardSmMTReq OK");
  result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
  if( result != ET96MAP_E_OK ) 
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSegmentedSms: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
}

static void DoUSSRUserResponceError(const SmscCommand& cmd , MapDialog* dialog)
{
  __trace2__("MAP::%s MAP.did:{0x%x}",__FUNCTION__,dialog->dialogid_map);
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0;
  ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_REQUEST_T error;
  error.errorCode = 34; /*Sytem failure */
  ET96MAP_USSD_STRING_T ussdString = {0,};
  //memset(&ussdString,0,sizeof(ussdString));
  UCHAR_T result;
  if ( dialog->version == 2 ) 
  {
    result = Et96MapV2ProcessUnstructuredSSRequestResp(
      dialog->ssn,dialog->dialogid_map,dialog->invokeId,
      &ussdEncoding,
      &ussdString,
      &error);
  }
  else throw runtime_error(
    FormatText("MAP::%s incorrect dialog version %d",__FUNCTION__,dialog->version));
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::%s Resp return error 0x%x",__FUNCTION__,result));
  CloseMapDialog(dialog->dialogid_map,dialog->ssn);
  dialog->state = MAPST_END;
  SendOkToSmsc(dialog);
  DropMapDialog(dialog);
}

static long long NextSequence()
{
  static long long sequence = (((long long)time(0))<<32);
  return ++sequence;
}

static void DoUSSRUserResponce(const SmscCommand& cmd , MapDialog* dialog)
{
  __trace2__("MAP::%s MAP.did:{0x%x}",__FUNCTION__,dialog->dialogid_map);
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0x0f;
  ET96MAP_USSD_STRING_T ussdString = {0,};
  unsigned text_len;
  const unsigned char* text = (const unsigned char*)cmd->get_sms()->getBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,&text_len);
  if ( text_len > 160 ) throw runtime_error(
    FormatText("MAP::%s MAP.did:{0x%x} very long string %d",__FUNCTION__,dialog->dialogid_map,text_len));
  unsigned bytes = 0;
  {
    unsigned elen = sizeof(ussdString.ussdStr);
    bytes = ConvertText27bit(text,text_len,ussdString.ussdStr,&elen);
  }
  ussdString.ussdStrLen = bytes;
  UCHAR_T result;
  if ( dialog->version == 2 ) 
  {
    result = Et96MapV2ProcessUnstructuredSSRequestResp(
      dialog->ssn,dialog->dialogid_map,dialog->invokeId,
      &ussdEncoding,
      &ussdString,
      0);
  }
  else throw runtime_error(
    FormatText("MAP::%s incorrect dialog version %d",__FUNCTION__,dialog->version));
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::%s Resp return error 0x%x",__FUNCTION__,result));
  CloseMapDialog(dialog->dialogid_map,dialog->ssn);
  dialog->state = MAPST_END;
  SendOkToSmsc(dialog);
  DropMapDialog(dialog);
}

void MAPIO_PutCommand(const SmscCommand& cmd ){ MAPIO_PutCommand(cmd, 0 ); }

static void MAPIO_PutCommand(const SmscCommand& cmd, MapDialog* dialog2=0 )
{
  //__trace2__("MAP::%s MAP.did:{0x%x}",__FUNCTION__,dialog->dialogid_map);
  unsigned dialogid_smsc = cmd->get_dialogId();
  unsigned dialogid_map = 0;
  __trace2__(">>MAPPROXY::putCommand dialog:0x%x (state:NONE)",dialogid_smsc);
  DialogRefGuard dialog;
  MAP_TRY {  
    if ( dialogid_smsc > 0xffff ) { // SMSC dialog
      if ( cmd->get_commandId() != DELIVERY && cmd->get_commandId() != QUERYABONENTSTATUS)
        throw MAPDIALOG_BAD_STATE("MAP::putCommand: must be DELIVERY or QUERYABONENTSTATUS");
      try
      {
        if ( cmd->get_commandId() == DELIVERY  && cmd->get_sms()->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP ) )
        {
          unsigned serviceOp = cmd->get_sms()->getIntProperty(Tag::SMPP_USSD_SERVICE_OP );
          string s_seq(cmd->get_sms()->getDestinationAddress().value);// = "";//cmd->get_sms()->getStrProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
          if (s_seq.length()==0) throw MAPDIALOG_FATAL_ERROR("MAP::PutCommand: empty user_message_reference");
          long long sequence;
          istringstream(s_seq) >> sequence;
          if ( sequence == 0 )
            throw MAPDIALOG_FATAL_ERROR(
              FormatText("MAP::PutCommand: invaid sequence %s",s_seq.c_str()));
          try 
          {
            if ( serviceOp == USSD_PSSR_RESP ) 
            {
              {
                MutexGuard ussd_map_guard( ussd_map_lock );
                USSD_MAP::iterator it = ussd_map.find(sequence);
                if ( it == ussd_map.end() )
                  throw MAPDIALOG_FATAL_ERROR(
                    FormatText("MAP::PutCommand: can't find session %s",s_seq.c_str()));
                dialogid_map = it->second;
              }
              dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_map));
              if ( dialog.isnull() )
                throw MAPDIALOG_FATAL_ERROR(
                  FormatText("MAP::putCommand: Opss, here is no dialog with id x%x seq: %s",dialogid_smsc,s_seq.c_str()));
              __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
              if ( !dialog->isUSSD )
                throw MAPDIALOG_FATAL_ERROR(
                  FormatText("MAP::putCommand: Opss, NO ussd dialog with id x%x, seq: %s",dialogid_smsc,s_seq.c_str()));
              if ( dialog->state != MAPST_USSDWaitResponce )
                throw MAPDIALOG_BAD_STATE(
                  FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
              {
                unsigned mr = cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)&0x0ffff;
                if ( dialog->ussdMrRef != mr )
                  throw MAPDIALOG_FATAL_ERROR(
                    FormatText("MAP::putCommand: Opss, bad message_reference 0x%x must be 0x%x",
                      mr,dialog->ussdMrRef));
              }
              dialog->dialogid_smsc = dialogid_smsc;
              DoUSSRUserResponce(cmd,dialog.get());
              return;
            }
            else
              throw MAPDIALOG_FATAL_ERROR(
                FormatText("MAP::PutCommand: invaid USSD code 0x%x",serviceOp));
          }
          catch(...)
          {
            MutexGuard ussd_map_guard( ussd_map_lock );
            ussd_map.erase(sequence);
            throw;
          }
        }else if ( !dialog2  ) {
          if ( cmd->get_commandId() == DELIVERY )
            dialog.assign(MapDialogContainer::getInstance()->
                      createOrAttachSMSCDialog(
                        dialogid_smsc,
                        SSN,
                        string(cmd->get_sms()->getDestinationAddress().value),
                        cmd));
          else // QUERYABONENTSTATUS
            dialog.assign(MapDialogContainer::getInstance()->
                      createOrAttachSMSCDialog(
                        dialogid_smsc,
                        SSN,
                        /*string(cmd->get_sms()->getDestinationAddress().value)*/"",
                        cmd));

          if ( dialog.isnull() ) {
            //throw MAPDIALOG_TEMP_ERROR("Can't create or attach dialog");
            //SendRescheduleToSmsc(dialogid_smsc);
            __trace2__("MAP::%s: was scheduled (state:NONE) ",__FUNCTION__);
            return;
          }
        }else{
          dialog.assign(dialog2->AddRef());
          dialogid_map = dialog->dialogid_map;
          dialogid_map = MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,SSN);
          dialog->dialogid_smsc = dialogid_smsc;
          dialog->dropChain = false;
        }
      }catch(exception& e){
        __trace2__("#except#MAP::PutCommand# when create dialog");
        __trace2__("   <exception>:%s",e.what());
        //throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
        throw MAPDIALOG_FATAL_ERROR("MAP::PutCommand: can't create dialog");
      }
      if ( dialog.isnull() ) {
        __trace2__("MAP::putCommand: can't create SMSC->MS dialog (locked), request has bean attached");
        // command has bean attached by dialog container
      }else{
        dialog->isQueryAbonentStatus = (cmd->get_commandId() == QUERYABONENTSTATUS);
        if ( dialog->isQueryAbonentStatus )
        {
          dialog->QueryAbonentCommand = cmd;
          if ( dialog->QueryAbonentCommand->get_abonentStatus().addr.getLenght() == 0 )
            throw MAPDIALOG_FATAL_ERROR("incorrect address");
        }
        dialog->dropChain = false;
        dialog->wasDelivered = false;
        dialog->hlrWasNotified = false;
        dialog->state = MAPST_START;
        __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
        dialogid_map = dialog->dialogid_map;
        StartDialogProcessing(dialog.get(),cmd);
      }
    }else{ // MAP dialog
      if ( dialog2 ) throw runtime_error("MAP::putCommand can't chain MAPINPUT");
      dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_smsc));
      if ( dialog.isnull() )
        throw MAPDIALOG_FATAL_ERROR(
          FormatText("MAP::putCommand: Opss, here is no dialog with id x%x",(ET96MAP_DIALOGUE_ID_T)dialogid_smsc));
      //dialog->state = MAPST_START;
      __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
      dialogid_map = dialogid_smsc;
      if ( dialog->state == MAPST_WaitSubmitCmdConf ){
        if ( dialog->isUSSD )
        {
          dialog->state = MAPST_USSDWaitResponce;
          __trace2__("MAP::%s processing USSD submit responce");
          if ( cmd->get_resp()->get_status() != 0 )
          {
            __trace2__("MAP::%s delivery error");
            DoUSSRUserResponceError(cmd,dialog.get());
            CloseMapDialog(dialog->dialogid_map,dialog->ssn);
            DropMapDialog(dialog.get());
          }
          else
          {
            __trace2__("MAP::%s delivery ok");
            {
              MutexGuard ussd_map_guard( ussd_map_lock );
              ussd_map[dialog->ussdSequence] = dialog->dialogid_map;
            }
          }
        }
        else
        {
          ResponseMO(dialog.get(),cmd->get_resp()->get_status());
          CloseMapDialog(dialog->dialogid_map,dialog->ssn);
          DropMapDialog(dialog.get());
        }
      }else 
        throw MAPDIALOG_BAD_STATE(
          FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
}

static inline 
void DoProvErrorProcessing(ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( provErrCode_p != 0 ){
    if ( *provErrCode_p == 0x02 || // unsupported service
         *provErrCode_p == 0x03 || // mystyped parametor
         *provErrCode_p == 0x06 || // unexcpected responnse from peer
         *provErrCode_p == 0x09 || // invalid responce recived
         (*provErrCode_p > 0x0a && *provErrCode_p <= 0x10)) // unxpected component end other
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::%s fatal *provErrCode_p: 0x%x",__FUNCTION__,*provErrCode_p),
        MAP_ERRORS_BASE+*provErrCode_p);
    else
      throw MAPDIALOG_TEMP_ERROR(
        FormatText("MAP::%s temp *provErrCode_p: 0x%x",__FUNCTION__,*provErrCode_p),
        MAP_ERRORS_BASE+*provErrCode_p);
  }
}

static inline
void DoRInfoErrorProcessor(
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( errorSendRoutingInfoForSm_sp != 0 ){
    bool fatal = false;
    switch( errorSendRoutingInfoForSm_sp->errorCode){
      case 1:case 11:case 13:case 21:fatal = true;
      case 6:fatal = false;
      default:fatal = true;
    }
    if ( fatal )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::%s fatal errorSendRoutingInfoForSm_sp->errorCode: 0x%x",
                   __FUNCTION__,
                   errorSendRoutingInfoForSm_sp->errorCode),
                   MAP_ERRORS_BASE+errorSendRoutingInfoForSm_sp->errorCode);
    else      
      throw MAPDIALOG_TEMP_ERROR(
        FormatText("MAP::%s temp errorSendRoutingInfoForSm_sp->errorCode: 0x%x",
                 __FUNCTION__,
                 errorSendRoutingInfoForSm_sp->errorCode),
                 MAP_ERRORS_BASE+errorSendRoutingInfoForSm_sp->errorCode);
  }
  DoProvErrorProcessing(provErrCode_p);
}


extern "C"
USHORT_T Et96MapGetACVersionConf(ET96MAP_LOCAL_SSN_T localSsn,UCHAR_T version,ET96MAP_SS7_ADDR_T *ss7Address_sp,ET96MAP_APP_CONTEXT_T ac)
{
  __trace2__("MAP::%s ",__FUNCTION__);
  unsigned dialogid_map = 0;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    MAPIO_TaskACVersionNotifier();   
    if ( version == 3 ) version = 2;
    char text[32];
    SS7ToText(ss7Address_sp,text);
    string s_key(text);
    __trace2__("MAP::%s: [exists %s]?",__FUNCTION__,s_key.c_str());
    typedef multimap<string,unsigned>::iterator I;
    MutexGuard guard(x_map_lock);
    pair<I,I> range = x_map.equal_range(s_key);
    if ( range.first == range.second ) return ET96MAP_E_OK;
      //throw runtime_error("MAP::Et96MapGetACVersionConf has no address for AC resolving");
    I it;
    for ( it = range.first; it != range.second;++it) {
      unsigned dialogid_map = 0;
      unsigned dialogid_smsc = 0;
      MAP_TRY{
        dialogid_map = it->second;
        DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
        if ( dialog.isnull() ) {
          throw MAPDIALOG_HEREISNO_ID(
            FormatText("MAP::dialog 0x%x is not present",dialogid_map));
        }
        else
        {
          __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
          dialogid_smsc = dialog->dialogid_smsc;
          switch( dialog->state ){
          case MAPST_WaitHlrVersion:
            dialog->version = version;
            dialog->hlrVersion = version;
            dialog->state = MAPST_RInfoFallBack;
            SendRInfo(dialog.get());
            break;
          case MAPST_WaitMcsVersion:
            dialog->version = version;
            SendSms(dialog.get());
            /*
            if ( SendSms(dialog.get()) == SMS_SEGMENTATION )
              dialog->state = MAPST_WaitSpecOpenConf;
            else
              dialog->state = MAPST_WaitOpenConf;
            */
            break;
          case MAPST_ImsiWaitACVersion:
            dialog->version = version;
            dialog->state = MAPST_ImsiWaitOpenConf;
            SendRInfo(dialog.get());
            break;
          default:
            throw MAPDIALOG_BAD_STATE(
              FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
          }
        }
      }MAP_CATCH(dialogid_map,dialogid_smsc);
    }
    x_map.erase(s_key);
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  __trace2__("MAP::%s done",__FUNCTION__);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapOpenConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_OPEN_RESULT_T openResult,
  ET96MAP_REFUSE_REASON_T *refuseReason_p,
  ET96MAP_SS7_ADDR_T *respondingAddr_sp,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_USERDATA_T *specificInfo_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {throw 
      MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",dialogid_map));}
    __trace2__("MAP::%s:DELIVERY_SM %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitDelRepConf:
      if ( openResult == ET96MAP_RESULT_NOT_OK ){
        dialog->state = MAPST_MTFINAL;
        __trace2__(":ERROR:MAP::%s reason 0x%x",refuseReason_p?*refuseReason_p:0);
        DropMapDialog(dialog.get());
      }
      // nothing;  
      break;
    case MAPST_RInfoFallBack:
    case MAPST_WaitSpecOpenConf:
    case MAPST_WaitOpenConf:
    case MAPST_ImsiWaitOpenConf:
      if ( openResult == ET96MAP_RESULT_NOT_OK ){
        if ( refuseReason_p && *refuseReason_p == ET96MAP_APP_CONTEXT_NOT_SUPP ){
          if ( dialog->version != 1 ){
            --dialog->version;
            dialogid_map = RemapDialog(dialog.get(),localSsn);
            switch ( dialog->state ) {
            case MAPST_RInfoFallBack: 
            case MAPST_ImsiWaitOpenConf:
              SendRInfo(dialog.get());
              break;
            case MAPST_WaitSpecOpenConf:
            case MAPST_WaitOpenConf:
              SendSms(dialog.get());
              break;
            }
            break;
          }else{
            //dialog->state = MAPST_BROKEN;
            throw MAPDIALOG_FATAL_ERROR(
              FormatText("MAP::%s version alredy 1 and unsupported!",__FUNCTION__),
              refuseReason_p?(MAP_ERRORS_BASE+*refuseReason_p):MAP_FALURE);
          }
        }else{
          //dialog->state = MAPST_BROKEN;
          //throw MAPDIALOG_BAD_STATE(
          //  FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
          /*throw MAPDIALOG_FATAL_ERROR(
            FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc),
            provErrCode_p?MAP_ERRORS_BASE+*provErrCode_p:MAP_FALURE);*/
          DoProvErrorProcessing(provErrCode_p);
        }
      }
      switch(dialog->state){
      case MAPST_RInfoFallBack:
        dialog->hlrVersion = dialog->version;
        dialog->state = MAPST_WaitRInfoConf;
        break;
      case MAPST_WaitSpecOpenConf:
        dialog->state = MAPST_WaitSpecDelimeter;
        break;
      case MAPST_WaitOpenConf:
        dialog->state = MAPST_WaitSmsConf;
        break;
      case MAPST_ImsiWaitOpenConf:
        dialog->state = MAPST_ImsiWaitRInfo;
        break;
      }
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d",__FUNCTION__,dialog->state));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

#if 0
void DoMTConfErrorProcessor(
  ET96MAP_ERROR_FORW_SM_MT_T* errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  if ( errorForwardSMmt_sp ){
    bool fatal = false;
    switch( errorForwardSMmt_sp->errorCode ){
    case 27: /*Absent*/
    case 31: /*Busy*/
      fatal = false;
      break;
    case 32: /*Delivery failure*/
      fatal = false;
      break;
    case 34: /*System failure*/
      fatal = false;
      break;
    default:
      fatal = true;
    }
    if ( fatal ) 
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::%s fatal *errorForwardSMmt_sp: 0x%x",__FUNCTION__,errorForwardSMmt_sp->errorCode),MAP_ERRORS_BASE+errorForwardSMmt_sp->errorCode);
    else
      throw MAPDIALOG_TEMP_ERROR(
        FormatText("MAP::%s temp *errorForwardSMmt_sp: 0x%x",__FUNCTION__,errorForwardSMmt_sp->errorCode),MAP_ERRORS_BASE+errorForwardSMmt_sp->errorCode);
  }
  /*else if (provErrCode_p){
    throw MAPDIALOG_TEMP_ERROR(
      FormatText("MAP::%s temp *provErrCode_p: 0x%x",__FUCTION__,*provErrCode_p));
  }*/
  if ( provErrCode_p != 0 ){
    if ( *provErrCode_p == 0x02 || // unsupported service
         *provErrCode_p == 0x03 || // mystyped parametor
         *provErrCode_p == 0x06 || // unexcpected responnse from peer
         *provErrCode_p == 0x09 || // invalid responce recived
         (*provErrCode_p > 0x0a && *provErrCode_p <= 0x10)) // unxpected component end other
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::%s fatal *provErrCode_p: 0x%x",__FUNCTION__,*provErrCode_p),MAP_ERRORS_BASE+*provErrCode_p);
    else
      throw MAPDIALOG_TEMP_ERROR(
        FormatText("MAP::%s temp *provErrCode_p: 0x%x",__FUNCTION__,*provErrCode_p),MAP_ERRORS_BASE+*provErrCode_p);
  }
}
#endif

static USHORT_T  Et96MapVxSendRInfoForSmConf_Impl( 
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T *imsi_sp,
  ET96MAP_ADDRESS_T *mscNumber_sp,
  ET96MAP_LMSI_T *lmsi_sp,
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p,
  unsigned version,
  ET96MAP_MWD_SET_T *mwdSet,
  ET96MAP_LOCATION_INFO_T *locationInfo_sp
  )
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    if ( version == 1 ) 
      throw runtime_error(
        FormatText("MAP::%s unsupported protocol version %d",__FUNCTION__,version));
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__FUNCTION__,_di));
    }
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
    
    dialog->routeErr = 0;
    
    if ( dialog->state == MAPST_WaitRInfoConf )
    {
      if ( errorSendRoutingInfoForSm_sp && 
        (errorSendRoutingInfoForSm_sp->errorCode == 27 ) )
      {
        __trace2__("MAP::%s absent subscriber",__FUNCTION__);
        dialog->subscriberAbsent = true;
        dialog->routeErr = MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_ERRORS_BASE+27);
      }else{
        try {
          DoRInfoErrorProcessor(errorSendRoutingInfoForSm_sp,provErrCode_p);
        }catch(MAPDIALOG_ERROR& e){
          __trace2__("MAP::%s was error %s",__FUNCTION__,e.what());
          dialog->routeErr = e.code;
        }
      }
    }
    else DoRInfoErrorProcessor(errorSendRoutingInfoForSm_sp,provErrCode_p);
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitRInfoConf:
      if ( dialog->routeErr ) {
        dialog->state = MAPST_WaitRInfoClose;
        break;
      }
    case MAPST_ImsiWaitRInfo:
      {
        dialog->s_imsi = ImsiToString(imsi_sp);
        dialog->s_msc = MscToString(mscNumber_sp);
        mkSS7GTAddress( &dialog->destMscAddr, mscNumber_sp, 8 );
        dialog->smRpDa.typeOfAddress = ET96MAP_ADDRTYPE_IMSI;
        dialog->smRpDa.addrLen = imsi_sp->imsiLen;
        memcpy( dialog->smRpDa.addr, imsi_sp->imsi, imsi_sp->imsiLen );
#if !defined DISABLE_TRACING    
        {
          auto_ptr<char> b(new char[imsi_sp->imsiLen*4+1]);
          memset(b.get(),0,imsi_sp->imsiLen*4+1);
          for ( int i=0,k=0; i < imsi_sp->imsiLen; ++i ) {
            k += sprintf(b.get()+k,"%02x ",imsi_sp->imsi[i]);
          }
          __trace2__("MAP::Et96MapV2SendRInfoForSmConf:IMSI: %s",b.get());
        }
        {
          auto_ptr<char> b(new char[mscNumber_sp->addressLength*4+1]);
          memset(b.get(),0,mscNumber_sp->addressLength*4+1);
          for ( int i=0,k=0; i < (mscNumber_sp->addressLength+1)/2; ++i ) {
            k += sprintf(b.get()+k,"%02x ",mscNumber_sp->address[i]);
          }
          __trace2__("MAP::Et96MapV2SendRInfoForSmConf:MSCNUMBER: %s",b.get());
        }
#endif
        if ( dialog->state == MAPST_WaitRInfoConf )
          dialog->state = MAPST_WaitRInfoClose;
        else if ( dialog->state == MAPST_ImsiWaitRInfo )
          dialog->state = MAPST_ImsiWaitCloseInd;
        else
          throw MAPDIALOG_BAD_STATE(
            FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
        break;
      }
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}				       

extern "C"
USHORT_T Et96MapV1SendRInfoForSmConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T *imsi_sp,
  ET96MAP_LOCATION_INFO_T *locationInfo_sp,
  ET96MAP_LMSI_T *lmsi_sp,
  ET96MAP_MWD_SET_T *mwdSet,
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return Et96MapVxSendRInfoForSmConf_Impl(
    localSsn,dialogueId,invokeId,imsi_sp,0,lmsi_sp,errorSendRoutingInfoForSm_sp,provErrCode_p,
    1,mwdSet,locationInfo_sp);
}

extern "C"
USHORT_T  Et96MapV2SendRInfoForSmConf ( 
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T *imsi_sp,
  ET96MAP_ADDRESS_T *mscNumber_sp,
  ET96MAP_LMSI_T *lmsi_sp,
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  return Et96MapVxSendRInfoForSmConf_Impl(
    localSsn,dialogueId,invokeId,imsi_sp,mscNumber_sp,lmsi_sp,errorSendRoutingInfoForSm_sp,provErrCode_p,
    2,0,0);
}

extern "C"
USHORT_T Et96MapCloseInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_USERDATA_T *specificInfo_sp,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",_di));}
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitRInfoClose:
      if ( dialog->isQueryAbonentStatus ){
        int status;
        if ( dialog->routeErr || dialog->subscriberAbsent ) 
          status = AbonentStatus::OFFLINE;
        else 
          status = AbonentStatus::ONLINE;
        SendAbonentStatusToSmsc(dialog.get(),status);
        dialog->state = MAPST_CLOSED;
        DropMapDialog(dialog.get());
      }else{
        if ( !dialog->routeErr ) {
          MapDialogContainer::getInstance()->reAssignDialog(dialogueId,localSsn);
          dialogueId = dialog->dialogid_map;
          dialog->state = MAPST_WaitMcsVersion;
          QueryMcsVersion(dialog.get());
        }else{
          __trace2__("MAP::%s: RIfno Return no route, close dialog");
          dialog->state = MAPST_CLOSED;
          dialogid_smsc = dialog->dialogid_smsc;
          //throw MAPDIALOG_ERROR(dialog->routeErr,"lisy error");
          SendErrToSmsc(dialog->dialogid_map,dialog->routeErr);
          DropMapDialog(dialog.get());
        }
      }
      break;
    case MAPST_WaitSmsClose:
      // penging processing
      dialog->state = MAPST_CLOSED;
      SendOkToSmsc(dialog.get());
      DropMapDialog(dialog.get());
      break;
    case MAPST_WaitDelClose:
    case MAPST_WaitDelRepConf:
      dialog->state = MAPST_CLOSED;
      DropMapDialog(dialog.get());
      break;
    case MAPST_ImsiWaitCloseInd:
      dialog->state = MAPST_END;
      ContinueImsiReq(dialog->associate,dialog->s_imsi,dialog->s_msc);
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapUAbortInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_USER_REASON_T *userReason_p,
  ET96MAP_DIAGNOSTIC_INFO_T *diagInfo_p,
  ET96MAP_USERDATA_T *specificInfo_sp,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__FUNCTION__,dialogueId));
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s dialog 0x%x , userReason: 0x%x",__FUNCTION__,dialogid_map,userReason_p?*userReason_p:-1);
    //throw runtime_error("UABORT");
    throw MAPDIALOG_TEMP_ERROR("UABORT",MAP_ERRORS_BASE+(userReason_p?*userReason_p:-1));
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

USHORT_T Et96MapPAbortInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_PROV_REASON_T provReason,
  ET96MAP_SOURCE_T source,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__FUNCTION__,dialogueId));
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s dialog 0x%x provReason: 0x%x",__FUNCTION__,dialogid_map,provReason);
    throw MAPDIALOG_TEMP_ERROR("PABORT",MAP_ERRORS_BASE+provReason);
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapOpenInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_SS7_ADDR_T *ss7DestAddr_sp,
  ET96MAP_SS7_ADDR_T *ss7OrigAddr_sp,
  ET96MAP_IMSI_T *destRef_sp,
  ET96MAP_ADDRESS_T *origRef_sp,
  ET96MAP_USERDATA_T *specificInfo_sp)
{
  try{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->createDialog(dialogueId,localSsn/*,0*/));
    dialog->hasIndAddress = false;
    if ( specificInfo_sp!=0 && specificInfo_sp->specificInfoLen >= 3 )
    {
      if ( specificInfo_sp->specificData[0] == 0x82 )
      {
        __trace2__("MAP::%s parse specific",__FUNCTION__);
        unsigned x = specificInfo_sp->specificData[1];
        if ( (((unsigned)specificInfo_sp->specificData[x+1] >> 4)&0x0f == 0xf ) )
          x = (x-1)*2-1;
        else 
          x = (x-1)*2;
        memcpy(&dialog->m_msAddr,specificInfo_sp->specificData+1,specificInfo_sp->specificInfoLen-1);
        dialog->m_msAddr.addressLength = x;
        dialog->hasIndAddress = true;
#if !defined DISABLE_TRACING        
        {
          ostringstream ost;
          unsigned x = specificInfo_sp->specificData[1]-1;
          for (unsigned i=0;i<x;++i) 
            ost << hex << setfill('0') << setw(2) << (((unsigned)(dialog->m_msAddr.address[i]))&0x0ff) << " ";
          __trace2__("MAP::%s::adr(%d):%s",__FUNCTION__,dialog->m_msAddr.addressLength,ost.str().c_str());
        }
#endif
      }
    }
    if ( dialog.isnull() )
      throw runtime_error("MAP:: can't create dialog");
    __trace2__("MAP:: create dialog with ptr 0x%p, dialogid 0x%x",dialog.get(),dialogueId);
    dialog->state = MAPST_WaitSms;
  }
  catch(exception& e)
  {
    __trace2__("#except#MAP::%s# MAP.did 0x%x",__FUNCTION__,dialogueId);
    __trace2__("   <exception>:%s",e.what());
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    MapDialogContainer::getInstance()->dropDialog(dialogueId);
    Et96MapOpenResp(localSsn,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
    Et96MapDelimiterReq(localSsn,dialogueId,0,0);
  }
  return ET96MAP_E_OK;
}

static USHORT_T Et96MapVxForwardSmMOInd_Impl (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp,
  unsigned version)
{
  bool open_confirmed = false;
  try{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__FUNCTION__,dialogueId));
    }
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->state = MAPST_WaitSmsMODelimiter2;
      break;
    case MAPST_WaitSmsMOInd:
      open_confirmed = true;
      dialog->state = MAPST_WaitSmsMODelimiter;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
    dialog->invokeId = invokeId;
    AttachSmsToDialog(dialog.get(),smRpUi_sp,smRpOa_sp);
    __trace2__("MAP::%s: %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
  }
  catch(exception& e)
  {
    __trace2__("#except#MAP::%s# MAP.did 0x%x",__FUNCTION__,dialogueId);
    __trace2__("   <exception>:%s",e.what());
    if ( !open_confirmed ){
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      //MapDialogContainer::getInstance()->dropDialog(dialogueId);
      DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
      }
      Et96MapOpenResp(localSsn,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
      Et96MapDelimiterReq(localSsn,dialogueId,0,0);
    }
    TryDestroyDialog(dialogueId);
  }
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapV1ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp)
{
  return Et96MapVxForwardSmMOInd_Impl(localSsn,dialogueId,invokeId,smRpDa_sp,smRpOa_sp,smRpUi_sp,1);
}

extern "C"
USHORT_T Et96MapV2ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp)
{
  return Et96MapVxForwardSmMOInd_Impl(localSsn,dialogueId,invokeId,smRpDa_sp,smRpOa_sp,smRpUi_sp,2);
}

static USHORT_T Et96MapVxForwardSmMTConf_Impl (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p,
  unsigned version)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__FUNCTION__,_di));
    }
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
    //DoMTConfErrorProcessor(errorForwardSMmt_sp,provErrCode_p);
    /*if ( provErrCode_p ) {
      throw MAPDIALOG_FATAL_ERROR(FormatText("provErrCode_p == 0x%x",*provErrCode_p),MAP_ERRORS_BASE+*provErrCode_p);
    }*/
    DoProvErrorProcessing(provErrCode_p);
    if ( errorForwardSMmt_sp )
    {
      if ( errorForwardSMmt_sp->errorCode == 5 ||
           errorForwardSMmt_sp->errorCode == 27 )
      {
        dialog->subscriberAbsent = true;
        throw MAPDIALOG_TEMP_ERROR("MAP::absent subscriber",MAP_ERRORS_BASE+errorForwardSMmt_sp->errorCode);
      }
      if ( errorForwardSMmt_sp->errorCode == 32 && 
        errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T::ET96MAP_MEM_CAPACITY_EXCEEDED )
      {
        dialog->memoryExceeded = true;
        throw MAPDIALOG_TEMP_ERROR("MAP::memory exceeded",MAP_ERRORS_BASE+errorForwardSMmt_sp->errorCode);
      }
      if ( errorForwardSMmt_sp->errorCode == 34 /*System failure*/||
           errorForwardSMmt_sp->errorCode == 31 /*Busy*/) 
      {
        throw MAPDIALOG_TEMP_ERROR(FormatText("errorForwardSMmt_sp->errorCode == 0x%x",errorForwardSMmt_sp->errorCode),
          MAP_ERRORS_BASE+errorForwardSMmt_sp->errorCode);
      }
      throw MAPDIALOG_FATAL_ERROR(FormatText("errorForwardSMmt_sp->errorCode == 0x%x",errorForwardSMmt_sp->errorCode),
          MAP_ERRORS_BASE+errorForwardSMmt_sp->errorCode);
    }
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitSmsConf:
      dialog->wasDelivered = true;
      dialog->state = MAPST_WaitSmsClose;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapV1ForwardSmMT_MOConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return Et96MapVxForwardSmMTConf_Impl(localSsn,dialogueId,invokeId,errorForwardSMmt_sp,provErrCode_p,1); 
}

extern "C"
USHORT_T Et96MapV2ForwardSmMTConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return Et96MapVxForwardSmMTConf_Impl(localSsn,dialogueId,invokeId,errorForwardSMmt_sp,provErrCode_p,2); 
}

extern "C"
USHORT_T Et96MapDelimiterInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  UCHAR_T priorityOrder)
{
  bool open_confirmed = false;
  USHORT_T result;
  ET96MAP_REFUSE_REASON_T reason;
  try{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__FUNCTION__,dialogueId));
    }
    __trace2__("MAP::%s: %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->state = MAPST_WaitSmsMOInd;
      reason = ET96MAP_NO_REASON;
      result = Et96MapOpenResp(dialog->ssn,dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: dialog opened error 0x%x",result));
      __trace2__("MAP::Et96MapDelimiterInd: dialog opened");
      result = Et96MapDelimiterReq(dialog->ssn,dialogueId,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: Et96MapDelimiterReq return error 0x%x",result));
      open_confirmed = true;
      break;
    case MAPST_WaitSmsMODelimiter2:
      reason = ET96MAP_NO_REASON;
      result = Et96MapOpenResp(dialog->ssn,dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: dialog opened error 0x%x",result));
      __trace2__("MAP::Et96MapDelimiterInd: dialog opened");
      dialog->state = MAPST_WaitImsiReq;
      PauseOnImsiReq(dialog.get());
      break;
    case MAPST_WaitSmsMODelimiter:
      open_confirmed = true;
      dialog->state = MAPST_WaitImsiReq;
      PauseOnImsiReq(dialog.get());
      break;
    case MAPST_WaitSpecDelimeter:
      dialog->state = MAPST_WaitSmsConf;
      SendSegmentedSms(dialog.get());
      break;
    case MAPST_WaitSmsClose:
      SendOkToSmsc(dialog.get());
      //dialog->state = MAPST_WaitSmsConf;
      SendNextMMS(dialog.get());
      break;
    case MAPST_WaitAlertDelimiter:
      ResponseAlertSC(dialog.get());
      CloseMapDialog(dialog->dialogid_map,dialog->ssn);
      DropMapDialog(dialog.get());
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
  }
  catch(exception& e)
  {
    __trace2__("#except#MAP::%s# MAP.did 0x%x",__FUNCTION__,dialogueId);
    __trace2__("   <exception>:%s",e.what());
    if ( !open_confirmed ){
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      //MapDialogContainer::getInstance()->dropDialog(dialogueId);
      DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
      }
      
      Et96MapOpenResp(localSsn,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
      Et96MapDelimiterReq(localSsn,dialogueId,0,0);
    }
    TryDestroyDialog(dialogueId);
  }
  return ET96MAP_E_OK;
}


/// ---------------------------------------------------------
unsigned char  lll_7bit_2_8bit[128] = {
0x40,0xa3,0x24,0xa5,0xe8,0xe9,0xf9,0xec, // 0
0xf2,0xc7,0x0a,0xd8,0xf8,0x0d,0xc5,0xe5, // 8
0x44,0x5f,0x46,0x47,0x4c,0x57,0x50,0x59, // 16
0x53,0x51,0x58,0x00,0xc6,0xe6,0xdf,0xc9, // 24
0x20,0x21,0x22,0x23,0xa4,0x25,0x26,0x27, // 32
0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f, // 40
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, // 48
0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f, // 56
0xa1,0x41,0x42,0x43,0x44,0x45,0x46,0x47, // 64
0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f, // 72
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, // 80
0x58,0x59,0x5a,0xc4,0xd6,0xd1,0xdc,0xa7, // 88
0xbf,0x61,0x62,0x63,0x64,0x65,0x66,0x67, // 96
0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f, // 104
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, // 112
0x78,0x79,0x7a,0xe4,0xf6,0xf1,0xfc,0xe0}; // 120

unsigned char  lll_8bit_2_7bit[256] = {
0x1b,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x0a,0x54,0x54,0x0d,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x20,0x21,0x22,0x23,0x02,0x25,0x26,0x27,
0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
0x58,0x59,0x5a,0x54,0x54,0x54,0x54,0x11,
0x54,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7a,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x40,0x54,0x01,0x24,0x03,0x54,0x5f,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x60,
0x54,0x54,0x54,0x54,0x5b,0x0e,0x1c,0x09,
0x54,0x1f,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x5d,0x54,0x54,0x54,0x54,0x5c,0x54,
0x0b,0x54,0x54,0x54,0x5e,0x54,0x54,0x1e,
0x7f,0x54,0x54,0x54,0x7b,0x0f,0x1d,0x54,
0x04,0x05,0x54,0x54,0x07,0x54,0x54,0x54,
0x54,0x7d,0x08,0x54,0x54,0x54,0x7c,0x54,
0x0c,0x06,0x54,0x54,0x7e,0x54,0x54,0x54};

static void ContinueImsiReq(MapDialog* dialog,const string& s_imsi,const string& s_msc)
{
  __trace2__("MAP::%s ismsi %s, msc: %s",__FUNCTION__,s_imsi.c_str(),s_msc.c_str());
  if ( dialog->state == MAPST_END ){// already closed
    return;
  }
  if ( s_imsi.length() == 0 || s_msc.length() == 0 )
  {
    dialog->state = MAPST_WaitSubmitCmdConf;
    ResponseMO(dialog,9);
    CloseMapDialog(dialog->dialogid_map,dialog->ssn);
    DropMapDialog(dialog);
  }
  else
  {
    dialog->state = MAPST_WaitSubmitCmdConf;
    dialog->s_imsi = s_imsi;
    dialog->s_msc = s_msc;
    SendSubmitCommand(dialog);
  }
}

static void PauseOnImsiReq(MapDialog* map)
{
  bool success = false;
  DialogRefGuard dialog(MapDialogContainer::getInstance()->createDialogImsiReq(SSN,map));
  if (dialog.isnull()) throw runtime_error(
    FormatText("MAP::%s can't create dialog",__FUNCTION__));
  unsigned dialogid_map = dialog->dialogid_map;
  MAP_TRY{
    if ( map->sms.get() == 0 ) 
      throw runtime_error(
        FormatText("MAP::%s has no SMS",__FUNCTION__));
    if ( !map->isUSSD )
    {
      __trace2__("MAP::%s make MAP mkMapAddress",__FUNCTION__);
      mkMapAddress( &dialog->m_msAddr, map->sms->getOriginatingAddress().value, map->sms->getOriginatingAddress().length );
    }
    else 
    {
      __trace2__("MAP::%s using preparsed address",__FUNCTION__);
      if (!map->hasIndAddress )
        throw runtime_error("MAP::%s MAP.did:{0x%x} has no originating address");
#if !defined DISABLE_TRACING    
      {
        auto_ptr<char> b(new char[sizeof(ET96MAP_ADDRESS_T)*3+1]);
        memset(b.get(),0,sizeof(ET96MAP_ADDRESS_T)*3+1);
        for ( int i=0,k=0; i < sizeof(ET96MAP_ADDRESS_T); ++i ) {
          unsigned char * d = (unsigned char*)&map->m_msAddr;
          k += sprintf(b.get()+k,"%02x ",(unsigned int)d[i]);
        }
        __trace2__("MAP::%s ARRDDRESS: %s",__FUNCTION__,b.get());
      }
#endif
      dialog->m_msAddr = map->m_msAddr;
    }
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
    __trace2__("MAP::%s: Query HLR AC version",__FUNCTION__);
    //dialog->mshlrAddr = map->mshlrAddr;
    dialog->state = MAPST_ImsiWaitACVersion;
    QueryHlrVersion(dialog.get());
  }MAP_CATCH(dialogid_map,0);
}

static string GetUSSDSubsystem(
  const char* text,
  unsigned length)
{
  const char* p = text;
  const char* pEnd = text+length;
  for ( ; p < pEnd; ++p ) if ( (*p != '#') && (*p != '*') ) break;
  const char* sBegin = p;
  for ( ; p < pEnd; ++p ) if ( (*p == '#') || (*p == '*') ) break;
  const char* sEnd = p;
  return string(sBegin,sEnd);
}

static unsigned MakeMrRef()
{
  return time(0)%0x0ffff;
}

extern "C"
USHORT_T Et96MapV2ProcessUnstructuredSSRequestInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDataCodingScheme,
  ET96MAP_USSD_STRING_T ussdString_s,
  ET96MAP_ADDRESS_T *msisdn_sp)
{
  unsigned __dialogid_map = 0;
  MAP_TRY{
    __trace2__("MAP::%s MAP.did:{0x%x}",__FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__FUNCTION__,dialogueId));
    dialog->isUSSD = true;
    __dialogid_map = dialogueId;
    string subsystem;
    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    __trace2__("MAP::%s request encoding 0x%x",__FUNCTION__,ussdDataCodingScheme);
    {
      MicroString ms;
      __trace2__("MAP::%s request length %d",__FUNCTION__,ussdString_s.ussdStrLen);
      unsigned chars = ussdString_s.ussdStrLen*8/7;
      Convert7BitToSMSC7Bit(ussdString_s.ussdStr,chars/*ussdString_s.ussdStrLen*/,&ms,0);
      subsystem = GetUSSDSubsystem(ms.bytes,ms.len);
      __trace2__("MAP::%s subsystem: %s",__FUNCTION__,subsystem.c_str());
      sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,ms.bytes,ms.len);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)MAP_SMSC7BIT_ENCODING);
    }
    Address dest_addr = Address(subsystem.c_str());
    dest_addr.type = 0;
    dest_addr.plan = 1;
    unsigned esm_class = 0;
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(dest_addr);
    dialog->sms = _sms;
    dialog->state = MAPST_WaitSmsMODelimiter2;
    //dialog->ussdSequence = NextSequence();
    dialog->ussdMrRef = MakeMrRef();
    dialog->sms->setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_IND);
    dialog->sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    //{
    //  ostringstream  ost;
    //  ost << dialog->ussdSequence;
    //  //dialog->sms->setStrProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,message_reference.str());
    //}
    //SendSubmitCommand (dialog.get());
    dialog->invokeId = invokeId;
  }MAP_CATCH(__dialogid_map,0);
  return ET96MAP_E_OK;
}

static void SendAlertToSMSC(MapDialog* dialog,ET96MAP_ADDRESS_T *mapAddr)
{
  Address addr;
  ConvAddrMSISDN2Smc(mapAddr,&addr);
  SmscCommand cmd = SmscCommand::makeHLRAlert(addr);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void ResponseAlertSC(MapDialog* dialog)
{
  USHORT_T result;
  if ( dialog->version == 2 ){
    result = Et96MapV2AlertSCResp(dialog->ssn,dialog->dialogid_map,dialog->invokeId,0);
  }else if ( dialog->version == 1 ){
    // nothing
  }else throw runtime_error("MAP::ResponseAlertSC:unsupported dialog version");
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::ResponseAlertSC: error 0x%hx",result));
}

extern "C"
USHORT_T Et96MapVxAlertSCInd_Impl(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ADDRESS_T *sCA_sp,
  unsigned version)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__FUNCTION__,_di));
    }
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->invokeId = invokeId;
      SendAlertToSMSC(dialog.get(),msisdnAlert_sp);
      dialog->state = MAPST_WaitAlertDelimiter;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapV2AlertSCInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ADDRESS_T *sCA_sp )
{
  return Et96MapVxAlertSCInd_Impl(localSsn,dialogueId,invokeId,msisdnAlert_sp,sCA_sp,2);
}

extern "C"
USHORT_T Et96MapV1AlertSCInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ADDRESS_T *sCA_sp )
{
  return Et96MapVxAlertSCInd_Impl(localSsn,dialogueId,invokeId,msisdnAlert_sp,sCA_sp,1);
}

void RememberMwdStatus(MapDialog* dialog,ET96MAP_ADDRESS_T* alert,ET96MAP_MWD_STATUS_T* status)
{
  if ( status != 0 )
  {
    dialog->mwdStatus = *status;
    dialog->hasMwdStatus = true;
  }
}

extern "C"
USHORT_T Et96MapV2InformSCInd (
  ET96MAP_LOCAL_SSN_T   localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T   invokeId,
  ET96MAP_ADDRESS_T     *msisdnAlert_sp,
  ET96MAP_MWD_STATUS_T  *mwdStatus_sp)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",_di));}
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__FUNCTION__,RouteToString(dialog.get()).c_str());
    __trace2__("MAP::%s: 0x%x  (state %d)",__FUNCTION__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitRInfoClose:
      RememberMwdStatus(dialog.get(),msisdnAlert_sp,mwdStatus_sp);
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

static bool NeedNotifyHLR(MapDialog* dialog)
{
  if ( dialog->hlrVersion == 0 ) {
    __trace2__("MAP::%s hlrVersion is NULL",__FUNCTION__);
  }
  return !dialog->hlrWasNotified && dialog->hlrVersion != 0 && 
    ((!dialog->wasDelivered && !dialog->routeErr && 
      ( ((unsigned)dialog->subscriberAbsent != (unsigned)dialog->mwdStatus.mnrf)||
        ((unsigned)dialog->memoryExceeded != (unsigned)dialog->mwdStatus.mcef ) 
      ))
    || 
      (dialog->wasDelivered && dialog->hasMwdStatus &&
        ( ((unsigned)dialog->subscriberAbsent != (unsigned)dialog->mwdStatus.mnrf)||
          ((unsigned)dialog->memoryExceeded != (unsigned)dialog->mwdStatus.mcef ) 
        )));
}
static void NotifyHLR(MapDialog* dialog)
{
  MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,HLR_SSN);
  dialog->hlrWasNotified = true;
  USHORT_T result;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
  SetVersion(appContext,dialog->hlrVersion);
  unsigned dialog_id = dialog->dialogid_map;
  __trace2__("MAP::%s dialogid:0x%x ssn:%d",__FUNCTION__,dialog_id,dialog->ssn);
  result = Et96MapOpenReq(
    HLR_SSN, dialog_id, 
    &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::%s: Et96MapOpenReq error 0x%x",__FUNCTION__,result));
  }
  ET96MAP_DEL_OUTCOME_T deliveryOutcom;
  if ( dialog->wasDelivered ) {
    deliveryOutcom = ET96MAP_TRANSFER_SUCCESSFUL;
    __trace2__("MAP::%s OK_DELIVERED",__FUNCTION__);
  }
  else if ( dialog->subscriberAbsent ) {
    deliveryOutcom = ET96MAP_SUBSCRIBER_ABSENT;
    __trace2__("MAP::%s SUBSCRIBER_ABSENT (flag:%d)",__FUNCTION__,dialog->mwdStatus.mnrf);
  }
  else if ( dialog->memoryExceeded ) {
    deliveryOutcom = ET96MAP_MEMORY_CAPACITY_OVERRUN;
    __trace2__("MAP::%s MEMORY_EXCEEDE (flag:%d)",__FUNCTION__,dialog->mwdStatus.mcef);
  }
  else {
    __trace2__("MAP::%s no way!",__FUNCTION__);
    //return; // Opps, strange way
    throw MAPDIALOG_ERROR(0,"no way");
  }
  if ( dialog->hlrVersion == 2) {
    result = Et96MapV2ReportSmDelStatReq(
      HLR_SSN,dialog->dialogid_map,1,&dialog->m_msAddr,&dialog->m_scAddr,deliveryOutcom);
  }else if ( dialog->hlrVersion == 1 ) {
    // ????? всегда ли ???????
    result = Et96MapV1ReportSmDelStatReq(
      HLR_SSN,dialog->dialogid_map,1,&dialog->m_msAddr,&dialog->m_scAddr);
  } else 
    throw runtime_error(
    FormatText("MAP::%s bad protocol version 0x%x",__FUNCTION__,dialog->hlrVersion));
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::%s: Et96MapV2ReportSmDelStatReq error 0x%x",__FUNCTION__,result));
  }
  result = Et96MapDelimiterReq(HLR_SSN,dialog->dialogid_map,0,0);
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::%s: Et96MapDelimiterReq error 0x%x",__FUNCTION__,result));
  }
  dialog->state = MAPST_WaitDelRepConf;
}

extern "C"
USHORT_T Et96MapV2ReportSmDelStatConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ERROR_REPORT_SM_DEL_STAT_T *errorReportSmDelStat_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  // nothig;
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapV1ReportSmDelStatConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_REPORT_SM_DEL_STAT_T *errorReportSmDelStat_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  // nothig;
  return ET96MAP_E_OK;
}

#else

#include "MapDialog_spcific.cxx"
void MAPIO_PutCommand(const SmscCommand& cmd )
{
  if ( cmd->get_commandId() == QUERYABONENTSTATUS )
  {
    SmscCommand xcmd = SmscCommand::makeQueryAbonentStatusResp(cmd->get_abonentStatus(),AbonentStatus::ONLINE);
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(xcmd);
  }
}

#endif


