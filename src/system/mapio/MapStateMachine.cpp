//$Id$

#if defined USE_MAP
#define MAP_DIALOGS_LIMIT 360

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
#include "../../util/smstext.h"

#include "../../mscman/MscManager.h"
#include "../../mscman/MscStatus.h"

using namespace smsc::mscman;

using namespace smsc::system;
using namespace smsc::util;


static const bool SMS_SEGMENTATION = true;
static inline unsigned GetMOLockTimeout() {return 45;}
static inline unsigned GetBusyDelay() { return 5; }
static void SendRInfo(MapDialog* dialog);
static bool SendSms(MapDialog* dialog);

struct XMOMAPLocker {
  string imsi;
  unsigned long startTime;
  unsigned parts;
  unsigned ref;
};
typedef std::map<string,XMOMAPLocker> XMOMAP;

static Mutex x_map_lock;
typedef multimap<string,unsigned> X_MAP;
static X_MAP x_map;
static Mutex ussd_map_lock;
typedef std::map<long long,unsigned> USSD_MAP;
static USSD_MAP ussd_map;
static Mutex x_momap_lock;
static XMOMAP x_momap;

static void ContinueImsiReq(MapDialog* dialog,const string& s_imsi,const string& s_msc);
static void PauseOnImsiReq(MapDialog* map);
static const string SC_ADDRESS() { return MapDialogContainer::GetSCAdress(); }
static const string USSD_ADDRESS() { return MapDialogContainer::GetUSSDAdress(); }
static bool NeedNotifyHLR(MapDialog* dialog);
static void ResponseAlertSC(MapDialog* dialog);
static void SendErrToSmsc(unsigned dialogid,unsigned code);
static void ForwardMO(MapDialog* dialog);

static void AbortMapDialog(unsigned dialogid,unsigned ssn)
{
  if ( dialogid == 0 ) return;
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
//  __map_trace2__("IMSI: %s",result.c_str());
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
//  __map_trace2__("MSC: %s",result.c_str());
  return result;
}

string LocationInfoToString(const ET96MAP_LOCATION_INFO_T* msc)
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
//  __map_trace2__("MSC: %s",result.c_str());
  return result;
}                                       

static unsigned MakeMrRef()
{
  return time(0)%0x0ffff;
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

static inline
void SetVersion(ET96MAP_APP_CNTX_T& ac,unsigned version){
  if ( version > 3 || version == 0 ) throw runtime_error(
    FormatText("MAP::%s: Opss, version = %d, why?",__func__,version));
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

#define MAP_ERRORS_BASE Status::MAP_ERR_BASE
#define MAP_FALURE (/*MAP_ERRORS_BASE+34*/8)

struct MAPDIALOG_ERROR : public runtime_error
{
  unsigned code;
  MAPDIALOG_ERROR(unsigned code,const string& s) :
    runtime_error(s),code(code){}
  MAPDIALOG_ERROR(const string& s) :
    runtime_error(s),code(MAKE_ERRORCODE(CMD_ERR_TEMP,0)){}
};

struct MAPDIALOG_XERROR : public runtime_error
{
  unsigned code;
  MAPDIALOG_XERROR(unsigned code,const string& s) :
    runtime_error(s),code(code){}
  MAPDIALOG_XERROR(const string& s) :
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
struct MAPDIALOG_TEMP_XERROR : public MAPDIALOG_XERROR
{
  MAPDIALOG_TEMP_XERROR(const string& s,unsigned c=MAP_FALURE) :
    MAPDIALOG_XERROR(MAKE_ERRORCODE(CMD_ERR_TEMP,c),s){}
};
struct MAPDIALOG_FATAL_XERROR : public MAPDIALOG_XERROR
{
  MAPDIALOG_FATAL_XERROR(const string& s,unsigned c=MAP_FALURE) :
    MAPDIALOG_XERROR(MAKE_ERRORCODE(CMD_ERR_FATAL,c),s){}
};


static void CloseMapDialog(unsigned dialogid,unsigned dialog_ssn){
  if ( dialogid == 0 ) return;
  USHORT_T res = Et96MapCloseReq (dialog_ssn,dialogid,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __map_trace2__("%s dialog 0x%x error, code 0x%hx",__func__,dialogid,res);
  }else{
    __map_trace2__("%s dialog 0x%x closed",__func__,dialogid);
  }
}

static void TryDestroyDialog(unsigned,bool send_error=false,unsigned err_code=0,unsigned ssn=0);
static string RouteToString(MapDialog*);

static void QueryHlrVersion(MapDialog*);
static void QueryMcsVersion(MapDialog*);

static ET96MAP_SS7_ADDR_T* GetScAddr()
{
  static ET96MAP_ADDRESS_T m_scAddr;
  static ET96MAP_SS7_ADDR_T scAddr;
  static bool initialized = false;
  if ( !initialized ) {
    mkMapAddress( &m_scAddr, SC_ADDRESS().c_str(), SC_ADDRESS().length() );
    mkSS7GTAddress( &scAddr, &m_scAddr, 8 );
    initialized = true;
  }
  return &scAddr;
}

static ET96MAP_SS7_ADDR_T* GetUSSDAddr()
{
  static ET96MAP_ADDRESS_T m_ussdAddr;
  static ET96MAP_SS7_ADDR_T ussdAddr;
  static bool initialized = false;
  if ( !initialized ) {
    mkMapAddress( &m_ussdAddr, USSD_ADDRESS().c_str(), USSD_ADDRESS().length() );
    mkSS7GTAddress( &ussdAddr, &m_ussdAddr, USSD_SSN );
    initialized = true;
  }
  return &ussdAddr;
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
  dialog->wasDelivered = false;
  dialog->hlrWasNotified = false;
  dialog->hasMwdStatus = false;
  dialog->memoryExceeded = false;
  dialog->subscriberAbsent = false;
  dialog->dropChain = false;
  dialog->associate = 0;
  dialog->id_opened = false;
  if ( !dialog->isQueryAbonentStatus ) {
    dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
    __map_trace2__("%s:DELIVERY_SM %s",__func__,RouteToString(dialog).c_str());
    mkMapAddress( &dialog->m_msAddr, dialog->sms->getDestinationAddress().value, dialog->sms->getDestinationAddress().length );
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), SC_ADDRESS().length() );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, SSN );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
    if( dialog->sms->hasStrProperty( Tag::SMSC_FORWARD_MO_TO ) ) {
      ForwardMO( dialog );
      return;
    } 
  }else{
    AbonentStatus& as = dialog->QueryAbonentCommand->get_abonentStatus();
    __map_trace2__("%s:Abonent Status cmd (%d.%d.%s)",__func__,(unsigned)as.addr.type,(unsigned)as.addr.plan,as.addr.value);
    mkMapAddress( &dialog->m_msAddr, as.addr.value, as.addr.length );
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, SSN );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
  }
  __map_trace__("StartDialogProcessing: Query HLR AC version");
  dialog->state = MAPST_WaitHlrVersion;
  QueryHlrVersion(dialog);
}

static void NotifyHLR(MapDialog* dialog);
static void MAPIO_PutCommand(const SmscCommand& cmd, MapDialog* dialog2=0 );

static void DropMapDialog_(unsigned dialogid,unsigned ssn){
  if ( dialogid == 0 ) return;
  DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid,ssn));
  if ( dialog.isnull() ) {
    __map_warn2__( "%s: dialog(0x%x) is null",__func__,dialogid);
    return;
  }
  __require__(dialog->ssn==ssn);
  if ( !dialog.isnull() ){
    {
      //MutexGuard(dialog->mutex);
      __map_trace2__("%s: dropping dialog 0x%x, chain size %d , dropChain %d, associate:%p",__func__,dialog->dialogid_map,dialog->chain.size(),dialog->dropChain, dialog->associate);
      unsigned __dialogid_map = dialog->dialogid_map;
      unsigned __dialogid_smsc = 0;
      if ( dialog->chain.size() == 0 ) {
        if ( dialog->associate != 0 && dialog->state != MAPST_END )
        {
          //Et96MapPAbortInd(SSN,dialog->associate->dialogid_map,0,0,0);
          dialog->associate->hlrVersion = dialog->hlrVersion;
          ContinueImsiReq(dialog->associate,"","");
          dialog->state = MAPST_END;
        }
        else if ( NeedNotifyHLR(dialog.get()) && !dialog->isUSSD )
        {
          try{
            dialog->Clean();
            NotifyHLR(dialog.get());
            return; // do not drop dialog!
          }catch(exception& e){
            // drop dialog
            if ( dialog->id_opened ) {
              AbortMapDialog(dialog->dialogid_map,dialog->ssn);
              dialog->id_opened = false;
            }
            __map_warn2__("%s: <exception> %s",__func__,e.what());
          }
        }
        //MapDialogContainer::getInstance()->dropDialog(dialog->dialogid_map);
        //__map_trace2__("MAP::%s: 0x%x - closed and droped - ",__func__,dialog->dialogid_map);
        //if ( dialog
        //return;
        goto dropDialogLabel;
      }
    }
    __map_trace2__("%s: restart on next in chain",__func__);
    if ( dialog->dropChain ) {
      //MutexGuard(dialog->mutex);
dropDialogLabel:
      MapDialogContainer::getInstance()->dropDialog(dialog->dialogid_map,dialog->ssn);
      for (;!dialog->chain.empty();dialog->chain.pop_front())
      {
        try{
          SmscCommand cmd = dialog->chain.front();
          SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,Status::SUBSCRBUSYMT));
        }catch(exception& e){
          __map_warn2__("%s: exception %s",__func__,e.what());
        }catch(...){
          __map_warn2__("%s: unknown exception",__func__);
        }
      }
    }else{
      SmscCommand cmd = dialog->chain.front();
      dialog->chain.pop_front();
      dialog->Clean();
      MAPIO_PutCommand(cmd, dialog.get());
    }
  }
}

static void DropMapDialog(MapDialog* dialog){
  DropMapDialog_(dialog->dialogid_map,dialog->ssn);
}

static unsigned RemapDialog(MapDialog* dialog,unsigned ssn){
  dialog->id_opened = false;
  dialog->invokeId = 0;
  return MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,dialog->ssn,ssn);
}

static void SendRescheduleToSmsc(unsigned dialogid)
{
  if ( dialogid == 0 ) return;
  __map_trace__("Send RESCHEDULE NOW to SMSC");
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,0));
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void SendAbonentStatusToSmsc(MapDialog* dialog,int status/*=AbonentStatus::UNKNOWN*/)
{
  __map_trace2__("Send abonent status(%d) to SMSC ",status);
  SmscCommand cmd = SmscCommand::makeQueryAbonentStatusResp(dialog->QueryAbonentCommand->get_abonentStatus(),status,dialog->s_msc);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void SendErrToSmsc(unsigned dialogid,unsigned code)
{
  if ( dialogid == 0 ) return;
  __map_trace2__("Send error 0x%x to SMSC dialogid=%x",code,dialogid);
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,code);
  if ( GET_STATUS_CODE(code) == Status::SUBSCRBUSYMT ){
    __map_trace2__("Set Busy delay %d",GetBusyDelay());
    cmd->get_resp()->set_delay(GetBusyDelay());
  }
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void SendOkToSmsc(/*unsigned dialogid*/MapDialog* dialog)
{
  if ( dialog == 0 || dialog->dialogid_smsc == 0 ) {
//    __map_warn2__("Send OK to SMSC: no smscid or no dialog exists");
    return;
  }
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialog->dialogid_smsc,0);
  Descriptor desc;
  desc.setImsi(dialog->s_imsi.length(),dialog->s_imsi.c_str());
  desc.setMsc(dialog->s_msc.length(),dialog->s_msc.c_str());
  cmd->get_resp()->setDescriptor(desc);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __map_trace2__("Sent OK to SMSC: MSC = %s, IMSI = %s",dialog->s_msc.c_str(), dialog->s_imsi.c_str());
}

static void CheckLockedByMO(MapDialog* dialog)
{
  MutexGuard guard(x_momap_lock);
  XMOMAP::iterator it = x_momap.find(dialog->s_imsi);
  if ( it != x_momap.end() )
  {
    if ( it->second.startTime+GetMOLockTimeout() <= time(0) )
    {
      __map_trace2__("UDHI:%s time expired, unlocked, recv(%d)",__func__,it->second.parts);
      x_momap.erase(it);
    }
    else{
      dialog->dropChain = true;
      __map_trace2__("MAP:UDHI:%s locked, reschedule NOW! recv(%d)",__func__,it->second.parts);
      throw MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,Status::LOCKEDBYMO),
                            "MAP:: Locked by MO: reschedule NOW!");
    }
  }
}

static void QueryHlrVersion(MapDialog* dialog)
{
  switch( dialog->state ){
      case MAPST_WaitHlrVersion:
          dialog->version = 2;
          dialog->hlrVersion = 2;
          dialog->state = MAPST_RInfoFallBack;
          SendRInfo(dialog);
          break;
      case MAPST_ImsiWaitACVersion:
          dialog->version = 2;
          dialog->state = MAPST_ImsiWaitOpenConf;
          SendRInfo(dialog);
          break;
      default:
          throw MAPDIALOG_BAD_STATE(
                  FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
  }
}

static void QueryMcsVersion(MapDialog* dialog)
{
  dialog->version = 2;
  SendSms(dialog);
}

/*
static void QueryHlrVersion(MapDialog* dialog)
{
  MutexGuard guard(x_map_lock);
  char text[128];
  SS7ToText(&dialog->mshlrAddr,text);
  string s_(text);
  __map_trace2__("QueryHlrVersion: [store %s]=0x%x",s_.c_str(),dialog->dialogid_map);
  x_map.insert( make_pair(s_,unsigned(dialog->dialogid_map)|(unsigned(dialog->ssn)<<24) ));
  USHORT_T result =
    Et96MapGetACVersionReq(SSN,&dialog->mshlrAddr,ET96MAP_SHORT_MSG_GATEWAY_CONTEXT);
  if ( result != ET96MAP_E_OK ) {
    x_map.erase(s_);
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::QueryHlrVersion: error 0x%x when GetAcVersion",result),MAP_FALURE);
  }
}

static void QueryMcsVersion(MapDialog* dialog)
{
  MutexGuard guard(x_map_lock);
  char text[128];
  SS7ToText(&dialog->destMscAddr,text);
  string s_(text);
  __map_trace2__("QueryMcsVersion: [store %s]=0x%x",s_.c_str(),dialog->dialogid_map);
  x_map.insert( make_pair(s_,unsigned(dialog->dialogid_map)|(unsigned(dialog->ssn)<<24) ));
  //x_map.insert(make_pair(s_,dialog->dialogid_map));
  USHORT_T result =
    Et96MapGetACVersionReq(SSN,&dialog->destMscAddr,ET96MAP_SHORT_MSG_MT_RELAY);
  if ( result != ET96MAP_E_OK ) {
    x_map.erase(s_);
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::QueryMcsVersion: error 0x%x when GetAcVersion",result),MAP_FALURE);
  }
}
*/

static void SendRInfo(MapDialog* dialog)
{
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
  memset(&dialog->mwdStatus,0,sizeof(dialog->mwdStatus));
  dialog->memoryExceeded = false;
  dialog->subscriberAbsent = false;
  SetVersion(appContext,dialog->version);
  //unsigned dialog_id = dialog->isMOreq?dialog->dialogid_req:dialog->dialogid_map;
  require ( dialog->ssn == SSN );
  unsigned dialog_id = dialog->dialogid_map;
  bool hiPrior = false;
  SMS *sms = dialog->sms.get();

  if ( !dialog->isQueryAbonentStatus ) {
    hiPrior = true;
/*    if( sms != 0 ) {
      __map_trace2__("MAP::%s dialogid:0x%x ssn:%d smpp_prio:%d lastres:%d ac:%d",__func__,dialog_id,dialog->ssn,sms->getIntProperty(Tag::SMPP_PRIORITY), sms->getLastResult(), sms?(int)sms->getAttemptsCount():-1);
      if( sms->getIntProperty(Tag::SMPP_PRIORITY) > 0 ) {
        hiPrior = true;
      } else {
        if( sms->getLastResult() != Status::ABSENTSUBSCR && 
            sms->getLastResult() != Status::SMDELIFERYFAILURE ) {
          hiPrior = true;
        } else if( (sms->getAttemptsCount()+1)%4 == 0 ) {
          hiPrior = true;
        }
      }
    }*/
  }
  __map_trace2__("MAP::%s dialogid:0x%x ssn:%d hiprior:%s ac:%d ver:%d",__func__,dialog_id,dialog->ssn,hiPrior?"true":"false", sms?(int)sms->getAttemptsCount():-1, dialog->version);
  USHORT_T result = Et96MapOpenReq(
    dialog->ssn, dialog_id,
    &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendRInfo: Et96MapOpenReq error 0x%x",result),MAP_FALURE);
  }
  dialog->id_opened = true;

  // �� !!!!
  if ( dialog->version != 2 && dialog->version != 1 ) dialog->version = 2;
  // !!!!!

  if ( dialog->version == 2 ) {
    result = Et96MapV2SendRInfoForSmReq(dialog->ssn, dialog_id, 0, &dialog->m_msAddr,
      hiPrior ? ET96MAP_ATTEMPT_DELIVERY : ET96MAP_DO_NOT_ATTEMPT_DELIVERY,
      &dialog->m_scAddr );
  }else if ( dialog->version == 1 ) {
    result = Et96MapV1SendRInfoForSmReq(dialog->ssn, dialog_id, 0, &dialog->m_msAddr,
      hiPrior ? ET96MAP_ATTEMPT_DELIVERY : ET96MAP_DO_NOT_ATTEMPT_DELIVERY,
      &dialog->m_scAddr, 0, 0);
  }else throw runtime_error(
    FormatText("MAP::SendRInfo: incorrect dialog version %d",dialog->version));
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendRInfo: Et96MapVxSendRInfoForSmReq error 0x%x",result),MAP_FALURE);
  }
  result = Et96MapDelimiterReq(dialog->ssn, dialog_id, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendRInfo: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
  }
}

void ResponseMO(MapDialog* dialog,unsigned status)
{
  ET96MAP_ERROR_FORW_SM_MO_T err ;
  memset(&err,0,sizeof(ET96MAP_ERROR_FORW_SM_MO_T));
  dialog->wasDelivered = true;
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
    if( Status::isErrorPermanent(status) ) {
      err.errorCode = 21;
    } else {
      err.errorCode = 36;
    }
    break;
  };
  __map_trace2__("MAP::%s errCode=0x%x status=%d (state %d) ",__func__,err.errorCode,status,dialog->state);
  USHORT_T result;
  if ( dialog->version == 3 ) {
    ET96MAP_SM_RP_UI_T ui;
    ui.signalInfoLen=0;
    result = Et96MapV3ForwardSmMOResp(
      dialog->ssn,
      dialog->dialogid_map,
      dialog->invokeId,
      &ui,
      (status!=Status::OK)?&err:0);
  } else if ( dialog->version == 2 ) {
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
    __map_warn2__("ResponseMO: Et96MapVxForwardSmMOResp return error 0x%hx",result);
  }
  unsigned INVALID = (unsigned)-1;
  if ( dialog->udhiRef != INVALID )
  {
    MutexGuard guard(x_momap_lock);
    XMOMAPLocker* locker;
    __map_trace2__("UDHI:%s: find locker with imsi %s, ref %x",__func__,dialog->s_imsi.c_str(),dialog->udhiRef);
    XMOMAP::iterator it = x_momap.find(dialog->s_imsi);
    if ( it == x_momap.end() )
    {
      __map_trace2__("UDHI:%s: create locker",__func__);
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
      __map_trace2__("UDHI:%s: update locker part %d/%d",__func__,dialog->udhiMsgNum,dialog->udhiMsgCount);
      ++locker->parts;
      if ( locker->parts >= dialog->udhiMsgCount ){
        x_momap.erase(dialog->s_imsi);
        __map_trace2__("UDHI:%s: unlock part ",__func__);
      }else
        locker->startTime = time(0);
    }
    else
    {
      __map_trace2__("UDHI:%s: update locker part %d/%d",__func__,dialog->udhiMsgNum,dialog->udhiMsgCount);
      locker->ref = dialog->udhiRef;
      locker->parts = 1;
      locker->startTime = time(0);
      //locker->imsi
    }
  }
}

static void ForwardMO(MapDialog* dialog) {
  ET96MAP_ADDRESS_T fwdAddr;
  ET96MAP_SS7_ADDR_T destAddr;
  ET96MAP_SM_RP_DA_T smRpDa;
  ET96MAP_SM_RP_OA_T smRpOa;
  ET96MAP_SM_RP_UI_T ui;

  smsc::sms::SMS *sms = dialog->sms.get();
  smsc::sms::Address addr(sms->getStrProperty(Tag::SMSC_FORWARD_MO_TO).c_str());
  if( !sms->hasBinProperty(Tag::SMSC_MO_PDU) )
    throw MAPDIALOG_FATAL_ERROR("MAP::ForwardMO: SMS has SMSC_FORWARD_MO_TO but has no SMSC_MO_PDU",smsc::system::Status::DATAMISSING);

  dialog->state = MAPST_WaitFwdMOOpenConf;
  dialog->version = 2;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_MO_RELAY;
  SetVersion(appContext,dialog->version);

  mkMapAddress( &fwdAddr, addr.value, addr.length );
  mkSS7GTAddress( &destAddr, &fwdAddr, 8 );

  mkRP_DA_Address( &smRpDa, addr.value, addr.length, ET96MAP_ADDRTYPE_SCADDR );
  mkRP_OA_Address( &smRpOa, sms->getOriginatingAddress().value, sms->getOriginatingAddress().length, ET96MAP_ADDRTYPE_MSISDN );
  unsigned length;
  const char* mo_pdu = sms->getBinProperty(Tag::SMSC_MO_PDU, &length);

  memcpy(ui.signalInfo, mo_pdu, length );
  ui.signalInfoLen = (UCHAR_T)length;

  if( smsc::logger::_map_cat->isDebugEnabled() ) {
   {
    char *text = new char[256*4+1];
    unsigned char *buf = (unsigned char *)&smRpDa;
    int buflen = smRpDa.addrLen+2;
    int k = 0;
    for ( int i=0; i<buflen; i++){
      k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
    }
    text[k]=0;
    __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN, "rpda: %s",text);

    buf = (unsigned char *)&smRpOa;
    buflen = smRpOa.addrLen+2;
    k = 0;
    for ( int i=0; i<buflen; i++){
      k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
    }
    text[k]=0;
    __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN, "rpoa: %s",text);
    
    buf = (unsigned char *)&ui;
    buflen = ui.signalInfoLen+1;
    k = 0;
    for ( int i=0; i<buflen; i++){
      k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
    }
    text[k]=0;
    __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN, "ui: %s",text);
    delete text;
   }
  }


  __map_trace2__("MAP:: ForwardMO: forwarding msg %s->%s to sc: %s", sms->getOriginatingAddress().toString().c_str(), sms->getDestinationAddress().toString().c_str(), addr.toString().c_str() );

  USHORT_T result = Et96MapOpenReq( SSN, dialog->dialogid_map, &appContext, &destAddr, &dialog->scAddr, 0, 0, 0 );
  if ( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::ForwardMO: Et96MapOpenReq error 0x%x",result),MAP_FALURE);

  result = Et96MapV2ForwardSmMOReq(SSN,dialog->dialogid_map, 1, &smRpDa, &smRpOa, &ui);
  if ( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::ForwardMO: Et96MapV2ForwardSmMOReq error 0x%x",result),MAP_FALURE);
  
  result = Et96MapDelimiterReq(SSN,dialog->dialogid_map, 0, 0);
  if ( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::ForwardMO: Et96MapV2ForwardSmMOReq error 0x%x",result),MAP_FALURE);
}

static inline
unsigned ParseSemiOctetU(unsigned char v)
{
  unsigned result = (v>>4)*10+(v&0x0f);
  return result;
}

static void AttachSmsToDialog(MapDialog* dialog,ET96MAP_SM_RP_UI_T *ud,ET96MAP_SM_RP_OA_T *srcAddr)
{
  Address src_addr;
  ConvAddrMSISDN2Smc(srcAddr,&src_addr);
  if( smsc::logger::_map_cat->isDebugEnabled() )
  {
    char text[sizeof(*ud)*4] = {0,};
    int k = 0;
    for ( int i=0; i<ud->signalInfoLen; ++i){
      k+=sprintf(text+k,"%02x ",(unsigned)ud->signalInfo[i]);
    }
    __map_trace2__("%s:dialog 0x%x received PDU from: %s signalInfoLen 0x%x: %s", __func__, dialog->dialogid_map, src_addr.toString().c_str(), ud->signalInfoLen, text );
  }
  if( ud->signalInfo[0]&0x3 == 0 ) { // SMS_DELIVER_REPORT
    throw runtime_error( "SMS_DELIVER_REPORT recieved but not supported in this version" );
  } else if(ud->signalInfo[0]&0x3 == 2 ) {
    throw runtime_error( "SMS_COMMAND recieved but not supported in this version" );
  }
  auto_ptr<SMS> _sms ( new SMS() );
  SMS& sms = *_sms.get();
  sms.setBinProperty(Tag::SMSC_MO_PDU, (const char *)ud->signalInfo, ud->signalInfoLen );
  Address dest_addr;
  SMS_SUMBMIT_FORMAT_HEADER* ssfh = (SMS_SUMBMIT_FORMAT_HEADER*)ud->signalInfo;
  MAP_SMS_ADDRESS* msa = (MAP_SMS_ADDRESS*)(ud->signalInfo+2);
  if( msa->len > 20 ) throw runtime_error( "Address length is invalid in received PDU" );
  unsigned msa_len = msa->len/2+msa->len%2+2;
  unsigned char protocol_id = *(unsigned char*)(ud->signalInfo+2+msa_len);
  unsigned char user_data_coding = *(unsigned char*)(ud->signalInfo+2+msa_len+1);
  sms.setIntProperty(Tag::SMSC_ORIGINAL_DC,user_data_coding);
  unsigned tpvpLen = (ssfh->u.head.tp_vp==0)?0:(ssfh->u.head.tp_vp==2)?1:7;
  unsigned char user_data_len = *(unsigned char*)(ud->signalInfo+2+tpvpLen+msa_len+2);

/*  if( smsc::logger::_map_cat->isDebugEnabled() )
  {
    __map_trace2__("MR(8) = 0x%x",ssfh->mr);
    __map_trace2__("MSG_TYPE_IND(2) = 0x%x",ssfh->u.head.mg_type_ind);
    __map_trace2__("MSG_VPF(2) = 0x%x",ssfh->u.head.tp_vp);
    __map_trace2__("MSG_UDHI(1) = %d",ssfh->u.head.udhi);
    __map_trace2__("MSG_REJECT_DUPL(1) = %d",ssfh->u.head.reject_dupl);
    __map_trace2__("MSG_REPLY_PATH(1) = %d",ssfh->u.head.reply_path);
    __map_trace2__("MSG_SRR(1) = %d",ssfh->u.head.srr);
    __map_trace2__("MSA tonpi.va(8):0x%x, ton(3):0x%x, npi(4):0x%x, len(8):%d, msa_len=%d",
               msa->tonpi,msa->st.ton,msa->st.npi,msa->len, msa_len);
    __map_trace2__("protocol_id = 0x%x",protocol_id);
    __map_trace2__("user_data_encoding = 0x%x",user_data_coding);
    __map_trace2__("tp_vp len= %d", tpvpLen);
    __map_trace2__("user_data_len = %d",user_data_len);
  }*/
  unsigned max_data_len = (ud->signalInfoLen-(2+tpvpLen+msa_len+2+1) );
  unsigned char* user_data = (unsigned char*)(ud->signalInfo+2+tpvpLen+msa_len+2+1);
  if ( ssfh->u.head.tp_vp != 0 )
  {
    unsigned char* tvp = (unsigned char*)(ud->signalInfo+2+msa_len+1+1);
//    __map_trace2__("TVP = 0x%x , first octet 0x%x",(unsigned)ssfh->u.head.tp_vp,(unsigned)*tvp);
    time_t timeValue = time(0);
    if ( ssfh->u.head.tp_vp == 2 ){
parse_tvp_scheme1:
      if ( *tvp <= 143 ) timeValue+=(*tvp+1)*(5*60);
      else if ( *tvp <= 167 ) timeValue+=(12*60*60)+((*tvp-143)*(30*60));
      else if ( *tvp <= 196 ) timeValue+=(*tvp-166)*(60*60*24);
      else /*if ( *tvp <= 255 )*/ timeValue+=(*tvp-192)*(60*60*24*7);
    }else if (ssfh->u.head.tp_vp == 1 ){
      if (tpvpLen!=7)
        throw runtime_error(FormatText("MAP:%s:incorrect tpvp data",__func__));
parse_tvp_scheme2:
      struct tm dtm;
      dtm.tm_year = ParseSemiOctetU(tvp[0]);
      dtm.tm_mon  = ParseSemiOctetU(tvp[1]);
      dtm.tm_mday = ParseSemiOctetU(tvp[2]);
      dtm.tm_hour = ParseSemiOctetU(tvp[3]);
      dtm.tm_min  = ParseSemiOctetU(tvp[4]);
      dtm.tm_sec  = ParseSemiOctetU(tvp[5]);
      unsigned tzOctet = ParseSemiOctetU(tvp[6]);
      int timeZoneX = tzOctet&0x080?tzOctet:(0-(int)(tzOctet&0x07f));
//      __map_trace2__("TVP: %d:%d:%d:%d:%d:%d:%d",dtm.tm_year,dtm.tm_mon,dtm.tm_mday,dtm.tm_hour,dtm.tm_min,dtm.tm_sec,tzOctet);
      if (!( dtm.tm_mon >= 1 && dtm.tm_mon <= 12 )) throw runtime_error("bad month");
      dtm.tm_mon-=1;
      if ( !(dtm.tm_year >= 0 && dtm.tm_year <= 99) ) throw runtime_error("bad year");
      dtm.tm_year+=100; // year = x-1900
      if ( !(dtm.tm_mday >= 1 && dtm.tm_mday <= 31) ) throw runtime_error("bad mday");
      if ( !(dtm.tm_hour >= 0 && dtm.tm_hour <= 23) ) throw runtime_error("bad hour");
      if ( !(dtm.tm_min >= 0 && dtm.tm_min <= 59) ) throw runtime_error("bad min");
      if ( !(dtm.tm_sec >=0 && dtm.tm_sec <= 59) ) throw runtime_error("bad sec");
      timeValue = mktime(&dtm);
      if ( !(timeValue != -1) ) throw runtime_error("invalid time");
      timeValue -= timeZoneX*900;
      timeValue -= timezone;
    }else if (ssfh->u.head.tp_vp == 3 ){
      unsigned char tags = *tvp;
      unsigned valForm = tags&0x7;
      unsigned char* dta = tvp;
      while ( *dta & 0x080 ) { ++dta; if ( dta-tvp == 7 ) throw runtime_error("out of tvp data"); }
//      __map_trace2__("tvp 0x%x dta 0x%x valForm 0x%x",tvp,dta,valForm);
      switch(valForm)
      {
      case 0:
//        __map_trace__("tpvp enhanced has no validity");
        goto none_validity;
      case 0x1:
        if ( dta-tvp > 6 ) throw runtime_error("out of tvp data");
        tvp = dta;
        goto parse_tvp_scheme1;
        break;
      case 0x2:
        if ( *dta == 0 ) {
//          __map_trace__("tpvp enhanced has relative value 0 assumed as novalidity");
          goto none_validity;
        }
        timeValue+=(unsigned)*dta;
        break;
      case 0x3:
        if ( dta-tvp > 4 ) throw runtime_error("out of tvp data");
        timeValue+=((unsigned)dta[0])*60*60+((unsigned)dta[1])*60+((unsigned)dta[2]);
        break;
      default:
        throw runtime_error("bad tp_vp enhanced format");
      }
    }
//    __map_trace2__("set validity 0x%x (time:0x%x)",timeValue,time(0));
    sms.setValidTime(timeValue);
none_validity:;
  }
  unsigned encoding = 0;

// PARSE __DATACODING__

  if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
       (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
  {
    if ( user_data_coding&(1<<5) ){
      __map_warn__("required compression!!!");
      throw runtime_error("MAP:: required compression");
    }
    encoding = user_data_coding&0x0c;
    if ( (user_data_coding & 0xc0) == 0x40 ) // 01xxxxxx
      sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x03);
    if ( user_data_coding & 0x10 ) // xxx1xxxx
      sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
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
// ----- changed 25.07.2003
//    if ( user_data_coding & 0x4 ) encoding = MAP_8BIT_ENCODING;
//    else encoding = MAP_OCTET7BIT_ENCODING;
//    sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
// -----
    encoding = user_data_coding&0x0c;
    sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
  }
  else{
    __map_warn2__("unknown coding scheme 0x%x",user_data_coding);
    throw runtime_error("unknown coding scheme");
  }

  if ( encoding == MAP_OCTET7BIT_ENCODING )
  {
    unsigned octet_data_len = (user_data_len+1)*7/8;
    if ( octet_data_len > max_data_len )
      throw runtime_error(FormatText("bad user_data_len %d must be <= %d, PDU len=%d",octet_data_len,max_data_len,ud->signalInfoLen));
    if ( ssfh->u.head.udhi){
      unsigned udh_len = ((unsigned)*user_data)&0x0ff;
      if ( udh_len >= octet_data_len )
        throw runtime_error(FormatText("MAP:: octet_data_len %d, but udhi_len %d",octet_data_len,udh_len));
    }
  }
  else
  {
    if ( user_data_len > max_data_len )
      throw runtime_error(FormatText("bad user_data_len %d must be <= %d, PDU len=%d",user_data_len,max_data_len,ud->signalInfoLen));
    if ( ssfh->u.head.udhi){
      unsigned udh_len = ((unsigned)*user_data)&0x0ff;
      if ( udh_len >= user_data_len )
        throw runtime_error(FormatText("MAP:: user_data_len %d, but udhi_len %d",user_data_len,udh_len));
    }
  }

  {
    if (  encoding == MAP_OCTET7BIT_ENCODING ){
      if ( ssfh->u.head.udhi){
        MicroString ms;
        auto_ptr<unsigned char> b(new unsigned char[255*2]);
        unsigned udh_len = ((unsigned)*user_data)&0x0ff;
//        __map_trace2__("ud_length 0x%x udh_len 0x%x",user_data_len,udh_len);
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = user_data_len-x/7;
//        __map_trace2__("text symbols 0x%x bit offset 0x%x",symbols,x-(udh_len+1)*8);
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
    if ( ssfh->u.head.udhi )
    {
      unsigned ref = INVALID;
      unsigned msgNum = INVALID;
      unsigned msgCount = INVALID;

      unsigned len = ((unsigned)*user_data)&0x0ff;
//      __map_trace2__("UDHI: udh length %d",len);
      unsigned char* udh = user_data+1;
      unsigned ptr = 0;
      for (; ptr+2 < len; ptr+=2 )
      {
        unsigned elLength = udh[ptr+1];
//        __map_trace2__("UDHI: ptr %d, tag %d, len %d",ptr,udh[ptr],elLength);
        if ( udh[ptr] == 0 || udh[ptr] == 8)
        {
          if ( udh[ptr] == 0 ) {
            ref = udh[ptr+2];
            msgCount = udh[ptr+3];
            msgNum   = udh[ptr+4];
            break;
          }else{
            //ref = ntohs(*(unsigned short*)(udh+ptr+2));
            unsigned short tmpVal;
            memcpy(&tmpVal,udh+ptr+2,2);
            ref = ntohs(tmpVal);
            msgCount = udh[ptr+4];
            msgNum   = udh[ptr+5];
            break;
          }
        }
        else ptr+=elLength;
      }
      dialog->udhiRef = ref;
      dialog->udhiMsgNum = msgNum;
      dialog->udhiMsgCount = msgCount;
//      __map_trace2__("UDHI: ref %x, msgNum %d, msgCont %d ",ref,msgNum,msgCount);
    }
    else
    {
      dialog->udhiRef = INVALID;
      dialog->udhiMsgNum = INVALID;
      dialog->udhiMsgCount = INVALID;
    }
  }
  unsigned esm_class = 0;
  esm_class |= (ssfh->u.head.udhi?0x40:0);
  esm_class |= (ssfh->u.head.reply_path?0x80:0);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
  //sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
  sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
  sms.setMessageReference(ssfh->mr);
  if ( ssfh->u.head.srr ) sms.setIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST,1);
  sms.setOriginatingAddress(src_addr);
  ConvAddrMap2Smc(msa,&dest_addr);
  sms.setDestinationAddress(dest_addr);
  dialog->sms = _sms;
}

static void SendSubmitCommand(MapDialog* dialog)
{
  if ( dialog->sms.get() == 0 )
    throw runtime_error("MAP::hereis no SMS for submiting");
  Descriptor desc;
  desc.setImsi(dialog->s_imsi.length(),dialog->s_imsi.c_str());
  desc.setMsc(dialog->s_msc.length(),dialog->s_msc.c_str());
  dialog->sms->setOriginatingDescriptor(desc);
  if ( dialog->isUSSD ) {
    Address src_addr;
    ConvAddrMap2Smc((const MAP_SMS_ADDRESS*)&dialog->m_msAddr,&src_addr);
    dialog->sms->setOriginatingAddress(src_addr);
    istringstream(src_addr.value)>>dialog->ussdSequence;
    {
      MutexGuard ussd_map_guard( ussd_map_lock );
      ussd_map[dialog->ussdSequence] = dialog->dialogid_map;
    }
  }
  __map_trace2__("Submit %s to SMSC: IMSI = %s, MSC = %s, %s",dialog->isUSSD?"USSD":"SMS",dialog->s_imsi.c_str(),dialog->s_msc.c_str(),RouteToString(dialog).c_str());

  try{
    MscManager::getMscStatus().report(dialog->s_msc.c_str(),true);
  }
  catch(exception& e)
  {
    __map_warn2__("<exception>: %s",e.what());
  }

  MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
  unsigned ssn_var = (dialog->ssn == USSD_SSN) ? 0x10000 : 0;
  SmscCommand cmd = SmscCommand::makeSumbmitSm(
    *dialog->sms.get(),(unsigned(dialog->dialogid_map)&0xffff)|ssn_var);
  proxy->putIncomingCommand(cmd);
}

static const unsigned DIALOGID_BEFORE_CREATING = 0x10000;

static void TryDestroyDialog(unsigned dialogid,bool send_error,unsigned err_code,unsigned ssn)
{
  {
    __map_trace2__("TryDestroyDialog: dialog 0x%x , reason error",dialogid);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid,ssn));
    if ( dialog.isnull() ) {
      __map_trace2__("TryDestroyDialog: has no dialog 0x%x",dialogid);
      return;
    }
    __require__(dialog->ssn==ssn);
    __map_trace2__("TryDestroyDialog: dialogid 0x%x state %d",dialog->dialogid_map,dialog->state);
    if ( send_error)
    {
      dialog->dropChain = true;
      try{
        if ( dialog->isQueryAbonentStatus && dialog->QueryAbonentCommand.IsOk() ){
          int status;
          status = AbonentStatus::UNKNOWNVALUE;
          SendAbonentStatusToSmsc(dialog.get(),status);
        }else{
          if( dialog->state != MAPST_WaitNextMMS ) {
            SendErrToSmsc(dialog->dialogid_smsc,err_code);
          }
        }
      }catch(...){
        __map_warn__("TryDestroyDialog: catched exception when send error response to smsc");
      }
    }
    if ( send_error ){
      __map_trace2__("ABORT: dialog 0x%x",dialog->dialogid_map);
      if ( dialog->id_opened ){
        AbortMapDialog(dialog->dialogid_map,dialog->ssn);
        dialog->id_opened = false;
      }
    }else{
      switch(dialog->state){
      case MAPST_ABORTED:
        __map_trace2__("ABORT: dialog 0x%x",dialog->dialogid_map);
        if ( dialog->id_opened ) {
          AbortMapDialog(dialog->dialogid_map,dialog->ssn);
          dialog->id_opened = false;
        }
        break;
      default:
        CloseMapDialog(dialog->dialogid_map,dialog->ssn);
      }
    }
  }
  DropMapDialog_(dialogid,ssn);
}

static string RouteToString(MapDialog* dialog)
{
  static string skiped("<unknown> -> <unknown>");
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
}

#define MAP_TRY  try {
#define MAP_CATCH(__dialogid_map,__dialogid_smsc,__ssn)     \
}catch(MAPDIALOG_HEREISNO_ID& x){\
  __map_warn2__("%s: here is no dialogid 0x%x/0x%x <exception>:%s",__func__,__dialogid_map,__dialogid_smsc,x.what());\
}catch(MAPDIALOG_ERROR& err){\
  __map_trace2__("%s: error dialogid 0x%x/0x%x <exception>:%s",__func__,__dialogid_map,__dialogid_smsc,err.what());\
  TryDestroyDialog(__dialogid_map,true,err.code,__ssn);\
}catch(exception& e){\
  __map_trace2__("%s: exception dialogid 0x%x/0x%x <exception>:%s",__func__,__dialogid_map,__dialogid_smsc, e.what());\
  TryDestroyDialog(__dialogid_map,true,MAKE_ERRORCODE(CMD_ERR_FATAL,0),__ssn);\
}

static bool SendSms(MapDialog* dialog){
  CheckLockedByMO(dialog);
  dialog->wasDelivered = false;

  if ( !MscManager::getMscStatus().check(dialog->s_msc.c_str()) )
    throw MAPDIALOG_TEMP_ERROR("MSC BLOCKED",Status::BLOCKEDMSC);

  bool mms = FALSE;
  if( dialog->version > 1 ) {
    if( dialog->chain.size() != 0 ) {
      mms = TRUE;
    } else if( dialog->sms.get()->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND) ) {
      mms = TRUE;
    }
  }
  if ( dialog->version < 2 ) mms = false;
//  mms = false;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_MT_RELAY;
  SetVersion(appContext,dialog->version);
  USHORT_T result;
  bool segmentation = false;
  __map_trace2__("%s: chain size is %d mms=%d dlg->mms=%s dlg->invoke=%d",__func__,dialog->chain.size(),mms,dialog->mms?"true":"false", (int)dialog->invokeId);

  dialog->state = MAPST_WaitSmsConf;
  if ( !dialog->mms ) {
    dialog->invokeId = 0;
    dialog->state = MAPST_WaitOpenConf;
    result = Et96MapOpenReq(SSN,dialog->dialogid_map,&appContext,&dialog->destMscAddr,&dialog->scAddr,0,0,0);
    if ( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::SendSms: Et96MapOpenReq error 0x%x",result),MAP_FALURE);
    dialog->id_opened = true;
  }

  dialog->smRpOa.typeOfAddress = ET96MAP_ADDRTYPE_SCADDR;
  dialog->smRpOa.addrLen = (dialog->m_scAddr.addressLength+1)/2+1;
  dialog->smRpOa.addr[0] = dialog->m_scAddr.typeOfAddress;
  memcpy( dialog->smRpOa.addr+1, dialog->m_scAddr.address, (dialog->m_scAddr.addressLength+1)/2 );

  ET96MAP_SM_RP_UI_T* ui;
  dialog->auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(ui=new ET96MAP_SM_RP_UI_T);
  mkDeliverPDU(dialog->sms.get(),ui,mms);
  if ( dialog->version > 1 
       && !dialog->mms 
       && (ui->signalInfoLen > (143-(dialog->smRpOa.addrLen+1)-(dialog->smRpDa.addrLen+1)-(dialog->scAddr.ss7AddrLen)) 
       || mms ) ) {
    result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
    segmentation = true;
    dialog->state = MAPST_WaitSpecOpenConf;
  }else{
    dialog->invokeId++;
    if ( dialog->version == 2 ) {
      result = Et96MapV2ForwardSmMTReq( dialog->ssn, dialog->dialogid_map, dialog->invokeId, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), mms?TRUE:FALSE);
    }else if ( dialog->version == 1 ){
      result = Et96MapV1ForwardSmMT_MOReq( dialog->ssn, dialog->dialogid_map, dialog->invokeId, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get());
    }else throw runtime_error(
      FormatText("MAP::SendSMSCToMT: incorrect dialog version %d",dialog->version));
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT: Et96MapVxForwardSmMTReq error 0x%x",result),MAP_FALURE);
    result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
  }
  dialog->mms = mms;
  return segmentation;
}

static void SendNextMMS(MapDialog* dialog)
{
  __map_trace2__("%s: dialogid 0x%x  (state %d/NEXTMMS)",__func__,dialog->dialogid_map,dialog->state);
  if ( dialog->chain.size() == 0 ) {
    if( dialog->sms.get()->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND) ) {
      if( !dialog->wasDelivered ) {
        __map_trace__("SendNextMMS: messages was not delivered. Aborting long message sending");
        AbortMapDialog( dialog->dialogid_map, dialog->ssn );
        DropMapDialog(dialog);
        return;
      } else {
        __map_trace2__("SendNextMMS: no messages in chain. Waiting long message next part for %d", dialog->sms.get()->getConcatMsgRef());
        dialog->state = MAPST_WaitNextMMS;
        return;
      }
    } else throw runtime_error("MAP::SendNextMMS: has no messages to send");
  }
  SmscCommand cmd = dialog->chain.front();
  dialog->chain.pop_front();
  dialog->dialogid_smsc = cmd->get_dialogId();
  dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
  SendSms(dialog);
}

static void SendSegmentedSms(MapDialog* dialog)
{
  USHORT_T result;
// We have already opened dialog and don't need to check this condition.
//  CheckLockedByMO(dialog);
  dialog->invokeId++;
  if ( dialog->version == 2 ) {
    result = Et96MapV2ForwardSmMTReq( dialog->ssn, dialog->dialogid_map, dialog->invokeId, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), dialog->mms);
  }else if ( dialog->version == 1 ) {
//    result = Et96MapV1ForwardSmMT_MOReq( dialog->ssn, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get());
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSegmentedSms: MAPv1 do not support/required segmentation dialogid 0x%x",dialog->dialogid_map),MAP_FALURE);
  }else throw runtime_error(
    FormatText("MAP::SendSegmentedSms: incorrect dialog version %d",dialog->version));
  if( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSegmentedSms: Et96MapVxForwardSmMTReq error 0x%x",result),MAP_FALURE);
  result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
  if( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSegmentedSms: Et96MapDelimiterReq error 0x%x",result),MAP_FALURE);
}

static void DoUSSRUserResponceError(const SmscCommand* cmd , MapDialog* dialog)
{
  __map_trace2__("%s: dialogid 0x%x",__func__,dialog->dialogid_map);
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0;
  ET96MAP_ERROR_PROCESS_UNSTRUCTURED_SS_REQUEST_T error;
  error.errorCode = 34; /*Sytem failure */
  ET96MAP_USSD_STRING_T ussdString = {0,};
  //memset(&ussdString,0,sizeof(ussdString));
  UCHAR_T result;
  if ( dialog->version == 2 )
  {
    result = Et96MapV2ProcessUnstructuredSSRequestResp(
      dialog->ssn,dialog->dialogid_map,dialog->origInvokeId,
      &ussdEncoding,
      &ussdString,
      &error);
  }
  else throw runtime_error(
    FormatText("MAP::%s incorrect dialog version %d",__func__,dialog->version));
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::%s Resp return error 0x%x",__func__,result));
  CloseMapDialog(dialog->dialogid_map,dialog->ssn);
  {
    MutexGuard ussd_map_guard( ussd_map_lock );
    __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
    ussd_map.erase(dialog->ussdSequence);
  }
  dialog->dropChain = true;
  dialog->state = MAPST_END;
  if( cmd != 0 ) SendOkToSmsc(dialog);
  DropMapDialog(dialog);
}

static long long NextSequence()
{
  static long long sequence = (((long long)time(0))<<32);
  return ++sequence;
}

static void DoUSSRUserResponce( MapDialog* dialog)
{
  __map_trace2__("%s: dialogid 0x%x",__func__,dialog->dialogid_map);
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0x0f;
  unsigned encoding = dialog->sms->getIntProperty(Tag::SMPP_DATA_CODING);
  ET96MAP_USSD_STRING_T ussdString = {0,};
  unsigned text_len;

  const unsigned char* text = (const unsigned char*)dialog->sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
  if(text_len==0 && dialog->sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    text=(const unsigned char*)dialog->sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&text_len);
  }

  if ( text_len > 160 )
    throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} very long msg text %d",__func__,dialog->dialogid_map,text_len));

  unsigned bytes = 0;
  if( encoding == MAP_UCS2_ENCODING ) {
    bytes = text_len;
    memcpy( ussdString.ussdStr, text, text_len );
    ussdEncoding = 0x48;
  } else if( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING || encoding == MAP_SMSC7BIT_ENCODING ) {
    if (encoding == MAP_SMSC7BIT_ENCODING ) {
      bytes = ConvertSMSC7bit27bit(text,text_len,ussdString.ussdStr,0);
    } else {
      unsigned elen = 0;
      bytes = ConvertText27bit(text,text_len,ussdString.ussdStr,&elen);
    }
    // if buffer have trailing 7 unfilled bits place <cr> there
    if( bytes*8-text_len*7 == 7 ) ussdString.ussdStr[bytes-1] |= (0x0D<<1);
    ussdEncoding = 0x01;
  } else { //8 bit
    bytes = text_len;
    memcpy( ussdString.ussdStr, text, text_len );
    ussdEncoding = 0x44;
  }
/*  if( smsc::logger::_map_cat->isDebugEnabled() ) {
    char *buf = new char[text_len*4+1];
    int k = 0;
    for ( int i=0; i<bytes; i++){
      k+=sprintf(buf+k,"%02x ",(unsigned)text[i]);
    }
    buf[k]=0;
    __map_trace2__("USSD orig string enc=0x%02X dump: %s",encoding,buf);
    delete buf;

    buf = new char[bytes*4+1];
    k = 0;
    for ( int i=0; i<bytes; i++){
      k+=sprintf(buf+k,"%02x ",(unsigned)ussdString.ussdStr[i]);
    }
    buf[k]=0;
    __map_trace2__("USSD string ussdenc=0x%02X dump: %s",ussdEncoding,buf);
    delete buf;
  }*/

  ussdString.ussdStrLen = bytes;
  UCHAR_T result;
  if ( dialog->version == 2 )
  {
    result = Et96MapV2ProcessUnstructuredSSRequestResp(
      dialog->ssn,dialog->dialogid_map,dialog->origInvokeId,
      &ussdEncoding,
      &ussdString,
      0);
  }
  else throw runtime_error(
    FormatText("MAP::%s incorrect dialog version %d",__func__,dialog->version));
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::%s Resp return error 0x%x",__func__,result));
  CloseMapDialog(dialog->dialogid_map,dialog->ssn);
  {
    MutexGuard ussd_map_guard( ussd_map_lock );
    __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
    ussd_map.erase(dialog->ussdSequence);
  }
  dialog->state = MAPST_END;
  SendOkToSmsc(dialog);
  DropMapDialog(dialog);
}

static void DoUSSDRequestOrNotifyReq(MapDialog* dialog)
{
  dialog->isUSSD = true;
  __map_trace2__("%s: dialogid 0x%x opened=%s invoke=%d ssn=%d",__func__,dialog->dialogid_map,dialog->id_opened?"true":"false", dialog->invokeId, dialog->ssn);
  if( !dialog->id_opened ) {
    bool dlg_found = false;
    istringstream(string(dialog->sms->getDestinationAddress().value))>>dialog->ussdSequence;
    {
      MutexGuard ussd_map_guard( ussd_map_lock );
      USSD_MAP::iterator it = ussd_map.find(dialog->ussdSequence);
      if ( it != ussd_map.end() ) {
        // USSD dialog already exists on this abonent
        dlg_found = true;
      } else {
        ussd_map[dialog->ussdSequence] = dialog->dialogid_map;
      }
    }
    if(dlg_found) {
      __map_trace2__("%s: dialogid 0x%x, ussd dialog already exists for %s",__func__,dialog->dialogid_map,dialog->abonent.c_str());
      SendErrToSmsc(dialog->dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::SUBSCRBUSYMT));
      dialog->state = MAPST_END;
      DropMapDialog(dialog);
      return;
    }
  }
  ET96MAP_USSD_STRING_T ussdString = {0,};
  unsigned text_len;
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0x0f;
  unsigned encoding = dialog->sms->getIntProperty(Tag::SMPP_DATA_CODING);

  const unsigned char* text = (const unsigned char*)dialog->sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
  if(text_len==0 && dialog->sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    text=(const unsigned char*)dialog->sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&text_len);
  }
  if ( text_len > 160 )
    throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} very long msg text %d",__func__,dialog->dialogid_map,text_len));

  unsigned bytes = 0;
  if( encoding == MAP_UCS2_ENCODING ) {
    bytes = text_len;
    memcpy( ussdString.ussdStr, text, text_len );
    ussdEncoding = 0x48;
  } else if( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING || encoding == MAP_SMSC7BIT_ENCODING ) {
    if (encoding == MAP_SMSC7BIT_ENCODING ) {
      bytes = ConvertSMSC7bit27bit(text,text_len,ussdString.ussdStr,0);
    } else {
      unsigned elen = 0;
      bytes = ConvertText27bit(text,text_len,ussdString.ussdStr,&elen);
    }
    // if buffer have trailing 7 unfilled bits place <cr> there
    if( bytes*8-text_len*7 == 7 ) ussdString.ussdStr[bytes-1] |= (0x0D<<1);
    ussdEncoding = 0x01;
  } else { //8 bit
    bytes = text_len;
    memcpy( ussdString.ussdStr, text, text_len );
    ussdEncoding = 0x44;
  }
  if( smsc::logger::_map_cat->isDebugEnabled() ) {
    char *text = new char[bytes*4+1];
    int k = 0;
    for ( int i=0; i<bytes; i++){
      k+=sprintf(text+k,"%02x ",(unsigned)ussdString.ussdStr[i]);
    }
    text[k]=0;
    __map_trace2__("USSD string enc=0x%02X ussdenc=0x%02X bytes=%d dump: %s",encoding,ussdEncoding,bytes,text);
    delete text;
  }

  ussdString.ussdStrLen = bytes;
  int serviceOp = dialog->sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP);
  if( serviceOp == USSD_USSR_REQ ) {
    dialog->state = MAPST_WaitUSSDReqConf;
  } else {
    dialog->state = MAPST_WaitUSSDNotifyConf;
  }
  UCHAR_T result;
  if( !dialog->id_opened ) {
    dialog->ussdMrRef = MakeMrRef();
    ET96MAP_APP_CNTX_T appContext;
    appContext.acType = ET96MAP_NETWORK_UNSTRUCTURED_SS_CONTEXT;
    SetVersion(appContext, dialog->version);
    ET96MAP_USERDATA_T specificInfo;
    specificInfo.specificInfoLen=3+(dialog->m_msAddr.addressLength+1)/2;
    specificInfo.specificData[0] = 0x82;
    specificInfo.specificData[1] = 1+(dialog->m_msAddr.addressLength+1)/2;
    specificInfo.specificData[2] = dialog->m_msAddr.typeOfAddress;
    memcpy( specificInfo.specificData+3, dialog->m_msAddr.address, (dialog->m_msAddr.addressLength+1)/2 );

    if( serviceOp == USSD_USSR_REQ ) {
      dialog->state = MAPST_WaitUSSDReqOpenConf;
    } else {
      dialog->state = MAPST_WaitUSSDNotifyOpenConf;
    }
    dialog->invokeId=0;
    ET96MAP_ADDRESS_T origAddr;
    mkMapAddress( &origAddr, dialog->sms->getOriginatingAddress().value, dialog->sms->getOriginatingAddress().length );

    result = Et96MapOpenReq( dialog->ssn, dialog->dialogid_map, &appContext, &dialog->mshlrAddr, GetUSSDAddr(), 0, &origAddr, &specificInfo );
    if ( result != ET96MAP_E_OK )
      throw runtime_error(
        FormatText("MAP::%s Et96MapOpenReq return error 0x%x",__func__,result));
  }
  dialog->invokeId++;
  ET96MAP_ALERTING_PATTERN_T alertPattern = ET96MAP_ALERTING_PATTERN_LEVEL2;
  if( serviceOp == USSD_USSR_REQ ) {
    result = Et96MapV2UnstructuredSSRequestReq( dialog->ssn, dialog->dialogid_map, dialog->invokeId, ussdEncoding, ussdString, &alertPattern);
  } else {
    result = Et96MapV2UnstructuredSSNotifyReq( dialog->ssn, dialog->dialogid_map, dialog->invokeId, ussdEncoding, ussdString, &alertPattern);
  }
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::%s Et96MapV2UnstructuredSSRequestReq return error 0x%x",__func__,result));
  
  result = Et96MapDelimiterReq( dialog->ssn, dialog->dialogid_map, 0, 0 );
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::%s Et96MapDelimiterReq return error 0x%x",__func__,result));
}

void MAPIO_PutCommand(const SmscCommand& cmd ){
/*  if( MapDialogContainer::getInstance()->getNumberOfDialogs() > MAP_DIALOGS_LIMIT &&
      ( ( cmd->get_commandId() == DELIVERY && 
          !cmd->get_sms()->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP ) &&
          !MapDialogContainer::getInstance()->isWaitingNextMMS(cmd)
         ) || 
         cmd->get_commandId() == QUERYABONENTSTATUS
       )
     ) 
  {
    SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_TEMP,Status::THROTTLED));
  } else { */
    MAPIO_PutCommand(cmd, 0 );
/*  } */
}

static void MAPIO_PutCommand(const SmscCommand& cmd, MapDialog* dialog2 )
{
  unsigned dialogid_smsc = cmd->get_dialogId();
  unsigned dialogid_map = 0;
  unsigned dialog_ssn = 0;
  DialogRefGuard dialog;
  MAP_TRY {
    if( !isMapBound() ) throw runtime_error("MAP is not bound yet");
    if ( cmd->get_commandId() != SUBMIT_RESP ) { 
      if ( cmd->get_commandId() != DELIVERY && cmd->get_commandId() != QUERYABONENTSTATUS)
        throw MAPDIALOG_BAD_STATE("MAP::putCommand: must be DELIVERY or QUERYABONENTSTATUS");
      try
      {
        if ( cmd->get_commandId() == DELIVERY  && cmd->get_sms()->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP ) )
        {
          unsigned serviceOp = cmd->get_sms()->getIntProperty(Tag::SMPP_USSD_SERVICE_OP );
          __map_trace2__("putCommand dialogid_smsc /0x%x USSD OP %d",dialogid_smsc, serviceOp);
          string s_seq(cmd->get_sms()->getDestinationAddress().value);
          if (s_seq.length()==0) throw MAPDIALOG_FATAL_ERROR("MAP::PutCommand: empty destination address");
          long long sequence;
          istringstream(s_seq) >> sequence;
          if ( sequence == 0 )
            throw MAPDIALOG_FATAL_ERROR(
              FormatText("MAP::PutCommand: invalid sequence %s",s_seq.c_str()));
          try
          {
            bool dlg_found = false;
            {
              MutexGuard ussd_map_guard( ussd_map_lock );
              USSD_MAP::iterator it = ussd_map.find(sequence);
              if ( it != ussd_map.end() ) {
                dialogid_map = it->second;
                dialog_ssn = USSD_SSN;
                dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_map,dialog_ssn));
                if( !dialog.isnull() ) {
                  if ( !dialog->isUSSD )
                    throw MAPDIALOG_FATAL_ERROR(
                      FormatText("MAP::putCommand: Opss, NO ussd dialog with id x%x, seq: %s",dialogid_smsc,s_seq.c_str()));
                  __require__(dialog->ssn == dialog_ssn);
                  dlg_found = true;
                } else {
                  __map_trace2__("%s: ussd lock found for %lld dialogid 0x%x but no dialog exists, erase ussd lock", __func__,sequence,dialogid_map);
                  ussd_map.erase(sequence);
                }
              }
            }
            if ( serviceOp == USSD_PSSR_RESP )
            {
              if ( !dlg_found ) {
                SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND));
                throw MAPDIALOG_FATAL_ERROR( FormatText("MAP::putCommand: Opss, here is no dialog with id x%x seq: %s",dialogid_smsc,s_seq.c_str()));
              }
              {
                unsigned mr = cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)&0x0ffff;
                if ( dialog->ussdMrRef != mr ) {
                  SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND));
                  throw MAPDIALOG_FATAL_ERROR(
                    FormatText("MAP::putCommand: Opss, bad message_reference 0x%x must be 0x%x",
                      mr,dialog->ussdMrRef));
                }
              }
              if (dialog->state == MAPST_WaitSubmitCmdConf) {
                // Seems PSSR_RESP goes earlier than submitResp
                __map_trace2__("%s: dialogid 0x%x deliver earlier then submit resp for USSD dlg, deliver was chained", __func__,dialog->dialogid_map);
                dialog->chain.insert(dialog->chain.begin(), cmd);
                return;
              } else if ( !(dialog->state == MAPST_ReadyNextUSSDCmd || dialog->state == MAPST_USSDWaitResponce )) {
                throw MAPDIALOG_BAD_STATE(FormatText("MAP::%s ussd resp bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
              }
              dialog->dialogid_smsc = dialogid_smsc;
              dialog->isQueryAbonentStatus = false;
              dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
              DoUSSRUserResponce(dialog.get());
              return;
            } else if(serviceOp == USSD_USSR_REQ || serviceOp == USSD_USSN_REQ) {
              if( dlg_found ) {
                {
                  unsigned mr = cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)&0x0ffff;
                  if ( dialog->ussdMrRef != mr ) {
                    SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND));
                    throw MAPDIALOG_FATAL_ERROR(
                      FormatText("MAP::putCommand: Opss, bad message_reference 0x%x must be 0x%x",
                        mr,dialog->ussdMrRef));
                  }
                }
                dialog->id_opened = true;
                dialog->dialogid_smsc = dialogid_smsc;
                if (dialog->state == MAPST_WaitSubmitCmdConf) {
                  // Seems deliver goes earlier than submitResp
                  __map_trace2__("%s: dialogid 0x%x deliver earlier then submit resp for USSD dlg, deliver was chained", __func__,dialog->dialogid_map);
                  dialog->chain.insert(dialog->chain.begin(), cmd);
                  return;
                } else if ( !(dialog->state == MAPST_ReadyNextUSSDCmd || dialog->state == MAPST_USSDWaitResponce)) {
                  throw MAPDIALOG_BAD_STATE(
                    FormatText("MAP::%s ussd req/notify bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
                }
                dialog->isQueryAbonentStatus = false;
                dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
                DoUSSDRequestOrNotifyReq(dialog.get());
                return;
              } else {
                if( cmd->get_sms()->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ) {
                  SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND));
                  throw MAPDIALOG_FATAL_ERROR( FormatText("MAP::putCommand: Here is no USSD dialog for MR %d smsc_dlg 0x%x seq: %s",cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE), dialogid_smsc,s_seq.c_str()), Status::USSDDLGNFOUND);
                }
                __map_trace2__("%s: trying to create USSD network intiated session dialogid_smsc 0x%x",__func__,dialogid_smsc);
                try{
                  if( !dialog2 ) { // not chained dialog
                    try {
                      dialog.assign(MapDialogContainer::getInstance()->
                              createOrAttachSMSCUSSDDialog(
                                dialogid_smsc,
                                SSN,
                                string(cmd->get_sms()->getDestinationAddress().value),
                                cmd));
                    } catch (ChainIsVeryLong& e) {
                      __map_trace2__("%s: %s ",__func__,e.what());
                      SendErrToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MSGQFUL));
                      //throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
                      return;
                    } catch (exception& e) {
                      __map_trace2__("%s: %s ",__func__,e.what());
                      SendErrToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::THROTTLED));
                      return;//throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
                    }
                    if ( dialog.isnull() ) {
                      //throw MAPDIALOG_TEMP_ERROR("Can't create or attach dialog");
                      //SendRescheduleToSmsc(dialogid_smsc);
                      __map_trace2__("%s: was scheduled (state:NONE) dialogid_smsc 0x%x",__func__, dialogid_smsc);
                      return;
                    }
                  } else {
                    // command taken from chain
                    dialog_ssn = SSN;
                    dialog.assign(dialog2->AddRef());
                    dialogid_map = dialog->dialogid_map;
                    dialogid_map = MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,dialog->ssn,SSN);
                    __require__(dialog->ssn == dialog_ssn);
                    dialog->id_opened = false;
                    dialog->dialogid_smsc = dialogid_smsc;
                    dialog->dropChain = false;
                  }
                }catch(MAPDIALOG_ERROR& e){
                  throw;
                }catch(exception& e){
                  __map_warn2__("PutCommand: exception when create dialog. <exception>:%s",e.what());
                  //throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
                  throw MAPDIALOG_FATAL_ERROR("MAP::PutCommand: can't create dialog");
                }
                if( dialog.isnull() ) {
                  __map_trace__("putCommand: can't create SMSC->MS ussd dialog (locked), request has bean attached");
                  // command has bean attached by dialog container
                } else {
                  dialog->dropChain = false;
                  dialog->wasDelivered = false;
                  dialog->hlrWasNotified = false;
                  dialogid_map = dialog->dialogid_map;
                  dialog->state = MAPST_START;
                  __map_trace2__("%s: dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
                  StartDialogProcessing(dialog.get(),cmd);
                  return;
                }
              }
            } else {
              SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND));
              throw MAPDIALOG_FATAL_ERROR(
                FormatText("MAP::PutCommand: invaid USSD code 0x%x",serviceOp));
            }
          }
          catch(...)
          {
            MutexGuard ussd_map_guard( ussd_map_lock );
            __map_trace2__("erase ussd lock for %lld", sequence);
            ussd_map.erase(sequence);
            throw;
          }
        }else if ( !dialog2  ) {
          try {
            dialog_ssn = SSN;
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
          } catch (ChainIsVeryLong& e) {
            __map_trace2__("%s: %s ",__func__,e.what());
            SendErrToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MSGQFUL));
            //throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
            return;
          } catch (NextMMSPartWaiting& e) {
            __map_trace2__("%s: %s ",__func__,e.what());
            SendErrToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,Status::SUBSCRBUSYMT));
            return;
          } catch (exception& e) {
            __map_trace2__("%s: %s ",__func__,e.what());
            SendErrToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::THROTTLED));
            return;//throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
          }
          if ( dialog.isnull() ) {
            //throw MAPDIALOG_TEMP_ERROR("Can't create or attach dialog");
            //SendRescheduleToSmsc(dialogid_smsc);
            __map_trace2__("%s: was scheduled (state:NONE) dialogid_smsc 0x%x",__func__, dialogid_smsc);
            return;
          }
        }else{
          dialog_ssn = SSN;
          dialog.assign(dialog2->AddRef());
          dialogid_map = dialog->dialogid_map;
          dialogid_map = MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,dialog->ssn,SSN);
          __require__(dialog->ssn == dialog_ssn);
          dialog->id_opened = false;
          dialog->dialogid_smsc = dialogid_smsc;
          dialog->dropChain = false;
        }
      }catch(MAPDIALOG_ERROR& e){
        throw;
      }catch(exception& e){
        __map_warn2__("PutCommand: exception when create dialog. <exception>:%s",e.what());
        //throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
        throw MAPDIALOG_FATAL_ERROR("MAP::PutCommand: can't create dialog");
      }
      if ( dialog.isnull() ) {
        __map_trace__("putCommand: can't create SMSC->MS dialog (locked), request has bean attached");
        // command has bean attached by dialog container
      }else{
        dialog->isQueryAbonentStatus = (cmd->get_commandId() == QUERYABONENTSTATUS);
        if ( dialog->isQueryAbonentStatus )
        {
          dialog->QueryAbonentCommand = cmd;
          if ( dialog->QueryAbonentCommand->get_abonentStatus().addr.getLenght() == 0 )
            throw MAPDIALOG_FATAL_ERROR("incorrect address");
        }
        dialogid_map = dialog->dialogid_map;
        if( dialog->state != MAPST_SendNextMMS ) {
          dialog->state = MAPST_START;
          __map_trace2__("%s: dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
          StartDialogProcessing(dialog.get(),cmd);
        } else {
          dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
          SendSms(dialog.get());
        }
      }
    }else{ // !delivery !query abonent status it's submit resp
      dialogid_map = dialogid_smsc&0x0ffff;
      if ( ((dialogid_smsc >> 16)&0x3) == 1 ) dialog_ssn = USSD_SSN;
      else dialog_ssn = SSN; 
      if ( dialog2 ) throw runtime_error("MAP::putCommand can't chain MAPINPUT");
      dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_map,dialog_ssn));
      if ( dialog.isnull() ) {
/*        SendErrToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_FATAL,
          (dialog_ssn = USSD_SSN?Status::USSDDLGNFOUND:Status::DELIVERYTIMEDOUT)));*/
        __map_warn2__("MAP::putCommand: dialog not found for submit resp x0%x",dialogid_smsc);
        return;
      }
      //dialog->state = MAPST_START;
      dialogid_smsc = 0;
      __require__(dialog->ssn == dialog_ssn);
      __map_trace2__("%s: submit resp dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
      //dialogid_map = dialogid_smsc;
      if ( dialog->state == MAPST_WaitSubmitCmdConf ){
        if ( dialog->isUSSD )
        {
          dialog->state = MAPST_USSDWaitResponce;
          if ( cmd->get_resp()->get_status() != 0 )
          {
            DoUSSRUserResponceError(&cmd,dialog.get());
          } else {
            if( dialog->chain.size() > 0 ) {
              SmscCommand cmd_c = dialog->chain.front();
              dialog->chain.pop_front();
              __map_trace2__("%s found chained USSD deliver for that dialog", __func__);
              MAPIO_PutCommand(cmd_c, 0);
            }
          }
        }
        else
        {
          ResponseMO(dialog.get(),cmd->get_resp()->get_status());
          CloseMapDialog(dialog->dialogid_map,dialog->ssn);
          DropMapDialog(dialog.get());
        }
      }else if(dialog->state == MAPST_WaitSubmitUSSDRequestConf) {
        if ( cmd->get_resp()->get_status() != 0 )
        {
          {
            MutexGuard ussd_map_guard( ussd_map_lock );
            __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
            ussd_map.erase(dialog->ussdSequence);
          }
          dialog->dropChain = true;
          CloseMapDialog(dialog->dialogid_map,dialog->ssn);
          DropMapDialog(dialog.get());
        } else {
          dialog->state = MAPST_ReadyNextUSSDCmd;
          if( dialog->chain.size() > 0 ) {
            SmscCommand cmd_c = dialog->chain.front();
            dialog->chain.pop_front();
            __map_trace2__("%s found chained USSD deliver for that dialog", __func__);
            MAPIO_PutCommand(cmd_c, 0);
          }
        }
      }else if(dialog->state == MAPST_WaitSubmitUSSDRequestCloseConf) {
        {
          MutexGuard ussd_map_guard( ussd_map_lock );
          __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
          ussd_map.erase(dialog->ussdSequence);
        }
        DropMapDialog(dialog.get());
      }else if(dialog->state == MAPST_WaitSubmitUSSDNotifyConf) {
        if ( cmd->get_resp()->get_status() != 0 )
        {
          {
            MutexGuard ussd_map_guard( ussd_map_lock );
            __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
            ussd_map.erase(dialog->ussdSequence);
          }
          dialog->dropChain = true;
          CloseMapDialog(dialog->dialogid_map,dialog->ssn);
          DropMapDialog(dialog.get());
        } else {
          dialog->state = MAPST_ReadyNextUSSDCmd;
          if( dialog->chain.size() > 0 ) {
            SmscCommand cmd_c = dialog->chain.front();
            dialog->chain.pop_front();
            __map_trace2__("%s found chained USSD deliver for that dialog", __func__);
            MAPIO_PutCommand(cmd_c, 0);
          }
        }
      }else if(dialog->state == MAPST_WaitSubmitUSSDNotifyCloseConf) {
        {
          MutexGuard ussd_map_guard( ussd_map_lock );
          __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
          ussd_map.erase(dialog->ussdSequence);
        }
        DropMapDialog(dialog.get());
      }else
        throw MAPDIALOG_BAD_STATE(
          FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,dialog_ssn);
}

static inline
void DoProvErrorProcessing(ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( provErrCode_p != 0 ){
    if( Status::isErrorPermanent( Status::MAP_PROVIDER_ERR_BASE+*provErrCode_p ) ) {
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::%s fatal *provErrCode_p: 0x%x",__func__,*provErrCode_p),
        Status::MAP_PROVIDER_ERR_BASE+*provErrCode_p);
    } else {
      throw MAPDIALOG_TEMP_ERROR(
        FormatText("MAP::%s temp *provErrCode_p: 0x%x",__func__,*provErrCode_p),
        Status::MAP_PROVIDER_ERR_BASE+*provErrCode_p);
    }
  }
}

static inline
void DoMAPErrorProcessor(
  UCHAR_T errorCode,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( errorCode != 0 ){
    if( Status::isErrorPermanent( MAP_ERRORS_BASE+errorCode ) ) {
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::%s perm error errorCode: 0x%x",
                   __func__,
                   errorCode),
                   MAP_ERRORS_BASE+errorCode);
    } else {
      throw MAPDIALOG_TEMP_ERROR(
        FormatText("MAP::%s temp error errorCode: 0x%x",
                 __func__,
                 errorCode),
                 MAP_ERRORS_BASE+errorCode);
    }
  }
  DoProvErrorProcessing(provErrCode_p);
}


extern "C"
USHORT_T Et96MapGetACVersionConf(ET96MAP_LOCAL_SSN_T localSsn,UCHAR_T version,ET96MAP_SS7_ADDR_T *ss7Address_sp,ET96MAP_APP_CONTEXT_T ac)
{
  unsigned dialogid_map = 0;
  unsigned dialogid_smsc = 0;
  unsigned dialog_ssn = 0;
  MAP_TRY{
    MAPIO_TaskACVersionNotifier();
    if ( version == 3 ) version = 2;
    char text[128];
    SS7ToText(ss7Address_sp,text);
    string s_key(text);
    __map_trace2__("%s: is exists %s? version %d",__func__,s_key.c_str(),version);
    typedef multimap<string,unsigned>::iterator I;
    MutexGuard guard(x_map_lock);
    pair<I,I> range = x_map.equal_range(s_key);
    if ( range.first == range.second ) return ET96MAP_E_OK;
      //throw runtime_error("MAP::Et96MapGetACVersionConf has no address for AC resolving");
    I it;
    for ( it = range.first; it != range.second;++it) {
      unsigned dialogid_map = 0;
      unsigned dialogid_smsc = 0;
      unsigned dialog_ssn = 0;
      MAP_TRY{
        dialogid_map = it->second&0xffff;
        dialog_ssn = it->second>>24;
        DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,dialog_ssn));
        if ( dialog.isnull() ) {
          throw MAPDIALOG_HEREISNO_ID(
            FormatText("MAP::dialog 0x%x is not present",dialogid_map));
        }
        else
        {
          __require__(dialog->ssn == dialog_ssn);
          __map_trace2__("%s: dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
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
              FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
          }
        }
      }MAP_CATCH(dialogid_map,dialogid_smsc,dialog_ssn);
    }
    x_map.erase(s_key);
  }MAP_CATCH(dialogid_map,dialogid_smsc,dialog_ssn);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_HEREISNO_ID( FormatText("MAP::dialog 0x%x is not present",dialogid_map));
    }
    __require__(dialog->ssn == localSsn);
    __map_trace2__("%s: dialogid 0x%x (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    /* ��������� openResult */
    switch( dialog->state ){
    case MAPST_WaitDelRepConf:
      if ( openResult == ET96MAP_RESULT_NOT_OK ){
        dialog->state = MAPST_MTFINAL;
        __map_trace2__("%s: error refuse reason 0x%x",__func__,refuseReason_p?*refuseReason_p:0);
        DropMapDialog(dialog.get());
      }
      // nothing;
      break;
    case MAPST_WaitFwdMOOpenConf:
      if( openResult == ET96MAP_RESULT_NOT_OK) {
        DoProvErrorProcessing(provErrCode_p);
        throw MAPDIALOG_FATAL_ERROR(
          FormatText("MAP::%s connection opening error, reason %d",__func__,refuseReason_p?*refuseReason_p:0),
          refuseReason_p?(Status::MAP_REFUSE_REASON_BASE+*refuseReason_p):Status::MAP_REFUSE_REASON_NO_REASON);
      }
      dialog->state = MAPST_WaitFwdMOConf;
      break;
    case MAPST_WaitUSSDReqOpenConf:
    case MAPST_WaitUSSDNotifyOpenConf:
      if( openResult == ET96MAP_RESULT_NOT_OK) {
        DoProvErrorProcessing(provErrCode_p);
        throw MAPDIALOG_FATAL_ERROR(
          FormatText("MAP::%s connection opening error, reason %d",__func__,refuseReason_p?*refuseReason_p:0),
          refuseReason_p?(Status::MAP_REFUSE_REASON_BASE+*refuseReason_p):Status::MAP_REFUSE_REASON_NO_REASON);
      }
      break;
    case MAPST_RInfoFallBack:
    case MAPST_WaitSpecOpenConf:
    case MAPST_WaitOpenConf:
    case MAPST_ImsiWaitOpenConf:
      if ( openResult == ET96MAP_RESULT_NOT_OK ){
        if ( refuseReason_p && *refuseReason_p == ET96MAP_APP_CONTEXT_NOT_SUPP ){
          __map_trace2__("%s: context unsupported , version %d",__func__,dialog->version);
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
              FormatText("MAP::%s version alredy 1 and unsupported!",__func__),
              Status::MAP_REFUSE_REASON_BASE+*refuseReason_p);
          }
        }else{
          DoProvErrorProcessing(provErrCode_p);
          throw MAPDIALOG_FATAL_ERROR(
            FormatText("MAP::%s connection opening error, reason %d",__func__,refuseReason_p?*refuseReason_p:0),
            refuseReason_p?(Status::MAP_REFUSE_REASON_BASE+*refuseReason_p):Status::MAP_REFUSE_REASON_NO_REASON);
        }
      }
      /* ������� � ���� ��������� */
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
      case MAPST_WaitUSSDReqOpenConf:
        dialog->state = MAPST_WaitUSSDReqConf;
        break;
      case MAPST_WaitUSSDNotifyOpenConf:
        dialog->state = MAPST_WaitUSSDNotifyConf;
        break;
      }
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d",__func__,dialog->state));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
  return ET96MAP_E_OK;
}

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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn == localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x (state %d) %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());

    dialog->routeErr = 0;

    if ( errorSendRoutingInfoForSm_sp ) {
         
      if(errorSendRoutingInfoForSm_sp->errorCode == 27 )
      {
        __map_trace2__("%s: absent subscriber",__func__);
        dialog->subscriberAbsent = true;
        if( version == 1 && mwdSet && *mwdSet == ET96MAP_ADDRESS_ALREADY_IN_FILE ) {
           dialog->mwdStatus.mnrf = 1;
        }
        dialog->routeErr = MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_ERRORS_BASE+27);
      } else {
        if(errorSendRoutingInfoForSm_sp->errorCode == 13 && (imsi_sp != 0 && mscNumber_sp != 0)) {
          // normal situation no error
        } else {
          try {
            DoMAPErrorProcessor(errorSendRoutingInfoForSm_sp->errorCode,0);
          }catch(MAPDIALOG_ERROR& e){
            __map_trace2__("%s: was error %s",__func__,e.what());
            dialog->routeErr = e.code;
          }
        }
      }
    } else if( provErrCode_p ) {
      try {
        DoMAPErrorProcessor(0,provErrCode_p);
      }catch(MAPDIALOG_ERROR& e){
        __map_trace2__("%s: was error %s",__func__,e.what());
        dialog->routeErr = e.code;
      }
    }
    switch( dialog->state ){
    case MAPST_WaitRInfoConf:
      if ( dialog->routeErr ) {
        dialog->state = MAPST_WaitRInfoClose;
        break;
      }
    case MAPST_ImsiWaitRInfo:
      {
        if ( dialog->routeErr ) {
          dialog->s_imsi = "";
          dialog->s_msc = "";
          dialog->state = MAPST_ImsiWaitCloseInd;
          break;
        }
        else {
          // extract msc number
          if( version == 1 ) {
            dialog->s_msc = LocationInfoToString(locationInfo_sp);
            __map_trace2__( "LocationInfo addr type: %s address: %s", locationInfo_sp->typeOfNumber==0x01?"roaming":"msc", dialog->s_msc.c_str() );
            mkSS7GTAddress( &dialog->destMscAddr, locationInfo_sp, 8 );
          } else {
            dialog->s_msc = MscToString(mscNumber_sp);
            mkSS7GTAddress( &dialog->destMscAddr, mscNumber_sp, 8 );
          }
          dialog->s_imsi = ImsiToString(imsi_sp);
          dialog->smRpDa.typeOfAddress = ET96MAP_ADDRTYPE_IMSI;
          dialog->smRpDa.addrLen = imsi_sp->imsiLen;
          memcpy( dialog->smRpDa.addr, imsi_sp->imsi, imsi_sp->imsiLen );
  //#if !defined DISABLE_TRACING
          if( smsc::logger::_map_cat->isDebugEnabled() ) {
            {
              auto_ptr<char> b(new char[imsi_sp->imsiLen*4+1]);
              memset(b.get(),0,imsi_sp->imsiLen*4+1);
              for ( int i=0,k=0; i < imsi_sp->imsiLen; ++i ) {
                k += sprintf(b.get()+k,"%02x ",imsi_sp->imsi[i]);
              }
              __map_trace2__("IMSI: %s",b.get());
            }
            {
              if( mscNumber_sp ) {
                auto_ptr<char> b(new char[mscNumber_sp->addressLength*4+1]);
                memset(b.get(),0,mscNumber_sp->addressLength*4+1);
                for ( int i=0,k=0; i < (mscNumber_sp->addressLength+1)/2; ++i ) {
                  k += sprintf(b.get()+k,"%02x ",mscNumber_sp->address[i]);
                }
                __map_trace2__("MSCNUMBER: %s",b.get());
              }
              if( locationInfo_sp ) {
                auto_ptr<char> b(new char[locationInfo_sp->addressLength*4+1]);
                memset(b.get(),0,locationInfo_sp->addressLength*4+1);
                for ( int i=0,k=0; i < (locationInfo_sp->addressLength+1)/2; ++i ) {
                  k += sprintf(b.get()+k,"%02x ",locationInfo_sp->address[i]);
                }
                __map_trace2__("LOCINFO: %s",b.get());
              }
            }
          }
  //#endif
          if ( dialog->state == MAPST_WaitRInfoConf )
            dialog->state = MAPST_WaitRInfoClose;
          else if ( dialog->state == MAPST_ImsiWaitRInfo )
            dialog->state = MAPST_ImsiWaitCloseInd;
          else
            throw MAPDIALOG_BAD_STATE(
              FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
        }
        break;
      }
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",_di));
    }
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    dialog->id_opened = false;
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
          if( dialog->sms.get()->hasIntProperty( Tag::SMPP_USSD_SERVICE_OP ) ) {
            MapDialogContainer::getInstance()->reAssignDialog(dialogueId,dialog->ssn,USSD_SSN);
            int serviceOp = dialog->sms.get()->getIntProperty( Tag::SMPP_USSD_SERVICE_OP );
            if( serviceOp == USSD_USSR_REQ || serviceOp == USSD_USSN_REQ ) {
              DoUSSDRequestOrNotifyReq(dialog.get());
            } else {
              throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::USSD_SERVICE_OP=%d not allowed to create dialog",serviceOp),Status::INVOPTPARAMVAL);
            }
          } else {
            MapDialogContainer::getInstance()->reAssignDialog(dialogueId,dialog->ssn,localSsn);
            dialog->id_opened = false;
            dialogueId = dialog->dialogid_map;
            dialogid_map = dialog->dialogid_map;
            dialog->state = MAPST_WaitMcsVersion;
            QueryMcsVersion(dialog.get());
          }
        }else{
          __map_trace__("RIfno returned no route, close dialog");
          dialog->state = MAPST_CLOSED;
          dialogid_smsc = dialog->dialogid_smsc;
          //throw MAPDIALOG_ERROR(dialog->routeErr,"lisy error");
          SendErrToSmsc(dialog->dialogid_smsc,dialog->routeErr);
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
    case MAPST_WaitAlertDelimiter:
    case MAPST_WaitUSSDReqClose:
    case MAPST_WaitUSSDNotifyCloseErr:
      dialog->state = MAPST_CLOSED;
      DropMapDialog(dialog.get());
      break;
    case MAPST_WaitUSSDNotifyClose:
      dialog->state = MAPST_WaitSubmitUSSDNotifyCloseConf;
      SendSubmitCommand(dialog.get());
      break;
    case MAPST_WaitUSSDReqDelim:
      dialog->state = MAPST_WaitSubmitUSSDRequestCloseConf;
      SendSubmitCommand(dialog.get());
      break;
    case MAPST_ImsiWaitCloseInd:
      dialog->associate->hlrVersion = dialog->hlrVersion;
      ContinueImsiReq(dialog->associate,dialog->s_imsi,dialog->s_msc);
      dialog->state = MAPST_END;
      DropMapDialog(dialog.get());
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn == localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    dialog->id_opened = false;
    __map_trace2__("%s: dialogid 0x%x userReason 0x%x",__func__,dialogid_map,userReason_p?*userReason_p:-1);
    //throw runtime_error("UABORT");
    throw MAPDIALOG_TEMP_ERROR("UABORT",Status::MAP_USER_REASON_BASE+(userReason_p?*userReason_p:-1));
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
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
  __map_trace2__("%s: dialogid 0x%x source=%d provReason=0x%x",__func__,dialogueId,source,provReason);
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() ) {
      //throw runtime_error(
      //  FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
      __map_trace2__("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId);
    }else{
      __require__(dialog->ssn==localSsn);
      dialogid_smsc = dialog->dialogid_smsc;
      dialog->id_opened = false;
      if( dialog->isUSSD ) {
        MutexGuard ussd_map_guard( ussd_map_lock );
        __map_trace2__("erase ussd lock for %lld", dialog->ussdSequence);
        ussd_map.erase(dialog->ussdSequence);
      }
      throw MAPDIALOG_TEMP_ERROR("PABORT",Status::MAP_PROVIDER_REASON_BASE+provReason);
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapOpenInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_SS7_ADDR_T *ss7DestAddr_sp,
  ET96MAP_SS7_ADDR_T *ss7OrigAddr_sp,
#ifdef MAP_R12
  ET96MAP_IMSI_OR_MSISDN_T *destRef_sp,
#else
  ET96MAP_IMSI_T *destRef_sp,
#endif
  ET96MAP_ADDRESS_T *origRef_sp,
  ET96MAP_USERDATA_T *specificInfo_sp)
{
  try{
    __map_trace2__("%s: dialog 0x%x ctx=%d ver=%d dstref=%p orgref=%p",__func__,dialogueId,appContext_sp->acType,appContext_sp->version,destRef_sp,origRef_sp );
    DialogRefGuard dialog(MapDialogContainer::getInstance()->createDialog(dialogueId,localSsn,appContext_sp->version));
    __require__(dialog->ssn==localSsn);
    dialog->hasIndAddress = false;
    if ( specificInfo_sp!=0 && specificInfo_sp->specificInfoLen >= 3 )
    {
      if ( specificInfo_sp->specificData[0] == 0x82 )
      {
        unsigned x = specificInfo_sp->specificData[1];
        if ( (((unsigned)specificInfo_sp->specificData[x+1] >> 4)&0x0f == 0xf ) )
          x = (x-1)*2-1;
        else
          x = (x-1)*2;
        memcpy(&dialog->m_msAddr,specificInfo_sp->specificData+1,specificInfo_sp->specificInfoLen-1);
        dialog->m_msAddr.addressLength = x;
        dialog->hasIndAddress = true;
      }
    }
    if ( dialog.isnull() )
      throw runtime_error("MAP:: can't create dialog");
    dialog->state = MAPST_WaitSms;
  }
  catch(exception& e)
  {
    __map_warn2__("%s: dialogid 0x%x <exception>:%s",__func__,dialogueId,e.what());
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    MapDialogContainer::getInstance()->dropDialog(dialogueId,localSsn);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,dialogueId));
    }
    __require__(dialog->ssn==localSsn);
    __map_trace2__("%s: dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
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
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
    dialog->invokeId = invokeId;
    AttachSmsToDialog(dialog.get(),smRpUi_sp,smRpOa_sp);
    __map_trace2__("%s: %s",__func__,RouteToString(dialog.get()).c_str());
  }
  catch(exception& e)
  {
    __map_warn2__("%s: dialogid 0x%x <exception>:%s",__func__,dialogueId,e.what());
    if ( !open_confirmed ){
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      //MapDialogContainer::getInstance()->dropDialog(dialogueId);
      DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
      	__require__(dialog->ssn==localSsn);
      }
      Et96MapOpenResp(localSsn,dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0);
//      Et96MapDelimiterReq(localSsn,dialogueId,0,0);
    } 
    ET96MAP_ERROR_FORW_SM_MO_T moResp;
    moResp.errorCode = ET96MAP_UE_SYS_FAILURE;
    moResp.u.systemFailureNetworkResource_s.networkResourcePresent = 0;
    if( version == 3 ) {
	ET96MAP_SM_RP_UI_T ui;
        ui.signalInfoLen=0;
      
        Et96MapV3ForwardSmMOResp(
        localSsn,
        dialogueId,
        invokeId,
        &ui,
        &moResp);
    } else if( version == 2 ) {
        Et96MapV2ForwardSmMOResp(
        localSsn,
        dialogueId,
        invokeId,
        &moResp);
    }else if ( version == 1 ) {
        Et96MapV1ForwardSmMOResp(
        localSsn,
        dialogueId,
        invokeId,
        &moResp);
    }
//    TryDestroyDialog(dialogueId,localSsn);
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
USHORT_T Et96MapV2ForwardSmMOConf(ET96MAP_LOCAL_SSN_T localSsn, 
                                ET96MAP_DIALOGUE_ID_T dialogueId,
                                ET96MAP_INVOKE_ID_T invokeId,
                                ET96MAP_ERROR_FORW_SM_MO_T* errorForwardSMmo_sp, 
                                ET96MAP_PROV_ERR_T* provErrCode_p) {
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x  (state %d) forward %s",__func__,dialog->dialogid_map,dialog->state, RouteToString(dialog.get()).c_str());

    try {
      DoMAPErrorProcessor( errorForwardSMmo_sp?errorForwardSMmo_sp->errorCode:0, provErrCode_p );
    }catch(MAPDIALOG_ERROR& e){
      SendErrToSmsc(dialog->dialogid_smsc,e.code);
      dialog->dialogid_smsc = 0; // ����� �������� ������ �� ����� � �������
      dialog->wasDelivered = false;
    }

    switch( dialog->state ){
    case MAPST_WaitFwdMOConf:
      if( dialog->dialogid_smsc != 0 ) {
        dialog->wasDelivered = true;
      }
      dialog->state = MAPST_WaitSmsClose;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
  return ET96MAP_E_OK;
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

extern "C"
USHORT_T Et96MapV3ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp,
  ET96MAP_IMSI_T     *imsi_sp)
{
  return Et96MapVxForwardSmMOInd_Impl(localSsn,dialogueId,invokeId,smRpDa_sp,smRpOa_sp,smRpUi_sp,3);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("%s: dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x  (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state, RouteToString(dialog.get()).c_str());
    try {

      if ( provErrCode_p && *provErrCode_p == ET96MAP_NO_RESPONSE_FROM_PEER ) {
        MscManager::getMscStatus().report(dialog->s_msc.c_str(),false);
      }
      if ( errorForwardSMmt_sp )
      {
        if ( errorForwardSMmt_sp->errorCode == 27 )
        {
          dialog->subscriberAbsent = true;
        }
        if ( errorForwardSMmt_sp->errorCode == 32 ) /*delivery error*/ {
          if ( errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T::ET96MAP_MEM_CAPACITY_EXCEEDED )
          {
            dialog->memoryExceeded = true;
          }
          else if ( errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T::ET96MAP_PROTOCOL_ERROR
            || errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T::ET96MAP_MO_SERVICE_CENTER_CONGESTION )
          {
            __map_trace2__("%s: Delivery failure reason 0x%x",__func__,errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason);
          }
        }
      }
      DoMAPErrorProcessor( errorForwardSMmt_sp?errorForwardSMmt_sp->errorCode:0, provErrCode_p );
    }catch(MAPDIALOG_ERROR& e){
      __map_trace2__("%s: %s", __func__,e.what());
      SendErrToSmsc(dialog->dialogid_smsc,e.code);
      dialog->dialogid_smsc = 0; // ����� �������� ������ �� ����� � �������
      dialog->wasDelivered = false;
    }

    switch( dialog->state ){
    case MAPST_WaitSmsConf:
      if( dialog->dialogid_smsc != 0 ) {
        dialog->wasDelivered = true;
      }
      dialog->state = MAPST_WaitSmsClose;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,dialogueId));
    }
    __require__(dialog->ssn==localSsn);
    __map_trace2__("%s: dialogid 0x%x (state %d) %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->state = MAPST_WaitSmsMOInd;
      reason = ET96MAP_NO_REASON;
      result = Et96MapOpenResp(dialog->ssn,dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: dialog opened error 0x%x",result));
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
      dialog->state = MAPST_WaitImsiReq;
      PauseOnImsiReq(dialog.get());
      break;
    case MAPST_WaitUssdDelimiter:
      reason = ET96MAP_NO_REASON;
      result = Et96MapOpenResp(dialog->ssn,dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: dialog opened error 0x%x",result));
      dialog->state = MAPST_WaitUssdImsiReq;
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
    case MAPST_WaitUSSDReqDelim:
      dialog->state = MAPST_WaitSubmitUSSDRequestConf;
      SendSubmitCommand(dialog.get());
      break;
    case MAPST_WaitUSSDNotifyClose:
      dialog->state = MAPST_WaitSubmitUSSDNotifyConf;
      SendSubmitCommand(dialog.get());
      CloseMapDialog(dialog->dialogid_map,dialog->ssn);
      DropMapDialog(dialog.get());
      break;
    case MAPST_MapNoticed:
      result = Et96MapOpenResp(dialog->ssn,dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("Et96MapDelimiterInd: dialog openresp error 0x%x",result));
      result = Et96MapCloseReq (dialog->ssn,dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: dialog closereq error 0x%x",result));
      dialog->state = MAPST_END;
      DropMapDialog(dialog.get());
      break;
    case MAPST_ABORTED:
      result = Et96MapCloseReq(localSsn,dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapDelimiterInd: dialog closereq error 0x%x",result));
      dialog->state = MAPST_END;
      DropMapDialog(dialog.get());
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
  }
  catch(exception& e)
  {
    __map_trace2__("%s: dialogid 0x%x <exception>:%s",__func__,dialogueId,e.what());
    if ( !open_confirmed ){
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      //MapDialogContainer::getInstance()->dropDialog(dialogueId);
      DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
        __require__(dialog->ssn==localSsn);
        Et96MapOpenResp(localSsn,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
        Et96MapCloseReq(localSsn,dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0);
        DropMapDialog(dialog.get());
      }
    } else {
      TryDestroyDialog(dialogueId,localSsn);
    }
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
  __map_trace2__("%s: ismsi %s, msc: %s, state %d",__func__,s_imsi.c_str(),s_msc.c_str(), dialog->state);
  if ( dialog->state == MAPST_END ){// already closed
    return;
  }
  if ( s_imsi.length() == 0 || s_msc.length() == 0 )
  {
    if( dialog->state == MAPST_WaitUssdImsiReq) {
      DoUSSRUserResponceError(0, dialog); // send system failure
    } else {
      ResponseMO(dialog,9);
      dialog->state = MAPST_WaitSubmitCmdConf;
      CloseMapDialog(dialog->dialogid_map,dialog->ssn);
      DropMapDialog(dialog);
    }
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
  unsigned localSsn = 0;
  DialogRefGuard dialog(MapDialogContainer::getInstance()->createDialogImsiReq(SSN,map));
  if (dialog.isnull()) throw runtime_error(
    FormatText("MAP::%s can't create dialog",__func__));
  unsigned dialogid_map = dialog->dialogid_map;
  localSsn = dialog->ssn;
  MAP_TRY{
    if ( map->sms.get() == 0 )
      throw runtime_error(
        FormatText("MAP::%s has no SMS",__func__));
    if ( !map->isUSSD )
    {
      mkMapAddress( &dialog->m_msAddr, map->sms->getOriginatingAddress().value, map->sms->getOriginatingAddress().length );
    }
    else
    {
      if (!map->hasIndAddress )
        throw runtime_error("MAP::%s MAP.did:{0x%x} has no originating address");
      dialog->m_msAddr = map->m_msAddr;
/*      if( smsc::logger::_map_cat->isDebugEnabled() )
      {
        auto_ptr<char> b(new char[sizeof(ET96MAP_ADDRESS_T)*3+1]);
        memset(b.get(),0,sizeof(ET96MAP_ADDRESS_T)*3+1);
        for ( int i=0,k=0; i < sizeof(ET96MAP_ADDRESS_T); ++i ) {
          unsigned char * d = (unsigned char*)&map->m_msAddr;
          k += sprintf(b.get()+k,"%02x ",(unsigned int)d[i]);
        }
        __map_trace2__("%s ARRDDRESS: %s",__func__,b.get());
      }
*/
    }
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
//    __map_trace2__("MAP::%s: Query HLR AC version",__func__);
    //dialog->mshlrAddr = map->mshlrAddr;
    dialog->state = MAPST_ImsiWaitACVersion;
    QueryHlrVersion(dialog.get());
  }MAP_CATCH(dialogid_map,0,localSsn);
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

static string GetUSSDRequestString(
  const char* text,
  unsigned length)
{
  const char* p = text;
  const char* pEnd = text+length-1;
  for ( ; p < pEnd; --pEnd ) if ( (*pEnd == '#') || (*pEnd == '*') ) break;
  if( pEnd == p ) throw runtime_error("No trailing # or * found in USSD request string");
  for ( ; p < pEnd; ++p ) if ( (*p != '#') && (*p != '*') ) break;
  const char* sBegin = p;
  for ( ; p < pEnd; ++p ) if ( (*p == '#') || (*p == '*') ) break;
  if( p == pEnd ) return string();
  else {
    return string(p+1,pEnd);
  }
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    __dialogid_map = dialogueId;
    dialog->invokeId = invokeId;
    dialog->origInvokeId = invokeId;
    if( msisdn_sp != 0 ) {
      dialog->m_msAddr.addressLength = msisdn_sp->addressLength;
      dialog->m_msAddr.typeOfAddress = msisdn_sp->typeOfAddress;
      memcpy(dialog->m_msAddr.address, msisdn_sp->address,ET96MAP_ADDRESS_LEN);
      dialog->hasIndAddress = true;
      __map_trace2__("MAP::%s has msisdn %d.%d", __func__,dialog->m_msAddr.addressLength, dialog->m_msAddr.typeOfAddress );
    }
    string subsystem;
    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();

    UCHAR_T udhPresent, msgClassMean, msgClass;
    unsigned dataCoding = (unsigned)convertCBSDatacoding2SMSC(ussdDataCodingScheme, &udhPresent, &msgClassMean, &msgClass);
    if( dataCoding == smsc::smpp::DataCoding::SMSC7BIT )
    {
      MicroString ms;
      unsigned chars = ussdString_s.ussdStrLen*8/7;
      Convert7BitToSMSC7Bit(ussdString_s.ussdStr,chars/*ussdString_s.ussdStrLen*/,&ms,0);
      subsystem = GetUSSDSubsystem(ms.bytes,ms.len);
      string ussdStr = GetUSSDRequestString(ms.bytes, ms.len);
      sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,ussdStr.c_str(),ussdStr.length());
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,ussdStr.length());
      sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
    } else {
/*      sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,ussdString_sp->ussdStr,ussdString_sp->ussdStrLen);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,ussdString_sp->ussdStrLen);
      sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);*/
      __map_warn2__("DCS 0x%02X received in Et96MapV2ProcessUnstructuredSSRequestInd", ussdDataCodingScheme );
      throw runtime_error("Datacoding other then GSM7bit is not supported in Et96MapV2ProcessUnstructuredSSRequestInd");
    }
    __map_trace2__("%s: dialogid 0x%x invokeid=%d request encoding 0x%x length %d subsystem %s",__func__,dialogueId,invokeId,ussdDataCodingScheme,ussdString_s.ussdStrLen,subsystem.c_str());
    subsystem.insert(0, ".5.0.ussd:");
    Address dest_addr = Address(subsystem.c_str());
//    dest_addr.type = 0;
//    dest_addr.plan = 1;
    unsigned esm_class = 2; // Transaction mode
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(dest_addr);
    dialog->sms = _sms;
    dialog->state = MAPST_WaitUssdDelimiter;
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
  }MAP_CATCH(__dialogid_map,0,localSsn);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapV2UnstructuredSSRequestConf(
    ET96MAP_LOCAL_SSN_T localSsn,
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_USSD_DATA_CODING_SCHEME_T *ussdDataCodingScheme_p,
    ET96MAP_USSD_STRING_T *ussdString_sp,
    ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T *errorUnstructuredSSReq_sp,
    ET96MAP_PROV_ERR_T *provErrCode_p) 
{
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    try {
      DoMAPErrorProcessor(errorUnstructuredSSReq_sp?errorUnstructuredSSReq_sp->errorCode:0,provErrCode_p);
    } catch (MAPDIALOG_ERROR& er) {
      __map_trace2__( "%s: %s", __func__, er.what() );
      SendErrToSmsc(dialog->dialogid_smsc, er.code );
      dialog->state = MAPST_WaitUSSDReqClose;
      return ET96MAP_E_OK;
    }
    // ������� �� �� USSDRequestReq
    SendOkToSmsc(dialog.get());

    if( smsc::logger::_map_cat->isDebugEnabled() && ussdString_sp) {
     {
      char *text = new char[ussdString_sp->ussdStrLen*4+1];
      int k = 0;
      for ( int i=0; i<ussdString_sp->ussdStrLen; i++){
        k+=sprintf(text+k,"%02x ",(unsigned)ussdString_sp->ussdStr[i]);
      }
      text[k]=0;
      __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, 
        "%s: ussd string len=%d, %s",__func__, ussdString_sp->ussdStrLen, text);
      delete text;
     }
    }
    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    SMS* old_sms = dialog->sms.get();
    Address originator = old_sms->getOriginatingAddress();
    UCHAR_T udhPresent, msgClassMean, msgClass;
    if( ussdDataCodingScheme_p && ussdString_sp ) {
      unsigned dataCoding = (unsigned)convertCBSDatacoding2SMSC(*ussdDataCodingScheme_p, &udhPresent, &msgClassMean, &msgClass);
      if( dataCoding == smsc::smpp::DataCoding::SMSC7BIT )
      {
        MicroString ms;
        unsigned chars = ussdString_sp->ussdStrLen*8/7;
        Convert7BitToSMSC7Bit(ussdString_sp->ussdStr,chars,&ms,0);
        __map_trace2__("ussd str len=%d/%d: %s", ms.len, strlen(ms.bytes), ms.bytes );
        sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,ms.bytes,ms.len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
        sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
      } else {
        sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,(const char*)(ussdString_sp->ussdStr),ussdString_sp->ussdStrLen);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,ussdString_sp->ussdStrLen);
        sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
      }
      __map_trace2__("%s: dialogid 0x%x request dcs 0x%x encoding 0x%x length %d subsystem %s",__func__,dialogueId,*ussdDataCodingScheme_p,dataCoding,ussdString_sp->ussdStrLen,originator.toString().c_str());
    } else {
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,0);
      sms.setIntProperty(Tag::SMPP_DATA_CODING,smsc::smpp::DataCoding::SMSC7BIT);
    }
    unsigned esm_class = 2; // Transaction mode
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(originator);
    sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_USSR_CONF);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    dialog->sms = _sms;
    dialog->state = MAPST_WaitUSSDReqDelim;
    dialog->invokeId = invokeId;
  }MAP_CATCH(dialogueId,0,localSsn);
  return ET96MAP_E_OK;
}

//_CB(MapUssdNotifyConf)

USHORT_T Et96MapV2UnstructuredSSNotifyConf(
    ET96MAP_LOCAL_SSN_T localSsn,
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_ERROR_USSD_NOTIFY_T *errorUssdNotify_sp,
    ET96MAP_PROV_ERR_T *provErrCode_p)
{    
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    try {
      DoMAPErrorProcessor(errorUssdNotify_sp?errorUssdNotify_sp->errorCode:0,provErrCode_p);
    } catch (MAPDIALOG_ERROR& er) {
      __map_trace2__( "%s: %s", __func__, er.what() );
      SendErrToSmsc(dialog->dialogid_smsc, er.code );
      dialog->state = MAPST_WaitUSSDNotifyCloseErr;
      return ET96MAP_E_OK;
    }
    // ������� �� �� USSDNotifyReq
    SendOkToSmsc(dialog.get());
    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    SMS* old_sms = dialog->sms.get();
    Address originator = old_sms->getOriginatingAddress();
    __map_trace2__("%s: dialogid 0x%x notify confirmation subsystem %s",__func__,dialogueId, originator.toString().c_str());
    unsigned esm_class = 2; // Transaction mode
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(originator);
    sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_USSN_CONF);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    dialog->sms = _sms;
    dialog->state = MAPST_WaitUSSDNotifyClose;
    dialog->invokeId = invokeId;
  }MAP_CATCH(dialogueId,0,localSsn);
  return ET96MAP_E_OK;
}

static void SendAlertToSMSC(MapDialog* dialog,ET96MAP_ADDRESS_T *mapAddr)
{
  Address addr;
  ConvAddrMSISDN2Smc(mapAddr,&addr);
    if( smsc::logger::_map_cat->isDebugEnabled() ) {
     {
      char *text = new char[mapAddr->addressLength*4+1];
      int k = 0;
      for ( int i=0; i<mapAddr->addressLength; i++){
        k+=sprintf(text+k,"%02x ",(unsigned)mapAddr->address[i]);
      }
      text[k]=0;
      __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, 
        "%s: mapaddress len=%d, type=%x, val=%s",__func__, mapAddr->addressLength, mapAddr->typeOfAddress, text);
      delete text;
     }
    }

  mkMapAddress( &dialog->m_msAddr, addr.value, addr.length );
  __map_trace2__("%s: addr len=%d val=%s", __func__, addr.length, addr.value );
  mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), 11 );
  mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
  mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
  SmscCommand cmd = SmscCommand::makeHLRAlert(addr);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void ResponseAlertSC(MapDialog* dialog)
{
  USHORT_T result;
  ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
  result = Et96MapOpenResp(dialog->ssn,dialog->dialogid_map,ET96MAP_RESULT_OK,&reason,0,0,0);
  if ( result != ET96MAP_E_OK )
    throw runtime_error(
      FormatText("MAP::ResponseAlertSC:et96MapOpenResp: error 0x%hx",result));
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn==localSsn);
    dialog->hlrVersion = version;
    dialogid_smsc = dialog->dialogid_smsc;
    dialog->hlrWasNotified = TRUE;
    __map_trace2__("%s: dialogid 0x%x (state %d)",__func__,dialog->dialogid_map,dialog->state);
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->invokeId = invokeId;
      SendAlertToSMSC(dialog.get(),msisdnAlert_sp);
      dialog->state = MAPST_WaitAlertDelimiter;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
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
  if ( status != 0 && !status->scAddrNotIncl )
  {
    // remember status if current sc addr not included in HLR
    // else it will force to notify HLR if absent subscriber happens
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",_di));}
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    if( msisdnAlert_sp && smsc::logger::_map_cat->isDebugEnabled() )
    {
       int len = (msisdnAlert_sp->addressLength+1)/2+2;
       char *text = new char[len*4+1];
       unsigned char* buf = (unsigned char*)msisdnAlert_sp;
       int k = 0;
       for ( int i=0; i<len; i++) {
	   k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
       }
       text[k]=0;
       __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, "msisdnAlert_s: %s",text);
       delete text;
    }
    if( mwdStatus_sp && smsc::logger::_map_cat->isDebugEnabled() )
    {
       int len = sizeof(ET96MAP_MWD_STATUS_T);
       char *text = new char[len*4+1];
       unsigned char* buf = (unsigned char*)mwdStatus_sp;
       int k = 0;
       for ( int i=0; i<len; i++){
	   k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
       }
       text[k]=0;
       __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, "mwdStatus_s: %s",text);
       delete text;
    }
    switch( dialog->state ){
    case MAPST_WaitRInfoClose:
      RememberMwdStatus(dialog.get(),msisdnAlert_sp,mwdStatus_sp);
      break;
    case MAPST_ImsiWaitCloseInd:
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn);
  return ET96MAP_E_OK;
}

static bool NeedNotifyHLR(MapDialog* dialog)
{
  return
    !dialog->isUSSD &&
    !dialog->hlrWasNotified && dialog->hlrVersion != 0 &&
    ((!dialog->wasDelivered &&
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
  MapDialogContainer::getInstance()->reAssignDialog(dialog->dialogid_map,dialog->ssn,SSN);
  dialog->id_opened = false;
  dialog->hlrWasNotified = true;
  USHORT_T result;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
  SetVersion(appContext,dialog->hlrVersion);
  unsigned dialog_id = dialog->dialogid_map;
  __map_trace2__("%s: dialogid 0x%x ssn %d",__func__,dialog_id,dialog->ssn);

  ET96MAP_DEL_OUTCOME_T deliveryOutcom;
  if ( dialog->wasDelivered ) {
    deliveryOutcom = ET96MAP_TRANSFER_SUCCESSFUL;
    __map_trace2__("%s: TRANSFER_SUCCESSFUL",__func__);
  }
  else if ( dialog->subscriberAbsent ) {
    deliveryOutcom = ET96MAP_SUBSCRIBER_ABSENT;
    __map_trace2__("%s: SUBSCRIBER_ABSENT (flag:%d)",__func__,dialog->mwdStatus.mnrf);
  }
  else if ( dialog->memoryExceeded ) {
    deliveryOutcom = ET96MAP_MEMORY_CAPACITY_OVERRUN;
    __map_trace2__("%s: MEMORY_CAPACITY_OVERRUN (flag:%d)",__func__,dialog->mwdStatus.mcef);
  }
  else {
    __map_trace2__("%s: no way! isUSSD=%s hlrWasNotified=%s hlrVersion=%d wasDelivered=%s routeErr=%x subscriberAbsent=%s mwdStatus.mnrf=%x memoryExceeded=%s mwdStatus.mcef=%x",__func__,
  	dialog->isUSSD?"Y":"N",
  	dialog->hlrWasNotified?"Y":"N", 
  	dialog->hlrVersion,
        dialog->wasDelivered?"Y":"N",
        dialog->routeErr,
        dialog->subscriberAbsent?"Y":"N",
        dialog->mwdStatus.mnrf,
        dialog->memoryExceeded?"Y":"N",
        dialog->mwdStatus.mcef
        );

    //return; // Opps, strange way
    throw MAPDIALOG_ERROR(0,"no way");
  }

  result = Et96MapOpenReq(
    SSN, dialog_id,
    &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::%s: Et96MapOpenReq error 0x%x",__func__,result));
  }
  dialog->id_opened = true;

  // �� !!!!
  if ( dialog->hlrVersion != 2 && dialog->hlrVersion != 1 ) dialog->hlrVersion = 2;
  // !!!!

  if ( dialog->hlrVersion == 2) {
    result = Et96MapV2ReportSmDelStatReq(
      SSN,dialog->dialogid_map,1,&dialog->m_msAddr,&dialog->m_scAddr,deliveryOutcom);
  }else if ( dialog->hlrVersion == 1 ) {
    // ????? ������ �� ???????
    result = Et96MapV1ReportSmDelStatReq(
      SSN,dialog->dialogid_map,1,&dialog->m_msAddr,&dialog->m_scAddr);
  } else
    throw runtime_error(
    FormatText("MAP::%s bad protocol version 0x%x",__func__,dialog->hlrVersion));
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::%s: Et96MapV2ReportSmDelStatReq error 0x%x",__func__,result));
  }
  result = Et96MapDelimiterReq(SSN,dialog->dialogid_map,0,0);
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::%s: Et96MapDelimiterReq error 0x%x",__func__,result));
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

extern "C"
USHORT_T Et96MapNoticeInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_DIALOGUE_ID_T relDialogueId,
  ET96MAP_DIAGNOSTIC_PROBLEM_T diagProblem,
  ET96MAP_RETURNED_OPERATION_T *retOperation_sp) 
{
  __map_trace2__("%s: dialogid 0x%x relDialogid 0x%x diagProblem %d retop 0x%x", __func__,
                dialogueId,relDialogueId,diagProblem,retOperation_sp );
  DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn));
  if ( !dialog.isnull() ) {
    if( dialog->state == MAPST_WaitSms ) {
      dialog->state = MAPST_MapNoticed;
    } else {
      __map_warn2__("%s: dialogid 0x%x unknown state %d", __func__, dialogueId, dialog->state);
    }
  } else {
    USHORT_T res = Et96MapCloseReq (localSsn,dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0);
  }
  return ET96MAP_E_OK;
}
#else

#include "MapDialog_spcific.cxx"
void MAPIO_PutCommand(const SmscCommand& cmd )
{
  if ( cmd->get_commandId() == QUERYABONENTSTATUS )
  {
    SmscCommand xcmd = SmscCommand::makeQueryAbonentStatusResp(cmd->get_abonentStatus(),AbonentStatus::ONLINE,"");
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(xcmd);
  }
}

#endif
