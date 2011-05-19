#ident "@(#)$Id$"

#if defined USE_MAP
#define MAP_DIALOGS_LIMIT 360

#include <stdlib.h>
#include <stdarg.h>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include "FraudControl.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "MapLimits.hpp"
#include "system/smsc.hpp"

using namespace std;

#include "MapDialog_spcific.cxx"
#include "MapDialogMkPDU.cxx"
#include "system/status.h"
#include "util/smstext.h"

#include "mscman/MscManager.h"
#include "mscman/MscStatus.h"
#include "resourcemanager/ResourceManager.hpp"
#include "util/templates/DummyAdapter.h"
#include "util/recoder/recode_dll.h"
#include "NetworkProfiles.hpp"

using namespace smsc::mscman;

using namespace smsc::system;
using namespace smsc::system::mapio;
using namespace smsc::util;
using namespace smsc::resourcemanager;
using smsc::util::templates::DummyGetAdapter;


static inline unsigned GetMOLockTimeout() {return MapDialogContainer::getMOLockTimeout();}
static inline unsigned GetBusyDelay() { return MapDialogContainer::getBusyMTDelay(); }
static inline unsigned GetLockedByMODelay() { return MapDialogContainer::getLockedByMoDelay(); }
static void SendRInfo(MapDialog* dialog);
static bool SendSms(MapDialog* dialog);


struct XMOMAPLocker {
  ET96MAP_ADDRESS_T m_msAddr;
  unsigned long startTime;
  unsigned parts;
  unsigned ref;
};
struct ET96MAP_ADDRESS_LESS:public binary_function<bool,ET96MAP_ADDRESS_T,ET96MAP_ADDRESS_T> {
  bool operator()(const ET96MAP_ADDRESS_T& a, const ET96MAP_ADDRESS_T& b) const {
    return a.addressLength<b.addressLength ||
      (a.addressLength == b.addressLength && memcmp(a.address,b.address,(a.addressLength+1)/2)<0);
  }
};

typedef std::map<ET96MAP_ADDRESS_T,XMOMAPLocker,ET96MAP_ADDRESS_LESS> XMOMAP;

//static Mutex x_map_lock;
//typedef multimap<string,unsigned> X_MAP;
//static X_MAP x_map;

static EventMonitor ussd_map_lock;
typedef std::map<long long,unsigned> USSD_MAP;
static USSD_MAP ussd_map;

struct UssdProcessingGuard{
  MapDialog* dlg;
  UssdProcessingGuard():dlg(0)
  {
  }
  ~UssdProcessingGuard()
  {
    if(dlg)
    {
      MutexGuard mg(ussd_map_lock);
      dlg->ussdProcessing=false;
      ussd_map_lock.notify(&dlg->condVar);
    }
  }
  void lockProcessing(MapDialog* argDlg)
  {
    dlg=argDlg;
    while(dlg->ussdProcessing)
    {
      ussd_map_lock.wait(&dlg->condVar);
    }
    dlg->ussdProcessing=true;
  }
};

static Mutex x_momap_lock;
static XMOMAP x_momap;

static void ContinueImsiReq(MapDialog* dialog,const String32& s_imsi,const String32& s_msc, const unsigned routeErr);
static void PauseOnAtiReq(MapDialog* map);
static void PauseOnImsiReq(MapDialog* map);
static void makeAtiRequest(MapDialog* map);
static const string& SC_ADDRESS() { return MapDialogContainer::GetSCAdress(); }
static const string& USSD_ADDRESS() { return MapDialogContainer::GetUSSDAdress(); }
static bool NeedNotifyHLR(MapDialog* dialog);
static void ResponseAlertSC(MapDialog* dialog);
static void SendStatusToSmsc(unsigned dialogid,unsigned code,bool isUssd,unsigned mr);
static void ForwardMO(MapDialog* dialog);

#define MAP_FALURE (/*MAP_ERRORS_BASE+34*/8)

/*struct MAPDIALOG_ERROR : public runtime_error
{
  unsigned code;
  MAPDIALOG_ERROR(unsigned code,const string& s) :
    runtime_error(s),code(code){}
  MAPDIALOG_ERROR(const string& s) :
    runtime_error(s),code(MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE)){}
};*/

struct MAPDIALOG_XERROR : public runtime_error
{
  unsigned code;
  MAPDIALOG_XERROR(unsigned code,const string& s) :
    runtime_error(s),code(code){}
  MAPDIALOG_XERROR(const string& s) :
    runtime_error(s),code(MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE)){}
};

struct MAPDIALOG_BAD_STATE : public MAPDIALOG_ERROR
{
  MAPDIALOG_BAD_STATE(const string& s) :
  MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE),s){}
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
  virtual ~MAPDIALOG_FATAL_ERROR()throw(){}
};
struct MAPDIALOG_HEREISNO_ID : public MAPDIALOG_ERROR
{
  MAPDIALOG_HEREISNO_ID(const string& s,unsigned c=MAP_FALURE) :
  MAPDIALOG_ERROR(0,s){}
  virtual ~MAPDIALOG_HEREISNO_ID()throw(){}
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

static string FormatText(const char* format,...)
{
  //auto_ptr<char> b(new char[1024]);
  //memset(b.get(),0,1024);
  char b[1024]={
    0,
  };
  va_list arg;
  va_start(arg,format);
  vsnprintf(b,1024-1,format,arg);
  va_end(arg);
  return string(b);
}

void checkMapReq(USHORT_T result, const char* func) {
  if ( result != ET96MAP_E_OK ) {
    const string &text = FormatText("MAP request call failed result=%d in %s", result, func);
    if ( result == 1009 || result == 1011 ) {
      throw MAPDIALOG_TEMP_ERROR(text, Status::SYSFAILURE);
    } else {
      throw runtime_error(text);
    }
  }
}

void warnMapReq(USHORT_T result, const char* func) {
  if ( result != ET96MAP_E_OK )
  {
    __map_warn2__("MAP request call failed result=%d in %s", result, func);
  }
}

void AbortMapDialog(unsigned dialogid,unsigned ssn, EINSS7INSTANCE_T rinst)
{
  if ( dialogid == 0 ) return;
  __map_trace2__("%s: dlg 0x%x",__func__,dialogid);
  warnMapReq( Et96MapUAbortReq(ssn INSTARG(rinst),dialogid,0,0,0,0), __func__);
}

String32 ImsiToString(const ET96MAP_IMSI_T* imsi)
{
  unsigned n = imsi->imsiLen;
  String32 rv;
  unsigned i = 0,idx=0;
  for ( ;i < n && idx<31; ++i ) {
    unsigned x0 = ((unsigned int)imsi->imsi[i])&0x0f;
    unsigned x1 = (((unsigned int)imsi->imsi[i])>>4)&0x0f;
    if ( x0 <= 9 )
    {
      rv[idx++]=x0+'0';
    }
    else break;
    if ( x1 <= 9 )
    {
      rv[idx++]=x1+'0';
    }
    else break;
  }
  rv[idx]=0;
  return rv;
}

String32 MscToString(const ET96MAP_ADDRESS_T* msc)
{
  unsigned bytes = (msc->addressLength+1)/2;
  String32 rv;
  int idx=0;
  for ( unsigned i=0; i<bytes && idx<31; ++i) {
    unsigned x0 = ((unsigned int)msc->address[i])&0x0f;
    unsigned x1 = (((unsigned int)msc->address[i])>>4)&0x0f;
    if ( x0 <= 9 )
    {
      rv[idx++]=x0+'0';
    }
    else break;
    if ( x1 <= 9 )
    {
      rv[idx++]=x1+'0';
    }
    else break;
  }
  rv[idx]=0;
  return rv;
}


bool FixedAddrCompare(const std::string& addr1,const std::string& addr2,int ignoreTail)
{
  if(addr1==addr2)return true;
  if(addr1.find(addr2)==0)return true;
  if(ignoreTail==0)return false;
  size_t l=addr1.length()-1;
  if(l>addr2.length()-1)l=addr2.length()-1;
  if(ignoreTail>l)return true;
  l-=ignoreTail;
  while(l)
  {
    if(addr1[l]!=addr2[l])return false;
    l--;
  }
  return true;
}

String32 SS7AddressToString(const ET96MAP_SS7_ADDR_T* addr)
{
  if(addr==0 || addr->ss7AddrLen==0)return "";

  uint8_t ind=addr->ss7Addr[0];
  bool spcInd=(ind&1)!=0;
  bool ssnInd=(ind&2)!=0;
  uint8_t gtInd=(ind>>2)&0x0f;
  uint8_t *ptr=(uint8_t*)(addr->ss7Addr);
  uint8_t *end=ptr+addr->ss7AddrLen;

  ptr++;//indicator

  if(spcInd)
  {
    // if 7th and 8th bits of second octet are zero,
    // 14 bit spc ind, 24 bit spc otherwise.
    if(ptr[1]&0xc0)
    {
      ptr++;
    }
    ptr+=2;
  }
  if(ssnInd)
  {
    ptr++;
  }

  bool isOdd=false;
  switch(gtInd)
  {
    case 1:
    {
      isOdd=((*ptr)&0x80)!=0; ptr++; // nature of address
    }break;
    case 2:
    {
      ptr++; // translation type
    }break;
    case 3:
    {
      ptr++;//trans type
      isOdd=((*ptr)&0x03)==1; ptr++; //(num plan, enc scheme)
    }break;
    case 4:
    {
      ptr++;//trans type
      isOdd=((*ptr)&0x03)==1; ptr++; //(num plan, enc scheme)
      ptr++;// nature of addr
    }break;
    default:return "";
  }

  String32 rv;
  int idx=0;
  for ( ; ptr!=end; ptr++)
  {
    unsigned x0 = *ptr&0x0f;
    unsigned x1 = (*ptr>>4)&0x0f;
    if ( x0 <= 9 )
    {
      rv[idx++]=x0+'0';
    }
    else
    {
      break;
    }
    if ( x1 <= 9 )
    {
      rv[idx++]=x1+'0';
    }
    else
    {
      break;
    }
  }
  if((idx&1)==0 && isOdd)
  {
    idx--;
  }
  rv[idx]=0;
  return rv;
}


smsc::core::buffers::FixedLengthString<32> LocationInfoToString(const ET96MAP_LOCATION_INFO_T* msc)
{
  unsigned bytes = (msc->addressLength+1)/2;
  smsc::core::buffers::FixedLengthString<32> rv;
  int idx=0;
  for ( unsigned i=0; i<bytes && idx<31; ++i) {
    unsigned x0 = ((unsigned int)msc->address[i])&0x0f;
    unsigned x1 = (((unsigned int)msc->address[i])>>4)&0x0f;
    if ( x0 <= 9 )
    {
      rv[idx++]=x0+'0';
    }
    else break;
    if ( x1 <= 9 )
    {
      rv[idx++]=x1+'0';
    }
    else break;
  }
  rv[idx]=0;
  return rv;
}

static unsigned MakeMrRef()
{
  return (unsigned)(time(0)%0x0ffffff);
}

static inline void eraseUssdLock(MapDialog *dialog, const char* function)
{
  if(dialog->ussdSequence==0)
  {
    return;
  }
  MutexGuard mg(ussd_map_lock);
  USSD_MAP::iterator it=ussd_map.find(dialog->ussdSequence);
  uint32_t val=dialog->dialogid_map|(dialog->ssn<<16)|(dialog->instanceId<<24);
  if(it==ussd_map.end())
  {
    __map_warn2__("%s: ussd lock not found for ussd dialog for %lld dlg 0x%x", function, dialog->ussdSequence, dialog->dialogid_map );
  }else if(it->second == val )
  {
    __map_trace2__("%s: erase ussd lock for %lld dlg 0x%x", function, dialog->ussdSequence, it->second);
    ussd_map.erase(it);
  } else
  {
    __map_warn2__("%s: could not erase ussd lock, dialog for %lld already reassigned to dlg 0x%x but requested dlg 0x%x", function, dialog->ussdSequence, it->second, dialog->dialogid_map );
  }
}

#ifdef EIN_HD
#define VERSION_NS
#else
#define VERSION_NS ET96MAP_APP_CNTX_T
#endif

static inline
void SetVersion(ET96MAP_APP_CNTX_T& ac,unsigned version)
{
  if ( version > 3 || version == 0 ) throw runtime_error(
    FormatText("%s: Opss, version = %d, why?",__func__,version));
  switch(version){
  case 3:
  //  ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_3;
  //  break;
  case 2:
    ac.version = VERSION_NS::ET96MAP_VERSION_2;
    break;
  case 1:
    ac.version = VERSION_NS::ET96MAP_VERSION_1;
    break;
  }
}

static void CloseMapDialog(unsigned dialogid,unsigned dialog_ssn,int rinst)
{
  if ( dialogid == 0 ) return;
  warnMapReq( Et96MapCloseReq (dialog_ssn INSTARG(rinst),dialogid,ET96MAP_NORMAL_RELEASE,0,0,0), __func__ );
}

static void TryDestroyDialog(unsigned,bool send_error,unsigned err_code,unsigned ssn,EINSS7INSTANCE_T rinst);

typedef smsc::core::buffers::FixedLengthString<64> String64;

static String64 RouteToString(MapDialog*);

static void QueryHlrVersion(MapDialog*);
static void QueryMcsVersion(MapDialog*);

static ET96MAP_SS7_ADDR_T* GetScAddr()
{
  static ET96MAP_ADDRESS_T m_scAddr;
  static ET96MAP_SS7_ADDR_T scAddr;
  static bool initialized = false;
  if ( !initialized )
  {
    mkMapAddress( &m_scAddr, SC_ADDRESS().c_str(), (unsigned)SC_ADDRESS().length() );
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
  if ( !initialized )
  {
    mkMapAddress( &m_ussdAddr, USSD_ADDRESS().c_str(), (unsigned)USSD_ADDRESS().length() );
    mkSS7GTAddress( &ussdAddr, &m_ussdAddr, USSD_SSN );
    initialized = true;
  }
  return &ussdAddr;
}
/*
extern void MAPIO_TaskACVersionNotifier();
void MAPIO_QueryMscVersionInternal()
{
  warnMapReq( Et96MapGetACVersionReq(SSN,GetScAddr(),ET96MAP_SHORT_MSG_MT_RELAY), __func__ );
}
*/

static void CheckLockedByMO(MapDialog* dialog)
{
  MutexGuard guard(x_momap_lock);
  XMOMAP::iterator it = x_momap.find(dialog->m_msAddr);
  if ( it != x_momap.end() )
  {
    if ( it->second.startTime+GetMOLockTimeout() <= time(0) )
    {
      __map_trace2__("%s: udhi time expired, unlocked, dlg 0x%x recv(%d)",__func__,dialog->dialogid_map, it->second.parts);
      x_momap.erase(it);
    }
    else
    {
      dialog->dropChain = true;
      __map_trace2__("%s: udhi locked, reschedule NOW! dlg 0x%x recv(%d)",__func__,dialog->dialogid_map,it->second.parts);
      throw MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,Status::LOCKEDBYMO),
                            "Locked by MO: reschedule NOW!");
    }
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
  if ( !dialog->isQueryAbonentStatus )
  {
    dialog->AssignSms(cmd->get_sms_and_forget());
    __map_trace2__("%s: DELIVERY_SM dlg 0x%x %s",__func__,dialog->dialogid_map,RouteToString(dialog).c_str());
    mkMapAddress( &dialog->m_msAddr, dialog->sms->getDestinationAddress() );
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), (unsigned)SC_ADDRESS().length() );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, SSN );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
    if( dialog->sms->hasStrProperty( Tag::SMSC_FORWARD_MO_TO ) )
    {
      ForwardMO( dialog );
      return;
    }
    CheckLockedByMO(dialog);
  }else
  {
    AbonentStatus& as = dialog->QueryAbonentCommand->get_abonentStatus();
    __map_trace2__("%s: Abonent Status cmd dlg 0x%x (%d.%d.%s)",__func__,dialog->dialogid_map,(unsigned)as.addr.type,(unsigned)as.addr.plan,as.addr.value);
    mkMapAddress( &dialog->m_msAddr, as.addr.value, as.addr.length );
    dialog->hasIndAddress = true;
    if(dialog->isAtiDialog)
    {
      mkMapAddress( &dialog->m_scAddr, USSD_ADDRESS().c_str(), (unsigned)USSD_ADDRESS().length() );
      mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, USSD_SSN );
      mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, HLR_SSN );
    }else
    {
      mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), (unsigned)SC_ADDRESS().length() );
      mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, SSN );
      mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, HLR_SSN );
    }
  }
  dialog->state = MAPST_WaitHlrVersion;
  if(dialog->isAtiDialog)
  {
    makeAtiRequest(dialog);
  }else
  {
    QueryHlrVersion(dialog);
  }
}

static void NotifyHLR(MapDialog* dialog);
static void MAPIO_PutCommand(const SmscCommand& cmd, MapDialog* dialog2=0 );

static void DropMapDialog(MapDialog* dlg)
{
//  if ( dialogid == 0 ) return;
  if ( !dlg )
  {
    __map_warn2__( "%s: dlg is null",__func__);
    return;
  }
  DialogRefGuard dialog(dlg->AddRef());
  //
  try{
    bool isChainEmpty;
    bool needNotifyHLR=false;
    {
      MutexGuard mg(dialog->mutex);
      isChainEmpty=dialog->chain.empty();
      if(isChainEmpty)
      {
        if(!(dialog->associate != 0 && dialog->state != MAPST_END) && NeedNotifyHLR(dialog.get()))
        {
          needNotifyHLR=true;
        }else
        {
          dialog->isDropping=true;
        }
      }
      if(!isChainEmpty && dialog->dropChain)
      {
        dialog->isDropping=true;
      }
    }

    __map_trace2__("%s: dropping dlg 0x%x, chain is empty %s , dropChain %d, associate:%p",__func__,dialog->dialogid_map,isChainEmpty?"yes":"no",dialog->dropChain, dialog->associate);
    if ( isChainEmpty )
    {
      if ( dialog->associate != 0 && dialog->state != MAPST_END )
      {
        dialog->associate->hlrVersion = dialog->hlrVersion;
        ContinueImsiReq(dialog->associate,"","",dialog->routeErr);
        dialog->state = MAPST_END;
      }
      else if ( needNotifyHLR )
      {
        try{
          NotifyHLR(dialog.get());
          return; // do not drop dialog!
        }catch(exception& e)
        {
          // drop dialog
          if ( dialog->id_opened )
          {
            AbortMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
            dialog->id_opened = false;
          }
          __map_warn2__("%s: <exception> %s",__func__,e.what());
        }
      }
      MapDialogContainer::getInstance()->dropDialog(dialog.get());
      return;
    }
  } catch (std::exception &e) {
    __map_warn2__("%s: exception %s",__func__,e.what());
  } catch (...) {
    __map_warn2__("%s: unknown exception",__func__);
  }
  __map_trace2__("%s: restart on next in chain",__func__);
  if ( dialog->dropChain )
  {
    {
      MutexGuard mg(dialog->mutex);
      for (;!dialog->chain.empty();dialog->chain.pop_front())
      {
        try{
          SmscCommand cmd = dialog->chain.front();
          SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,Status::SUBSCRBUSYMT),dialog->isUSSD,dialog->ussdMrRef);
        }catch(exception& e){
          __map_warn2__("%s: exception %s",__func__,e.what());
        }catch(...){
          __map_warn2__("%s: unknown exception",__func__);
        }
      }
    }
    MapDialogContainer::getInstance()->dropDialog(dialog.get());
  }else
  {
    // chain is not empty and !dropChain.
    // clean dialog and processing next command in chain
    SmscCommand cmd;
    {
      MutexGuard mg(dialog->mutex);
      if(!dialog->chain.empty())
      {
        cmd = dialog->chain.front();
        dialog->chain.pop_front();
        dialog->Clean();
      }
    }
    if(cmd.IsOk())
    {
      MAPIO_PutCommand(cmd, dialog.get());
    }
  }
}


static unsigned RemapDialog(MapDialog* dialog,unsigned ssn,ReAssignDialogType dlgType){
  dialog->id_opened = false;
  dialog->invokeId = 0;
  return MapDialogContainer::getInstance()->reAssignDialog(dialog->instanceId,dialog->dialogid_map,dialog->ssn,ssn,dlgType);
}

/*static void SendRescheduleToSmsc(unsigned dialogid)
{
  if ( dialogid == 0 ) return;
  __map_trace__("Send RESCHEDULE NOW to SMSC");
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,0));
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}*/

static void SendAbonentStatusToSmsc(MapDialog* dialog,int status/*=AbonentStatus::UNKNOWN*/)
{
  __map_trace2__("Send abonent status(%d) to SMSC dlg 0x%x",status,dialog->dialogid_map);
  //dialog->
  int code=dialog->routeErr;
  if(code==0 && dialog->callbarred)
  {
    code=Status::CALLBARRED;
  }
  if(code==0 && dialog->teleservicenotprov)
  {
    code=Status::TELSVCNOTPROVIS;
  }
  SmscCommand cmd = SmscCommand::makeQueryAbonentStatusResp(dialog->QueryAbonentCommand->get_abonentStatus(),status,code,dialog->s_msc,dialog->s_imsi);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void SendStatusToSmsc(unsigned dialogid,unsigned code,bool isUssd,unsigned mr)
{
  if ( dialogid == 0 ) return;
  __map_trace2__("Send status 0x%x to SMSC dialogid=%x",code,dialogid);
  if ( GET_STATUS_TYPE(code) == CMD_ERR_FATAL && GET_STATUS_CODE(code) == 0 )
  {
    code = MAKE_ERRORCODE(CMD_ERR_FATAL,Status::MAPINTERNALFAILURE);
  }
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,code);
  if ( GET_STATUS_CODE(code) == Status::SUBSCRBUSYMT &&
      GET_STATUS_TYPE(code) == CMD_ERR_RESCHEDULENOW )
  {
    cmd->get_resp()->set_delay(GetBusyDelay());
  } else if( GET_STATUS_CODE(code) == Status::LOCKEDBYMO )
  {
    cmd->get_resp()->set_delay(GetLockedByMODelay());
  }
  if(isUssd)
  {
    cmd->get_resp()->ussd_session_id=mr;
  }
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void SendErrToSmsc(MapDialog* dialog,unsigned code)
{
  unsigned dialogid=dialog->dialogid_smsc;
  if ( dialogid == 0 ) return;
  if( (code&0xFFFF) == 0 ) {
  __map_warn2__("Send error 0x%x with code 0 to SMSC dialogid=%x",code,dialogid);
  }
  __map_trace2__("Send error 0x%x to SMSC dialogid=%x",code,dialogid);
  if ( GET_STATUS_TYPE(code) == CMD_ERR_FATAL && GET_STATUS_CODE(code) == 0 ){
    code = MAKE_ERRORCODE(CMD_ERR_FATAL,Status::MAPINTERNALFAILURE);
  }
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,code);
  if ( GET_STATUS_CODE(code) == Status::SUBSCRBUSYMT
    && GET_STATUS_TYPE(code) == CMD_ERR_RESCHEDULENOW ){
    cmd->get_resp()->set_delay(GetBusyDelay());
  } else if( GET_STATUS_CODE(code) == Status::LOCKEDBYMO ) {
    cmd->get_resp()->set_delay(GetLockedByMODelay());
  }
  if(dialog->isUSSD)
  {
    cmd->get_resp()->ussd_session_id=dialog->ussdMrRef;
  }
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}


static void SendOkToSmsc(MapDialog* dialog)
{
  if ( dialog == 0 || dialog->dialogid_smsc == 0 ) {
    return;
  }
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialog->dialogid_smsc,0);
  Descriptor desc;
  desc.setImsi((uint8_t)dialog->s_imsi.length(),dialog->s_imsi.c_str());
  desc.setMsc((uint8_t)dialog->s_msc.length(),dialog->s_msc.c_str());
  cmd->get_resp()->setDescriptor(desc);
  if(dialog->isUSSD)
  {
    cmd->get_resp()->ussd_session_id=dialog->ussdMrRef;
  }
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __map_trace2__("Sent OK to SMSC: dlg 0x%x MSC(%d) = %s, IMSI(%d) = %s, %s",
                   dialog->dialogid_map,dialog->s_msc.length(),dialog->s_msc.c_str(),
                 dialog->s_imsi.length(),dialog->s_imsi.c_str(), RouteToString(dialog).c_str());
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
                  FormatText("%s: bad state %d, did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
  }
}

static void QueryMcsVersion(MapDialog* dialog)
{
  dialog->version = 2;
  SendSms(dialog);
}

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
    if( dialog->state == MAPST_ImsiWaitOpenConf ) {
      hiPrior = true;
    } else if( sms != 0 ) {
      if( sms->getIntProperty(Tag::SMPP_PRIORITY) == 3 ) {
        hiPrior = true;
      } else {
        if( sms->getLastResult() != Status::ABSENTSUBSCR &&
            sms->getLastResult() != Status::MSMEMCAPACITYFULL ) {
          hiPrior = true;
        } else if( (sms->getAttemptsCount())%4 == 0 ) {
          hiPrior = true;
        }
      }
    }
//    hiPrior = true;
  }
  __map_trace2__("%s: dlg 0x%x ssn:%d hiprior:%s ac:%d ver:%d",__func__,dialog_id,dialog->ssn,hiPrior?"true":"false", sms?(int)sms->getAttemptsCount():-1, dialog->version);
  checkMapReq( Et96MapOpenReq(
    dialog->ssn INSTDLGARG(dialog), dialog_id,
    &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 ), __func__);

  dialog->id_opened = true;

  if ( dialog->version != 2 && dialog->version != 1 ) dialog->version = 2;

  if ( dialog->version == 2 )
  {
    checkMapReq( Et96MapV2SendRInfoForSmReq(dialog->ssn INSTDLGARG(dialog), dialog_id, 0, &dialog->m_msAddr,
      hiPrior ? ET96MAP_ATTEMPT_DELIVERY : ET96MAP_DO_NOT_ATTEMPT_DELIVERY,
      &dialog->m_scAddr ), __func__);
  }else if ( dialog->version == 1 )
  {
    checkMapReq( Et96MapV1SendRInfoForSmReq(dialog->ssn INSTDLGARG(dialog), dialog_id, 0, &dialog->m_msAddr,
      hiPrior ? ET96MAP_ATTEMPT_DELIVERY : ET96MAP_DO_NOT_ATTEMPT_DELIVERY,
      &dialog->m_scAddr, 0, 0), __func__);
  }else
  {
    throw runtime_error(FormatText("SendRInfo: incorrect dialog version %d",dialog->version));
  }
  checkMapReq( Et96MapDelimiterReq(dialog->ssn INSTDLGARG(dialog), dialog_id, 0, 0 ), __func__);
}

#ifdef EIN_HD
#define FAILURE_MO_NS
#define FAILURE_MT_NS
#else
#define FAILURE_MO_NS ET96MAP_SM_DELIVERY_FAILURE_REASON_MO_T
#define FAILURE_MT_NS ET96MAP_SM_DELIVERY_FAILURE_REASON_MT_T
#endif

void ResponseMO(MapDialog* dialog,unsigned status)
{
  ET96MAP_ERROR_FORW_SM_MO_T err ;

  memset(&err,0,sizeof(ET96MAP_ERROR_FORW_SM_MO_T));
  dialog->wasDelivered = true;
  switch ( status )
  {
  case Status::OK: break;
  case Status::INVSCHED:
    err.errorCode = 32;
    err.u.smDeliveryFailureReason_s.reason = FAILURE_MO_NS::ET96MAP_PROTOCOL_ERR;
    break;
  default:
    if( Status::isErrorPermanent(status) ) {
      err.errorCode = 32;
      err.u.smDeliveryFailureReason_s.reason = FAILURE_MO_NS::ET96MAP_INVALID_SME_ADDRESS;
    } else {
      err.errorCode = 32;
      err.u.smDeliveryFailureReason_s.reason = FAILURE_MO_NS::ET96MAP_SERVICE_CENTER_CONGESTION;
    }
    break;
  };
  __map_trace2__("%s: dlg 0x%x errCode=0x%x status=%d (state %d) ",__func__,dialog->dialogid_map,err.errorCode,status,dialog->state);
  if ( dialog->version == 3 ) {
    ET96MAP_SM_RP_UI_T ui;
    ui.signalInfoLen=0;
    checkMapReq( Et96MapV3ForwardSmMOResp( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, &ui, (status!=Status::OK)?&err:0), __func__);
  } else if ( dialog->version == 2 ) {
    checkMapReq( Et96MapV2ForwardSmMOResp( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, (status!=Status::OK)?&err:0), __func__);
  }else if ( dialog->version == 1 ) {
    checkMapReq( Et96MapV1ForwardSmMOResp( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, (status!=Status::OK)?&err:0), __func__);
  }else throw runtime_error(
    FormatText("ResponseMO: incorrect dialog version %d",dialog->version));
  unsigned INVALIDUDHI = (unsigned)-1;
  if ( dialog->udhiRef != INVALIDUDHI )
  {
    MutexGuard guard(x_momap_lock);
    XMOMAPLocker* locker;
    __map_trace2__("%s: UDHI: find locker with ref %x",__func__,dialog->udhiRef);
    XMOMAP::iterator it = x_momap.find(dialog->m_msAddr);
    if ( it == x_momap.end() )
    {
      __map_trace2__("UDHI:%s: create locker",__func__);
      XMOMAPLocker lockerX;
      lockerX.m_msAddr = dialog->m_msAddr;
      lockerX.ref = INVALID;
      lockerX.parts = INVALID;
      locker = &x_momap[dialog->m_msAddr];
      *locker = lockerX;
      //locker = x_momap[dialog->s_imsi];
    }
    else locker = &it->second;
    if ( locker->ref == dialog->udhiRef )
    {
      __map_trace2__("%s: UDHI: update locker part %d/%d",__func__,dialog->udhiMsgNum,dialog->udhiMsgCount);
      ++locker->parts;
      if ( locker->parts >= dialog->udhiMsgCount ){
        x_momap.erase(dialog->m_msAddr);
        __map_trace2__("%s: UDHI: unlock part ",__func__);
      }else
        locker->startTime = time(0);
    }
    else
    {
      __map_trace2__("%s: UDHI: update locker part %d/%d",__func__,dialog->udhiMsgNum,dialog->udhiMsgCount);
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
    throw MAPDIALOG_FATAL_ERROR("ForwardMO: SMS has SMSC_FORWARD_MO_TO but has no SMSC_MO_PDU",smsc::system::Status::DATAMISSING);

  dialog->state = MAPST_WaitFwdMOOpenConf;
  dialog->version = 2;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_MO_RELAY;
  SetVersion(appContext,dialog->version);

  mkMapAddress( &fwdAddr, addr );
  mkSS7GTAddress( &destAddr, &fwdAddr, 8 );

  mkRP_DA_Address( &smRpDa, addr.value, addr.length, ET96MAP_ADDRTYPE_SCADDR );
  mkRP_OA_Address( &smRpOa, sms->getOriginatingAddress().value, sms->getOriginatingAddress().length, ET96MAP_ADDRTYPE_MSISDN );
  unsigned length;
  const char* mo_pdu = sms->getBinProperty(Tag::SMSC_MO_PDU, &length);
  if( length > ET96MAP_MAX_SIGNAL_INFO_LEN ) throw runtime_error("MO PDU too long");
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
    delete [] text;
   }
  }


  __map_trace2__("%s: dlg 0x%x forwarding msg %s->%s to sc: %s", __func__, dialog->dialogid_map, sms->getOriginatingAddress().toString().c_str(), sms->getDestinationAddress().toString().c_str(), addr.toString().c_str() );

  checkMapReq( Et96MapOpenReq( SSN INSTDLGARG(dialog), dialog->dialogid_map, &appContext, &destAddr, &dialog->scAddr, 0, 0, 0 ), __func__);
  checkMapReq( Et96MapV2ForwardSmMOReq(SSN INSTDLGARG(dialog),dialog->dialogid_map, 1, &smRpDa, &smRpOa, &ui), __func__);
  checkMapReq( Et96MapDelimiterReq(SSN INSTDLGARG(dialog),dialog->dialogid_map, 0, 0), __func__ );
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
  //SMS_SUMBMIT_FORMAT_HEADER* ssfh = (SMS_SUMBMIT_FORMAT_HEADER*)ud->signalInfo;
  uint8_t hdr=ud->signalInfo[0];
  uint8_t tp_vp=(hdr>>3)&0x3;
  bool udhi=(hdr&0x40)!=0;
  bool reply_path=(hdr&0x80)!=0;
  bool srr=(hdr&0x20)!=0;

  MAP_SMS_ADDRESS* msa = (MAP_SMS_ADDRESS*)(ud->signalInfo+2);
  if( msa->len > 20 ) throw runtime_error( "Address length is invalid in received PDU" );
  unsigned msa_len = msa->len/2+msa->len%2+2;
  unsigned char protocol_id = *(unsigned char*)(ud->signalInfo+2+msa_len);
  unsigned char user_data_coding = *(unsigned char*)(ud->signalInfo+2+msa_len+1);
  sms.setIntProperty(Tag::SMSC_ORIGINAL_DC,user_data_coding);
  unsigned tpvpLen = (tp_vp==0)?0:(tp_vp==2)?1:7;
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
  if ( tp_vp != 0 )
  {
    unsigned char* tvp = (unsigned char*)(ud->signalInfo+2+msa_len+1+1);
//    __map_trace2__("TVP = 0x%x , first octet 0x%x",(unsigned)ssfh->u.head.tp_vp,(unsigned)*tvp);
    time_t timeValue = time(0);
    if ( tp_vp == 2 ){
parse_tvp_scheme1:
      if ( *tvp <= 143 ) timeValue+=(*tvp+1)*(5*60);
      else if ( *tvp <= 167 ) timeValue+=(12*60*60)+((*tvp-143)*(30*60));
      else if ( *tvp <= 196 ) timeValue+=(*tvp-166)*(60*60*24);
      else /*if ( *tvp <= 255 )*/ timeValue+=(*tvp-192)*(60*60*24*7);
    }else if (tp_vp == 1 ){
      if (tpvpLen!=7)
        throw runtime_error(FormatText("%s:incorrect tpvp data",__func__));
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
    }else if (tp_vp == 3 ){
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
      throw runtime_error("required compression");
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
    if ( udhi){
      unsigned udh_len = ((unsigned)*user_data)&0x0ff;
      if ( udh_len >= octet_data_len )
        throw runtime_error(FormatText("octet_data_len %d, but udhi_len %d",octet_data_len,udh_len));
    }
  }
  else
  {
    if ( user_data_len > max_data_len )
      throw runtime_error(FormatText("bad user_data_len %d must be <= %d, PDU len=%d",user_data_len,max_data_len,ud->signalInfoLen));
    if ( udhi){
      unsigned udh_len = ((unsigned)*user_data)&0x0ff;
      if ( udh_len >= user_data_len )
        throw runtime_error(FormatText("user_data_len %d, but udhi_len %d",user_data_len,udh_len));
    }
  }

  {
    if (  encoding == MAP_OCTET7BIT_ENCODING )
    {
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)MAP_SMSC7BIT_ENCODING);
      if ( udhi)
      {
        MicroString ms;
        unsigned char b[255*2];
        unsigned udh_len = ((unsigned)*user_data)&0x0ff;
//        __map_trace2__("ud_length 0x%x udh_len 0x%x",user_data_len,udh_len);
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = user_data_len-x/7;
//        __map_trace2__("text symbols 0x%x bit offset 0x%x",symbols,x-(udh_len+1)*8);
        Convert7BitToSMSC7Bit(user_data+udh_len+1,symbols,&ms,x-(udh_len+1)*8);
        memcpy(b,user_data,udh_len+1);
        memcpy(b+udh_len+1,ms.bytes,ms.len);
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)b,udh_len+1+symbols);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,udh_len+1+symbols);
      }else
      {
        MicroString ms;
        Convert7BitToSMSC7Bit(user_data,user_data_len,&ms,0);
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,ms.bytes,ms.len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
      }
    }
    else
    {
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)encoding);
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)user_data,user_data_len);
      if(!HSNS_isEqual() && encoding==8)
      {
        unsigned msgLen;
        char* msg=(char*)sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&msgLen);
        UCS_ntohs(msg,msg,msgLen,udhi?0x40:0);
      }
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
    }
  }
  {
    unsigned INVALIDVALUE = (unsigned)-1;
    if ( udhi )
    {
      unsigned ref = INVALIDVALUE;
      unsigned msgNum = INVALIDVALUE;
      unsigned msgCount = INVALIDVALUE;

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
          if ( udh[ptr] == 0 )
          {
            ref = udh[ptr+2];
            msgCount = udh[ptr+3];
            msgNum   = udh[ptr+4];
            break;
          }else
          {
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
      dialog->udhiRef = INVALIDVALUE;
      dialog->udhiMsgNum = INVALIDVALUE;
      dialog->udhiMsgCount = INVALIDVALUE;
    }
  }
  unsigned esm_class = 0;
  esm_class |= (udhi?0x40:0);
  esm_class |= (reply_path?0x80:0);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
  //sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
  sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
  sms.setMessageReference(ud->signalInfo[1]);
  if ( srr ) sms.setIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST,1);
  sms.setOriginatingAddress(src_addr);
  ConvAddrMap2Smc(msa,&dest_addr);
  sms.setDestinationAddress(dest_addr);

  dialog->AssignSms(_sms.release());
}

static void SendSubmitCommand(MapDialog* dialog)
{
  if ( dialog->sms.get() == 0 )
    throw runtime_error("MAP::hereis no SMS for submiting");
  Descriptor desc;
  desc.setImsi((uint8_t)dialog->s_imsi.length(),dialog->s_imsi.c_str());
  desc.setMsc((uint8_t)dialog->s_msc.length(),dialog->s_msc.c_str());
  dialog->sms->setOriginatingDescriptor(desc);
  if ( dialog->isUSSD )
  {
    //istringstream(dialog->sms->getOriginatingAddress().value)>>dialog->ussdSequence;
    sscanf(dialog->sms->getOriginatingAddress().value,"%lld",&dialog->ussdSequence);
    {
      MutexGuard ussd_map_guard( ussd_map_lock );
      ussd_map[dialog->ussdSequence] = (dialog->instanceId<<24)|(((unsigned)dialog->ssn)<<16)|dialog->dialogid_map;
    }
  }
  __map_trace2__("%s: dlg 0x%x Submit %s to SMSC: IMSI = %s, MSC = %s, %s",__func__,dialog->dialogid_map,dialog->isUSSD?"USSD":"SMS",dialog->s_imsi.c_str(),dialog->s_msc.c_str(),RouteToString(dialog).c_str());

  /*
  try{
    if(!dialog->noSri)
    {
      MscManager::getMscStatus().report(dialog->s_msc.c_str(),true);
    }
  }
  catch(exception& e)
  {
    __map_warn2__("<exception>: %s",e.what());
  }
   */

  MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
  int rinst=dialog->instanceId;
  if(dialog->lastUssdMessage)
  {
    rinst=0xff;
  }
  uint32_t dialogid_smsc=(rinst<<24)|(((unsigned)dialog->ssn)<<16)|dialog->dialogid_map;
  SmscCommand cmd = SmscCommand::makeSumbmitSm(
    *dialog->sms.get(),dialogid_smsc);
  proxy->putIncomingCommand(cmd);
}

//static const unsigned DIALOGID_BEFORE_CREATING = 0x10000;

static void TryDestroyDialog(unsigned dialogid,bool send_error,unsigned err_code,unsigned ssn,EINSS7INSTANCE_T rinst)
{
  if(dialogid==0 || ssn==0)
  {
    __map_trace2__("%s: invalid dialogid/ssn 0x%x/%d",__func__, dialogid,ssn);
    return;
  }
  DialogRefGuard dialog;
  try
  {
    dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid,ssn,rinst));
    if ( dialog.isnull() )
    {
      __map_trace2__("%s: there is no dlg 0x%x",__func__,dialogid);
      return;
    }
    __require__(dialog->ssn==ssn);
    __map_trace2__("%s: dlg 0x%x state %d",__func__,dialog->dialogid_map,dialog->state);
    if ( send_error)
    {
      dialog->dropChain = true;
      try{
        if ( dialog->isQueryAbonentStatus && dialog->QueryAbonentCommand.IsOk() )
        {
          int status;
          status = AbonentStatus::UNKNOWNVALUE;
          SendAbonentStatusToSmsc(dialog.get(),status);
        }else
        {
          if( dialog->state != MAPST_WaitNextMMS )
          {
            SendErrToSmsc(dialog.get(),err_code);
          }
        }
      }catch(...){
        __map_warn__("TryDestroyDialog: catched exception when send error response to smsc");
      }
    }
    if ( send_error )
    {
      if ( dialog->id_opened )
      {
        AbortMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
        dialog->id_opened = false;
      }
    }else
    {
      switch(dialog->state)
      {
        case MAPST_ABORTED:
          if ( dialog->id_opened ) {
            AbortMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
            dialog->id_opened = false;
          }
          break;
        default:
          CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
      }
    }
  }catch(std::exception &e)
  {
    __map_warn2__("TryDestroyDialog: catched exception: %s", e.what());
  }catch(...)
  {
    __map_warn__("TryDestroyDialog: catched unexpected exception");
  }
  DropMapDialog(dialog.get());
}

static String64 RouteToString(MapDialog* dialog)
{
  static String64 skiped("<unknown> -> <unknown>");
  if ( dialog->sms.get() == 0 ) return skiped;
  //auto_ptr<char> b(new char[1024]);
  //memset(b.get(),0,1024);
  String64 rv;
  snprintf(rv.str,64,"%d.%d.%s -> %d.%d.%s",
    dialog->sms->getOriginatingAddress().getTypeOfNumber(),
    dialog->sms->getOriginatingAddress().getNumberingPlan(),
    dialog->sms->getOriginatingAddress().value,
    dialog->sms->getDestinationAddress().getTypeOfNumber(),
    dialog->sms->getDestinationAddress().getNumberingPlan(),
    dialog->sms->getDestinationAddress().value);
  return rv;
}

#define MAP_TRY  try {
#define MAP_CATCH(__dialogid_map,__dialogid_smsc,__ssn,__rinst)     \
}catch(MAPDIALOG_HEREISNO_ID& x){\
  __map_warn2__("%s: There is no dialogid 0x%x/0x%x <exception>:%s",__func__,__dialogid_map,__dialogid_smsc,x.what());\
}catch(MAPDIALOG_ERROR& err){\
  __map_warn2__("%s: error dialogid 0x%x/0x%x <exception>:%s",__func__,__dialogid_map,__dialogid_smsc,err.what());\
  TryDestroyDialog(__dialogid_map,true,err.code,__ssn,__rinst);\
}catch(exception& e){\
  __map_warn2__("%s: exception dialogid 0x%x/0x%x <exception>:%s",__func__,__dialogid_map,__dialogid_smsc, e.what());\
  TryDestroyDialog(__dialogid_map,true,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE),__ssn,__rinst);\
}catch(...){\
  __map_warn2__("%s: exception dialogid 0x%x/0x%x <exception>:...",__func__,__dialogid_map,__dialogid_smsc);\
  TryDestroyDialog(__dialogid_map,true,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE),__ssn,__rinst);\
}

static bool SendSms(MapDialog* dialog)
{
  dialog->wasDelivered = false;

  MscState mscSt=dialog->noSri?mscUnlocked:MscManager::getMscStatus().check(dialog->s_msc.c_str());
  if ( mscSt == mscLocked)
    throw MAPDIALOG_TEMP_ERROR("MSC BLOCKED",Status::BLOCKEDMSC);

  /*if(mscSt==mscUnlockedOnce)
  {
    dialog->needReportMsc=true;
  }
   */

  bool mms = FALSE;
  if( dialog->version > 1 )
  {
    mms=!dialog->chain.empty() || dialog->sms.get()->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND);
  }
  if ( dialog->version < 2 )
  {
    mms = false;
  }
//  mms = false;
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_MT_RELAY;
  SetVersion(appContext,dialog->version);
  bool segmentation = false;
  //__map_trace2__("%s: chain size is %d mms=%d dlg->mms=%s dlg->invoke=%d",__func__,dialog->chain.size(),mms,dialog->mms?"true":"false", (int)dialog->invokeId);

  dialog->state = MAPST_WaitSmsConf;

  dialog->smRpOa.typeOfAddress = ET96MAP_ADDRTYPE_SCADDR;
  dialog->smRpOa.addrLen = (dialog->m_scAddr.addressLength+1)/2+1;
  dialog->smRpOa.addr[0] = dialog->m_scAddr.typeOfAddress;
  memcpy( dialog->smRpOa.addr+1, dialog->m_scAddr.address, (dialog->m_scAddr.addressLength+1)/2 );

  ET96MAP_SM_RP_UI_T* ui;
  dialog->auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(ui=new ET96MAP_SM_RP_UI_T);
  mkDeliverPDU(dialog->sms.get(),ui,mms);
  if ( !dialog->mms )
  {
    dialog->invokeId = 0;
    dialog->state = MAPST_WaitOpenConf;
    checkMapReq( Et96MapOpenReq(SSN INSTDLGARG(dialog),dialog->dialogid_map,&appContext,&dialog->destMscAddr,&dialog->scAddr,0,0,0), __func__);
    dialog->id_opened = true;
  }

  if ( dialog->version > 1
       && !dialog->mms
       && (ui->signalInfoLen > (143-(dialog->smRpOa.addrLen+1)-(dialog->smRpDa.addrLen+1)-(dialog->scAddr.ss7AddrLen))
       || mms ) )
  {
    checkMapReq( Et96MapDelimiterReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, 0, 0 ), __func__);
    segmentation = true;
    dialog->state = MAPST_WaitSpecOpenConf;
  }else
  {
    dialog->invokeId++;
    if ( dialog->version == 2 )
    {
      checkMapReq( Et96MapV2ForwardSmMTReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), mms?TRUE:FALSE), __func__);
    }else if ( dialog->version == 1 )
    {
      checkMapReq( Et96MapV1ForwardSmMT_MOReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get()), __func__);
    }else
    {
      throw runtime_error(
      FormatText("MAP::SendSMSCToMT: incorrect dialog version %d",dialog->version));
    }
    checkMapReq( Et96MapDelimiterReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, 0, 0 ), __func__);
  }
  dialog->mms = mms;
  return segmentation;
}

static void SendNextMMS(MapDialog* dialog)
{
  __map_trace2__("%s: dialogid 0x%x  (state %d/NEXTMMS)",__func__,dialog->dialogid_map,dialog->state);
  SmscCommand cmd;
  bool needAbortDialog=false;
  {
    MutexGuard mg(dialog->mutex);
    if ( dialog->chain.empty() )
    {
      if( dialog->sms.get()->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND) )
      {
        if( !dialog->wasDelivered )
        {
          __map_trace__("SendNextMMS: messages was not delivered. Aborting long message sending");
          needAbortDialog=true;
          dialog->isDropping=true;
        } else
        {
          __map_trace2__("SendNextMMS: no messages in chain. Waiting long message next part for %d", dialog->sms.get()->getConcatMsgRef());
          dialog->state = MAPST_WaitNextMMS;
          return;
        }
      } else throw runtime_error("SendNextMMS: has no messages to send");
    }
    if(!needAbortDialog)
    {
      cmd = dialog->chain.front();
      dialog->chain.pop_front();
    }
  }
  if(needAbortDialog)
  {
    AbortMapDialog( dialog->dialogid_map, dialog->ssn ,dialog->instanceId);
    DropMapDialog(dialog);
  }else
  {
    dialog->dialogid_smsc = cmd->get_dialogId();
    dialog->AssignSms(cmd->get_sms_and_forget());
    SendSms(dialog);
  }
}

static void SendSegmentedSms(MapDialog* dialog)
{
// We have already opened dialog and don't need to check this condition.
//  CheckLockedByMO(dialog);
  dialog->invokeId++;
  if ( dialog->version == 2 ) {
    checkMapReq( Et96MapV2ForwardSmMTReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), dialog->mms), __func__);
  }else if ( dialog->version == 1 ) {
//    result = Et96MapV1ForwardSmMT_MOReq( dialog->ssn, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get());
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("SendSegmentedSms: MAPv1 do not support/required segmentation dialogid 0x%x",dialog->dialogid_map),MAP_FALURE);
  }else throw runtime_error(
    FormatText("SendSegmentedSms: incorrect dialog version %d",dialog->version));
  checkMapReq( Et96MapDelimiterReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, 0, 0 ), __func__);
}

static ET96MAP_USSD_DATA_CODING_SCHEME_T fillUSSDString(unsigned encoding, const unsigned char* text, unsigned text_len, ET96MAP_USSD_STRING_T* ussdString)
{
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0x0f;
  unsigned bytes = 0;
  if( encoding == MAP_UCS2_ENCODING )
  {
    if( text_len > ET96MAP_MAX_USSD_STR_LEN )
      throw VeryLongText();
    bytes = text_len;
    memcpy( ussdString->ussdStr, text, text_len );
    ussdEncoding = 0x48;
  } else if( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING || encoding == MAP_SMSC7BIT_ENCODING )
  {
    if (encoding == MAP_SMSC7BIT_ENCODING ) {
      bytes = ConvertSMSC7bit27bit(text,text_len,ussdString->ussdStr,0,ET96MAP_MAX_USSD_STR_LEN);
      // if buffer have trailing 7 unfilled bits place <cr> there
      if( bytes*8-text_len*7 == 7 ) ussdString->ussdStr[bytes-1] |= (0x0D<<1);
    } else {
      unsigned elen = 0;
      bytes = ConvertText27bit(text,text_len,ussdString->ussdStr,&elen,0,ET96MAP_MAX_USSD_STR_LEN);
      // if buffer have trailing 7 unfilled bits place <cr> there
      if( bytes*8-elen*7 == 7 ) ussdString->ussdStr[bytes-1] |= (0x0D<<1);
    }
    ussdEncoding = 0x0f;
  } else
  { //8 bit
    if( text_len > ET96MAP_MAX_USSD_STR_LEN )
      throw VeryLongText();
    bytes = text_len;
    memcpy( ussdString->ussdStr, text, text_len );
    ussdEncoding = 0x44;
  }
  ussdString->ussdStrLen = bytes;
  return ussdEncoding;
}

static int makeUssdErrorText( MapDialog* dialog,char *text, unsigned* textLen, int code, int routeErr = 0)
{
  DummyGetAdapter ga;
  ContextEnvironment ce;

  int dc=0;
  std::string locale = "en_en";
  if( dialog->version > 1 && dialog->sms.get() )
  {
    smsc::profiler::Profile p=Smsc::getInstance().getProfiler()->lookup(dialog->sms->getOriginatingAddress().toString().c_str());
    dc=p.codepage;
    locale = p.locale;
  }


  if( code > 0 )
  {
    char buf[32];
    sprintf(buf,"%d",code);
    std::string reason=ResourceManager::getInstance()->getString(locale,((string)"reason.")+buf);
    ce.exportStr("msg",reason.c_str());
    ce.exportInt("code",code);
  } else if( code == -1 )
  {
    std::string reason=ResourceManager::getInstance()->getString(locale,"ussd_invalid_cmd");
    ce.exportStr("msg",reason.c_str());
    ce.exportInt("code",0);
  } else
  {
    std::string reason=ResourceManager::getInstance()->getString(locale,"ussd_route_err");
    ce.exportStr("msg",reason.c_str());
    ce.exportInt("code",routeErr);
  }

  OutputFormatter* ofDelivered=ResourceManager::getInstance()->getFormatter(locale,"ussd_error");
  string out;
  if(!ofDelivered)
  {
    out="Unknown formatter ussd_error for locale ";
    out+=locale;
  } else
  {
    try{
      ofDelivered->format(out,ga,ce);
    }catch(exception& e)  {
      out="Formatting error: ";
      out+=e.what();
    }
  }
  if((dc&smsc::profiler::ProfileCharsetOptions::UssdIn7Bit) || !(dc&smsc::profiler::ProfileCharsetOptions::Ucs2))
  {
    if(smsc::util::hasHighBit(out.c_str(),out.length()))
    {
      char buf[256];
      Transliterate(out.c_str(),out.length(),CONV_ENCODING_CP1251,buf,sizeof(buf));
      out=buf;
      dc=0;
    }
  }else
  {
    if((dc&smsc::profiler::ProfileCharsetOptions::Ucs2) && smsc::util::hasHighBit(out.c_str(),out.length()))
    {
      short buf[256];
      size_t len=ConvertMultibyteToUCS2(out.c_str(),out.length(),buf,sizeof(buf),CONV_ENCODING_CP1251);
      for(size_t i=0;i<len/2;i++)
      {
        buf[i]=htons(buf[i]);
      }
      out.assign((char*)buf,len);
      dc=8;
    }else
    {
      dc=0;
    }
  }
  *textLen=(unsigned)out.length();
  size_t len=out.length();
  if(len>160)
  {
    len=160;
  }
  memcpy(text,out.c_str(),len);
  return dc;
}

static void DoUSSRUserResponceError(const SmscCommand& cmd , MapDialog* dialog)
{
  __map_trace2__("%s: dialogid 0x%x",__func__,dialog->dialogid_map);
  char text[1024] = {0,};
  unsigned textLen=0;
  int dc;
  if( cmd.IsOk() ) {
    if( cmd->get_commandId() == SUBMIT_RESP ) {
      dc=makeUssdErrorText(dialog,text, &textLen,cmd->get_resp()->get_status()&0x0000FFFF);
    } else {
      dc=makeUssdErrorText(dialog,text, &textLen,-1);
    }
  } else {
    dc=makeUssdErrorText(dialog,text, &textLen,-2, dialog->routeErr);
  }

  if ( dialog->version == 2 )
  {

    ET96MAP_USSD_STRING_T ussdString = {0,};
    ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding =
      fillUSSDString(dc,(unsigned char*)text,textLen, &ussdString);
    checkMapReq( Et96MapV2ProcessUnstructuredSSRequestResp(
                                                           dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,dialog->origInvokeId,
                                                           &ussdEncoding,
                                                           &ussdString,
                                                           0), __func__);
  } else if( dialog->version == 1 ) {
    if ( textLen > ET96MAP_MAX_SS_USER_DATA_LEN )
      throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} very long msg text %d",__func__,dialog->dialogid_map,textLen));

    ET96MAP_SS_USER_DATA_T ussdData;
    memcpy(ussdData.ssUserDataStr,text,textLen);
    ussdData.ssUserDataStrLen = textLen;
    checkMapReq( Et96MapV1ProcessUnstructuredSSDataResp( dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,dialog->origInvokeId,&ussdData, 0 ), __func__);
  } else throw runtime_error( FormatText("%s: incorrect dialog version %d",__func__,dialog->version));

  CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
  eraseUssdLock(dialog, __func__);
  dialog->dropChain = true;
  dialog->state = MAPST_END;
  if( cmd.IsOk() && cmd->get_commandId() != SUBMIT_RESP) SendOkToSmsc(dialog);
  DropMapDialog(dialog);
}

/*static long long NextSequence()
{
  static long long sequence = (((long long)time(0))<<32);
  return ++sequence;
}*/

static void DoUSSRUserResponce( MapDialog* dialog)
{
  __map_trace2__("%s: dialogid 0x%x",__func__,dialog->dialogid_map);
  unsigned encoding = dialog->sms->getIntProperty(Tag::SMPP_DATA_CODING);
  unsigned text_len;

  char tmpBuf[ET96MAP_MAX_USSD_STR_LEN+1];
  const uint8_t* text;
  if(dialog->sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    text=(const uint8_t*)dialog->sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&text_len);
  }else
  {
    text=(const uint8_t*)dialog->sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
  }
  if(encoding==8 && !HSNS_isEqual())
  {
    UCS_htons(tmpBuf,(const char*)text,text_len>ET96MAP_MAX_USSD_STR_LEN?ET96MAP_MAX_USSD_STR_LEN:text_len,dialog->sms->getIntProperty(Tag::SMPP_ESM_CLASS));
    text=(uint8_t*)tmpBuf;
  }

  if(text_len==0)
  {
    __map_warn2__("%s: attempt to send empty ussd string seq=%d, oa=%s,srcSme=%s,da=%s",
                  __func__,dialog->dialogid_smsc,dialog->sms->getOriginatingAddress().toString().c_str(),
                  dialog->sms->getSourceSmeId(),dialog->sms->getDestinationAddress().toString().c_str());
    text=(const unsigned char*)"\x0d";
    text_len=1;
    encoding=MAP_LATIN1_ENCODING;
  }

  if ( dialog->version == 2 )
  {
    /*
    if ( text_len > ET96MAP_MAX_USSD_STR_LEN )
      throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} very long msg text %d",__func__,dialog->dialogid_map,text_len));
    */

    ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0;
    try {
      ET96MAP_USSD_STRING_T ussdString = {0,};
      ussdEncoding = fillUSSDString( encoding, text, text_len, &ussdString );
      checkMapReq( Et96MapV2ProcessUnstructuredSSRequestResp(
                                                             dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,dialog->origInvokeId,
                                                             &ussdEncoding,
                                                             &ussdString,
                                                             0), __func__);
      SendOkToSmsc(dialog);
    } catch (VeryLongText &t) {
      char errtext[1024] = {0,};
      unsigned textLen=0;
      ET96MAP_USSD_STRING_T ussdString = {0,};
      int dc=makeUssdErrorText(dialog,errtext, &textLen,Status::USSDMSGTOOLONG );
      int err_text_len = (int)strlen(errtext);
      __map_warn2__("%s: dlg 0x%x very long ussd string %d %s",__func__,dialog->dialogid_map, text_len, errtext);
      ussdEncoding = fillUSSDString( dc, (unsigned char *)errtext, err_text_len, &ussdString );
      checkMapReq( Et96MapV2ProcessUnstructuredSSRequestResp(
                                                             dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,dialog->origInvokeId,
                                                             &ussdEncoding,
                                                             &ussdString,
                                                             0), __func__);
      SendErrToSmsc(dialog,MAKE_ERRORCODE(CMD_ERR_PERM,Status::USSDMSGTOOLONG));
    }
  } else if( dialog->version == 1 )
  {
    if ( text_len > ET96MAP_MAX_SS_USER_DATA_LEN )
      throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} very long msg text %d",__func__,dialog->dialogid_map,text_len));

    char data[ET96MAP_MAX_SS_USER_DATA_LEN*3];
    int data_len = 0;
    if( encoding == MAP_UCS2_ENCODING ) {
      char buf[ET96MAP_MAX_SS_USER_DATA_LEN];
      int len = ConvertUCS2ToMultibyte((const short*)text,text_len,buf,ET96MAP_MAX_SS_USER_DATA_LEN,CONV_ENCODING_CP1251);
      data_len=Transliterate(buf,len,CONV_ENCODING_CP1251,data,ET96MAP_MAX_SS_USER_DATA_LEN*3);
      if ( data_len > ET96MAP_MAX_SS_USER_DATA_LEN || data_len == -1)
        throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} very long msg text %d",__func__,dialog->dialogid_map,text_len));
      text = (const unsigned char*)data;
      text_len = data_len;
    }

    ET96MAP_SS_USER_DATA_T ussdData;
    memcpy(ussdData.ssUserDataStr,text,text_len);
    ussdData.ssUserDataStrLen = text_len;
    checkMapReq( Et96MapV1ProcessUnstructuredSSDataResp( dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,dialog->origInvokeId,&ussdData, 0 ), __func__);
    SendOkToSmsc(dialog);
  }
  else
  {
    throw runtime_error( FormatText("%s: incorrect dialog version %d",__func__,dialog->version));
  }
  CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
  eraseUssdLock(dialog, __func__);
  dialog->state = MAPST_END;
  DropMapDialog(dialog);

}

static bool isVlrUssdOp(int serviceOp)
{
  return serviceOp==USSD_USSR_REQ_VLR || serviceOp==USSD_USSR_REQ_VLR_LAST ||
         serviceOp==USSD_USSN_REQ_VLR || serviceOp==USSD_USSN_REQ_VLR_LAST;
}


static void DoUSSDRequestOrNotifyReq(MapDialog* dialog)
{
  dialog->isUSSD = true;
  __map_trace2__("%s: dialogid 0x%x opened=%s invoke=%d ssn=%d",__func__,dialog->dialogid_map,dialog->id_opened?"true":"false", dialog->invokeId, dialog->ssn);
  if( !dialog->id_opened )
  {
    bool dlg_found = false;
    //istringstream(string(dialog->sms->getDestinationAddress().value))>>dialog->ussdSequence;
    sscanf(dialog->sms->getDestinationAddress().value,"%lld",&dialog->ussdSequence);
    {
      MutexGuard mg(ussd_map_lock);
      USSD_MAP::iterator it = ussd_map.find(dialog->ussdSequence);
      if ( it != ussd_map.end() )
      {
        // USSD dialog already exists on this abonent
        dlg_found = true;
      } else
      {
        ussd_map[dialog->ussdSequence] = (dialog->instanceId<<24)|(((unsigned)dialog->ssn)<<16)|dialog->dialogid_map;
      }
    }
    if(dlg_found)
    {
      __map_trace2__("%s: dialogid 0x%x, ussd dialog already exists for %s",__func__,dialog->dialogid_map,dialog->abonent.c_str());
      SendErrToSmsc(dialog,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::SUBSCRBUSYMT));
      dialog->state = MAPST_END;
      DropMapDialog(dialog);
      return;
    }
  }
  if( dialog->id_opened && dialog->version != 2 ) {
    throw runtime_error( FormatText("%s: incorrect dialog version %d",__func__,dialog->version));
  }
  ET96MAP_USSD_STRING_T ussdString = {0,};
  unsigned text_len;
  unsigned encoding = dialog->sms->getIntProperty(Tag::SMPP_DATA_CODING);

  char tmpBuf[ET96MAP_MAX_USSD_STR_LEN+1];
  const uint8_t* text;
  if(dialog->sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    text=(const uint8_t*)dialog->sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&text_len);
  }else
  {
    text=(const uint8_t*)dialog->sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
  }
  if(encoding==8 && !HSNS_isEqual())
  {
    UCS_htons(tmpBuf,(const char*)text,text_len>ET96MAP_MAX_USSD_STR_LEN?ET96MAP_MAX_USSD_STR_LEN:text_len,dialog->sms->getIntProperty(Tag::SMPP_ESM_CLASS));
    text=(uint8_t*)tmpBuf;
  }

  if(text_len==0)
  {
    __map_warn2__("%s: attempt to send empty ussd string seq=%d, oa=%s,srcSme=%s,da=%s",
                  __func__,dialog->dialogid_smsc,dialog->sms->getOriginatingAddress().toString().c_str(),
                  dialog->sms->getSourceSmeId(),dialog->sms->getDestinationAddress().toString().c_str());
    text=(const unsigned char*)"\x0d";
    text_len=1;
    encoding=MAP_LATIN1_ENCODING;
  }

  /*
  if ( text_len > ET96MAP_MAX_USSD_STR_LEN )
    throw runtime_error(FormatText("%s: dlg=0x%x very long msg text %d",__func__,dialog->dialogid_map,text_len));
  */

  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdEncoding = 0;

  try {
    ussdEncoding = fillUSSDString(encoding,text,text_len, &ussdString);
  } catch (VeryLongText &t) {
    __map_warn2__("%s: dlg 0x%x very long ussd string %d",__func__,dialog->dialogid_map, text_len);
    SendErrToSmsc(dialog,MAKE_ERRORCODE(CMD_ERR_PERM,Status::USSDMSGTOOLONG));
    if(dialog->id_opened) {
      char errtext[1024] = {0,};
      unsigned textLen=0;
      ET96MAP_USSD_STRING_T ussdErrString = {0,};
      int dc=makeUssdErrorText(dialog,errtext, &textLen,Status::USSDMSGTOOLONG );
      int err_text_len = (int)strlen(errtext);
      ussdEncoding = fillUSSDString( dc, (unsigned char *)errtext, err_text_len, &ussdErrString );
      ET96MAP_ALERTING_PATTERN_T alertPattern = ET96MAP_ALERTING_PATTERN_LEVEL2;
      checkMapReq( Et96MapV2UnstructuredSSNotifyReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, ussdEncoding, ussdErrString, &alertPattern), __func__);
      dialog->state = MAPST_WaitUSSDErrorClose;
      checkMapReq( Et96MapDelimiterReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, 0, 0 ), __func__);
      return;
    } else {
      dialog->state = MAPST_END;
      DropMapDialog(dialog);
      return;
    }
  }

  int serviceOp = dialog->sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP);
  if( serviceOp == USSD_USSR_REQ || serviceOp==USSD_USSR_REQ_LAST ||
      serviceOp==USSD_USSR_REQ_VLR || serviceOp==USSD_USSR_REQ_VLR_LAST)
  {
    dialog->state = MAPST_WaitUSSDReqConf;
  } else
  {
    dialog->state = MAPST_WaitUSSDNotifyConf;
  }
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

    if( serviceOp == USSD_USSR_REQ || serviceOp==USSD_USSR_REQ_LAST ||
        serviceOp==USSD_USSR_REQ_VLR || serviceOp==USSD_USSR_REQ_VLR_LAST)
    {
      dialog->state = MAPST_WaitUSSDReqOpenConf;
    } else
    {
      dialog->state = MAPST_WaitUSSDNotifyOpenConf;
    }
    String64 subsystem = ".5.0.ussd:";
    strcat(subsystem.str,dialog->sms->getOriginatingAddress().value);
    dialog->subsystem = subsystem.c_str();
    dialog->invokeId=0;
    ET96MAP_ADDRESS_T origRef;
    mkMapAddress( &origRef, dialog->sms->getOriginatingAddress() );
    ET96MAP_IMSI_OR_MSISDN_T destRef;
    memset(&destRef,0,sizeof(destRef));
    if(isVlrUssdOp(serviceOp))
    {
      ET96MAP_IMSI_OR_MSISDN_T* destRefPtr=&destRef;
      DestRefValue drv=NetworkProfiles::getInstance().lookup(dialog->sms->getDestinationAddress()).drv;
      if(drv==drvDestAddr)
      {
        mkIMSIOrMSISDNFromAddress( &destRef, dialog->sms->getDestinationAddress() );
      }else if(drv==drvDestIMSI)
      {
        if( dialog->s_imsi.length() > 0 )
        {
          mkIMSIOrMSISDNFromIMSI( &destRef, dialog->s_imsi );
        } else
        {
          SendErrToSmsc(dialog,MAKE_ERRORCODE(CMD_ERR_PERM,Status::SYSERR));
          dialog->state = MAPST_END;
          DropMapDialog(dialog);
          return;
        }
      }else
      {
        destRefPtr=0;
      }
      ET96MAP_SS7_ADDR_T destAddr=dialog->destMscAddr;
      destAddr.ss7Addr[1]=7;
      checkMapReq( Et96MapOpenReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, &appContext, &destAddr, GetUSSDAddr(), destRefPtr, 0/*&origRef*/, 0/*&specificInfo*/ ), __func__);
    }else
    {
      ET96MAP_IMSI_OR_MSISDN_T* destRefPtr=&destRef;
      DestRefValue drv=NetworkProfiles::getInstance().lookup(dialog->sms->getDestinationAddress()).drv;
      if(drv==drvDestAddr)
      {
        mkIMSIOrMSISDNFromAddress( &destRef, dialog->sms->getDestinationAddress() );
      }else if(drv==drvDestIMSI)
      {
        if( dialog->s_imsi.length() > 0 )
        {
          mkIMSIOrMSISDNFromIMSI( &destRef, dialog->s_imsi );
        }else
        {
          __map_warn2__("%s: dlg 0x%x missing imsi for destRef",__func__,dialog->dialogid_map);
          SendErrToSmsc(dialog,MAKE_ERRORCODE(CMD_ERR_PERM,Status::MISSINGIMSIINNIUSSD));
          dialog->state = MAPST_END;
          DropMapDialog(dialog);
          return;
        }
      }else
      {
        destRefPtr=0;
      }

      checkMapReq( Et96MapOpenReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, &appContext, &dialog->mshlrAddr, GetUSSDAddr(), destRefPtr, &origRef, &specificInfo ), __func__);
    }
  }
  dialog->invokeId++;
  ET96MAP_ALERTING_PATTERN_T alertPattern = ET96MAP_ALERTING_PATTERN_LEVEL2;
  if( serviceOp==USSD_USSR_REQ     || serviceOp==USSD_USSR_REQ_LAST ||
      serviceOp==USSD_USSR_REQ_VLR || serviceOp==USSD_USSR_REQ_VLR_LAST)
  {
    checkMapReq( Et96MapV2UnstructuredSSRequestReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, ussdEncoding, ussdString, &alertPattern), __func__);
  } else
  {
    checkMapReq( Et96MapV2UnstructuredSSNotifyReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, dialog->invokeId, ussdEncoding, ussdString, &alertPattern), __func__);
  }
  checkMapReq( Et96MapDelimiterReq( dialog->ssn INSTDLGARG(dialog), dialog->dialogid_map, 0, 0 ), __func__);
}

void MAPIO_PutCommand(const SmscCommand& cmd ){
//  MutexGuard mapMutexGuard(mapMutex);
  MAPIO_PutCommand(cmd, 0 );
}

static bool isValidUssdOp(int serviceOp)
{
  return serviceOp == USSD_USSR_REQ || serviceOp == USSD_USSN_REQ ||
      serviceOp == USSD_PSSR_RESP || serviceOp == USSD_USSR_REQ_LAST ||
      serviceOp == USSD_USSN_REQ_LAST || serviceOp == USSD_USSR_REQ_VLR ||
      serviceOp == USSD_USSR_REQ_VLR_LAST || serviceOp == USSD_USSN_REQ_VLR ||
      serviceOp == USSD_USSN_REQ_VLR_LAST || serviceOp == USSD_USSREL_REQ ||
      serviceOp == USSD_REDIRECT;
}


static void MAPIO_PutCommand(const SmscCommand& cmd, MapDialog* dialog2 )
{
  unsigned dialogid_smsc = cmd->get_dialogId();
  unsigned dialogid_map = 0;
  unsigned dialog_ssn = 0;
  EINSS7INSTANCE_T rinst = 0;
  DialogRefGuard dialog;
  UssdProcessingGuard ussdGuard;

  if( MAP_connectedInstCount==0 || !isMapBound() )
  {
    __map_warn__("MAP is not bound yet");
    if(cmd->get_commandId()==DELIVERY)
    {
      SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_TEMP,Status::SMENOTCONNECTED),
          cmd->get_sms()->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP),cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
    }
    return;
  }


  MAP_TRY {
    if ( cmd->get_commandId() != SUBMIT_RESP )
    {
      if ( cmd->get_commandId() != DELIVERY && cmd->get_commandId() != QUERYABONENTSTATUS)
      {
        __map_warn__("putCommand: must be DELIVERY or QUERYABONENTSTATUS");
        return;
      }
      // this is deliver command
      if ( cmd->get_commandId() == DELIVERY  && cmd->get_sms()->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP ) )
      { // DELIVERY ussd operation
        unsigned serviceOp = cmd->get_sms()->getIntProperty(Tag::SMPP_USSD_SERVICE_OP );
        __map_trace2__("putCommand dialogid_smsc /0x%x USSD OP %d",dialogid_smsc, serviceOp);
        int64_t sequence=0;
        sscanf(cmd->get_sms()->getDestinationAddress().value,"%lld",&sequence);
        if ( sequence == 0 )
        {
          __map_warn2__("PutCommand: invalid dest addr for ussd='%s'",cmd->get_sms()->getDestinationAddress().value);
          SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::INVDSTADR),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
          return;
        }
        bool dlg_found = false;
        {
          MutexGuard mg(ussd_map_lock);
          USSD_MAP::iterator it = ussd_map.find(sequence);
          if ( it != ussd_map.end() )
          {
            dialogid_map = it->second&0xFFFF; // low short value is dlgid
            dialog_ssn = (it->second>>16)&0xFF; //high short value is ssn
            rinst=(it->second>>24)&0xFF;
            dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_map,dialog_ssn,rinst));
            if( !dialog.isnull() )
            {
              if ( !dialog->isUSSD )
                throw MAPDIALOG_FATAL_ERROR(
                    FormatText("putCommand: Found dialog is not USSD for smsc_id:0x%x, ussd_seq: %lld",dialogid_smsc,sequence));
              __require__(dialog->ssn == dialog_ssn);
              dlg_found = true;
              ussdGuard.lockProcessing(dialog.get());
            } else
            {
              __map_trace2__("%s: ussd lock found for %lld dialogid 0x%x but no dialog exists, erase ussd lock", __func__,sequence,dialogid_map);
              ussd_map.erase(sequence);
            }
          }
          if(dlg_found && serviceOp==USSD_USSREL_REQ)
          {
            ussd_map.erase(sequence);
          }
        }

        if(isValidUssdOp(serviceOp))
        {
          if( dlg_found )
          {
            unsigned mr = cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)&0x0ffffff;
            if ( !((dialog->ussdMrRef == mr) || ((mr&0xff0000)==0 && (mr&0xffff)==(dialog->ussdMrRef&0xffff))))
            {
              SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGREFMISM),true,mr);
              __map_warn2__("putCommand: dlgId:0x%x bad message_reference 0x%x, must be 0x%x",
                  dialog->dialogid_map,mr,dialog->ussdMrRef);
              return;
            }
            dialog->lastUssdMessage=serviceOp==USSD_USSR_REQ_LAST || serviceOp==USSD_USSN_REQ_LAST || serviceOp==USSD_USSR_REQ_VLR_LAST || serviceOp==USSD_USSN_REQ_VLR_LAST;
            dialog->id_opened = true;
            dialog->dialogid_smsc = dialogid_smsc;
            if (dialog->state == MAPST_WaitSubmitCmdConf || dialog->state == MAPST_WaitSubmitUSSDRequestConf )
            {
              // Seems deliver goes earlier than submitResp
              MutexGuard mg(dialog->mutex);
              if(!dialog->isDropping)
              {
                if(!dialog->chain.empty())
                {
                  __map_warn2__("Attempt to chain 2nd command(smscid=%d) to ussd dialog dlgId=%x",dialogid_smsc,dialog->dialogid_map);
                  SendStatusToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::USSDDLGREFMISM),dialog->isUSSD,dialog->ussdMrRef);
                  return;
                }
                __map_trace2__("%s: dialogid 0x%x deliver earlier then submit resp for USSD dlg, deliver was chained", __func__,dialog->dialogid_map);
                dialog->chain.insert(dialog->chain.begin(), cmd);
              }else
              {
                SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND),true,mr);
                __map_warn2__("putCommand: dlgId:0x%x isDropping while trying to chain new command",
                    dialog->dialogid_map);
              }
              return;
            } else if ( !(dialog->state == MAPST_ReadyNextUSSDCmd || dialog->state == MAPST_USSDWaitResponce))
            {
              throw MAPDIALOG_BAD_STATE(
                  FormatText("putCommand: ussd req/notify bad state %d, MAP.did 0x%x, SMSC.did 0x%x",dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
            }
            if(serviceOp==USSD_USSREL_REQ)
            {
              warnMapReq( Et96MapUAbortReq( dialog_ssn INSTDLGARG(dialog), dialogid_map, 0, 0, 0, 0 ), __func__);
              if(dialog->state == MAPST_WaitUSSDReqConf || dialog->state == MAPST_WaitUSSDNotifyConf ||
                 dialog->state == MAPST_WaitUSSDReqOpenConf || dialog->state == MAPST_WaitUSSDNotifyOpenConf)
              {
                SendErrToSmsc(dialog.get(),MAKE_ERRORCODE(CMD_ERR_PERM,Status::USSDSESSIONTERMABN));
              }
              SendStatusToSmsc(dialogid_smsc,0,true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
              DropMapDialog(dialog.get());
              return;
            }

            if(serviceOp==USSD_REDIRECT)
            {
              unsigned len;
              const char* val=cmd->get_sms()->getBinProperty(Tag::SMPP_CALLBACK_NUM,&len);
              if(len<4 || *val!=1)
              {
                SendStatusToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_PERM,Status::INVOPTPARAMVAL),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
                return;
              }
              SendStatusToSmsc(dialogid_smsc,Status::OK,true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));

              Address redirectAddr(len-3,val[1],val[2],val+3);///from SMPP_CALLBACK_NUM
              SMS sms=*cmd->get_sms();
              sms.setOriginatingAddress(sms.getDestinationAddress());
              sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_IND);
              Descriptor desc;
              desc.setImsi((uint8_t)dialog->s_imsi.length(),dialog->s_imsi.c_str());
              desc.setMsc((uint8_t)dialog->s_msc.length(),dialog->s_msc.c_str());
              sms.setOriginatingDescriptor(desc);
              dialog->subsystem=".5.0.ussd:";
              dialog->subsystem+=redirectAddr.value;
              __map_trace2__("ussd redirect: %s->%s",sms.getOriginatingAddress().toString().c_str(),dialog->subsystem.c_str());
              sms.setDestinationAddress(dialog->subsystem.c_str());
              MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
              int rinst2=dialog->instanceId;
              if(dialog->lastUssdMessage)
              {
                rinst2=0xff;
              }
              dialog->state = MAPST_WaitSubmitCmdConf;
              uint32_t dialogid_smsc2=(rinst2<<24)|(((unsigned)dialog->ssn)<<16)|dialog->dialogid_map;
              SmscCommand cmd2 = SmscCommand::makeSumbmitSm(
                sms,dialogid_smsc2);
              proxy->putIncomingCommand(cmd2);
              return;
            }

            dialog->isQueryAbonentStatus = false;
            dialog->AssignSms(cmd->get_sms_and_forget());
            if(serviceOp == USSD_PSSR_RESP)
            {
              DoUSSRUserResponce(dialog.get());
            }else
            {
              DoUSSDRequestOrNotifyReq(dialog.get());
            }
            return;
          } else
          {
            if ( serviceOp == USSD_PSSR_RESP )
            {
              SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
              __map_warn2__("putCommand: USSD dialog not found for smsc_id:0x%x ussd_seq: %lld",dialogid_smsc,sequence);
              return;
            }

            if( cmd->get_sms()->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) )
            {
              SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::INVOPTPARAMVAL),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
              __map_warn2__("putCommand: There is no USSD dialog for MR %d smsc_dlg 0x%x seq: %lld",cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE), dialogid_smsc,sequence);
              return;
            }
            __map_trace2__("%s: trying to create USSD network intiated session dialogid_smsc 0x%x",__func__,dialogid_smsc);
            if( !dialog2 )
            { // not chained dialog
              try {
                dialog.assign(MapDialogContainer::getInstance()->
                    createOrAttachSMSCUSSDDialog(
                        dialogid_smsc,
                        SSN,
                        string(cmd->get_sms()->getDestinationAddress().value),
                        cmd));
              } catch (exception& e)
              {
                __map_trace2__("%s: %s ",__func__,e.what());
                SendStatusToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::THROTTLED),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
                return;
              }
            } else
            {
              // command taken from chain
              __map_warn__("putCommand: chained command for NIUSSD. Shoudn't happen.");
              SendStatusToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::SYSERR),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
              return;
            }
            if(isVlrUssdOp(serviceOp) && NetworkProfiles::getInstance().lookup(cmd->get_sms()->getDestinationAddress()).niVlrMethod==asmATI)
            {
              __map_trace2__("using ati for niussd from %s to %s",cmd->get_sms()->getOriginatingAddress().toString().c_str(),cmd->get_sms()->getDestinationAddress().toString().c_str());
              dialog->useAtiAfterSri=true;
            }else
            {
              dialog->useAtiAfterSri=false;
            }
            dialog->lastUssdMessage=serviceOp==USSD_USSR_REQ_LAST || serviceOp==USSD_USSN_REQ_LAST || serviceOp==USSD_USSR_REQ_VLR_LAST || serviceOp==USSD_USSN_REQ_VLR_LAST;
            dialog->dropChain = false;
            dialog->wasDelivered = false;
            dialog->hlrWasNotified = false;
            dialogid_map = dialog->dialogid_map;
            dialog->state = MAPST_START;
            __map_trace2__("%s: dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
            StartDialogProcessing(dialog.get(),cmd);
            return;
          }
        } else
        {
          SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_FATAL,Status::USSDDLGNFOUND),true,cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
          __map_warn2__("putCommand: invaid USSD code 0x%x",serviceOp);
          return;
        }
      }else
        if ( !dialog2  )
        { // end of DELIVERY ussd operation
          // DELIVERY SMS no chained dialog
          try {
            dialog_ssn = SSN;
            if ( cmd->get_commandId() == DELIVERY )
            {
              dialog.assign(MapDialogContainer::getInstance()->
                  createOrAttachSMSCDialog(
                      dialogid_smsc,
                      SSN,
                      string(cmd->get_sms()->getDestinationAddress().value),
                      cmd));
              if(!dialog.isnull())
              {
                rinst=dialog->instanceId;
              }
            } else
            {
              // QUERYABONENTSTATUS
              AbonentStatusMethod asMethod=asmSRI4SM;
              AbonentStatus& as=cmd->get_abonentStatus();
              if(as.srm==AbonentStatus::srmATI)
              {
                asMethod=asmATI;
              }else if(as.srm==AbonentStatus::srmSRI4SM)
              {
                asMethod=asmSRI4SM;
              }else //default
              {
                asMethod=NetworkProfiles::getInstance().lookup(as.addr).asMethod;
              }

              if(asMethod==asmATI)
              {
                dialog.assign(MapDialogContainer::getInstance()->createAbonentStatusDialog(USSD_SSN,cmd));
                dialog->isAtiDialog=true;
              }else
              {
                dialog.assign(MapDialogContainer::getInstance()->createAbonentStatusDialog(SSN,cmd));
                dialog->isAtiDialog=false;
              }
            }
          } catch (ChainIsVeryLong& e) {
            __map_trace2__("%s: %s ",__func__,e.what());
            SendStatusToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MSGQFUL),false,0);
            //throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
            return;
          } catch (NextMMSPartWaiting& e) {
            __map_trace2__("%s: %s ",__func__,e.what());
            SendStatusToSmsc(cmd->get_dialogId(),MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,Status::SUBSCRBUSYMT),false,0);
            return;
          } catch (exception& e) {
            __map_trace2__("%s: %s ",__func__,e.what());
            if(cmd->get_commandId()==QUERYABONENTSTATUS)
            {
              SmscCommand rsp = SmscCommand::makeQueryAbonentStatusResp(cmd->get_abonentStatus(),smsc::smeman::AbonentStatus::UNKNOWNVALUE,smsc::system::Status::THROTTLED,"","");
              MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(rsp);
            }else
            {
              SendStatusToSmsc(dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::THROTTLED),false,0);
            }
            return;//throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
          }
          if ( dialog.isnull() ) {
            //throw MAPDIALOG_TEMP_ERROR("Can't create or attach dialog");
            //SendRescheduleToSmsc(dialogid_smsc);
            __map_trace2__("%s: was scheduled (state:NONE) dialogid_smsc 0x%x",__func__, dialogid_smsc);
            return;
          }
        }else
        {
          // DELIVERY SMS chained dialog
          dialog_ssn = SSN;
          dialog.assign(dialog2->AddRef());
          dialogid_map = dialog->dialogid_map;
          dialogid_map = MapDialogContainer::getInstance()->reAssignDialog(dialog->instanceId,dialog->dialogid_map,dialog->ssn,SSN,radtOut);
          __require__(dialog->ssn == dialog_ssn);
          dialog->id_opened = false;
          dialog->dialogid_smsc = dialogid_smsc;
          dialog->dropChain = false;
          rinst=dialog->instanceId;
        }
      if ( dialog.isnull() )
      {
        __map_trace__("putCommand: can't create SMSC->MS dialog (locked), request has bean attached");
        // command has bean attached by dialog container
      }else
      {
        dialogid_map = dialog->dialogid_map;
        if( dialog->state != MAPST_SendNextMMS )
        {
          dialog->state = MAPST_START;
          __map_trace2__("%s: dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
          StartDialogProcessing(dialog.get(),cmd);
        } else
        {
          dialog->AssignSms(cmd->get_sms_and_forget());
          SendSms(dialog.get());
        }
      }
    }else
    { // !delivery !query abonent status it's submit resp
      dialogid_map = dialogid_smsc&0xFFFF;
      dialog_ssn = (dialogid_smsc >> 16)&0xFF;
      rinst=(dialogid_smsc >>24)&0xff;
      if(rinst==0xff)
      {
        return;
      }
      if ( dialog2 )
      {
        throw runtime_error("MAP::putCommand can't chain MAPINPUT");
      }
      dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_map,dialog_ssn,rinst));
      if ( dialog.isnull() )
      {
        __map_warn2__("MAP::putCommand: dialog not found for submit resp x0%x",dialogid_smsc);
        return;
      }
      //dialog->state = MAPST_START;
      dialogid_smsc = 0;
      __require__(dialog->ssn == dialog_ssn);
      __map_trace2__("%s: submit resp dialogid 0x%x  (state %d)",__func__,dialog->dialogid_map,dialog->state);
      if ( dialog->state == MAPST_WaitSubmitCmdConf )
      {
        if ( dialog->isUSSD )
        {
          dialog->state = MAPST_USSDWaitResponce;
          if ( cmd->get_resp()->get_status() != 0 )
          {
            DoUSSRUserResponceError(cmd,dialog.get());
          } else
          {
            SmscCommand cmd_c;
            {
              MutexGuard mg(dialog->mutex);
              if( !dialog->chain.empty() )
              {
                cmd_c = dialog->chain.front();
                dialog->chain.pop_front();
                __map_trace2__("%s found chained USSD deliver for that dialog", __func__);
              }
            }
            if(cmd_c.IsOk())
            {
              MAPIO_PutCommand(cmd_c, 0);
            }
          }
        }
        else
        {
          ResponseMO(dialog.get(),cmd->get_resp()->get_status());
          CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
          DropMapDialog(dialog.get());
        }
      }else if(dialog->state == MAPST_WaitSubmitUSSDRequestConf)
      {
        if ( cmd->get_resp()->get_status() != 0 )
        {
          eraseUssdLock(dialog.get(), __func__);
          dialog->dropChain = true;
          CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
          DropMapDialog(dialog.get());
        } else {
          dialog->state = MAPST_ReadyNextUSSDCmd;
          SmscCommand cmd_c;
          {
            MutexGuard mg(dialog->mutex);
            if( !dialog->chain.empty() )
            {
              cmd_c = dialog->chain.front();
              dialog->chain.pop_front();
              __map_trace2__("%s found chained USSD deliver for that dialog", __func__);
            }
          }
          if(cmd_c.IsOk())
          {
            MAPIO_PutCommand(cmd_c, 0);
          }
        }
      }else if(dialog->state == MAPST_WaitSubmitUSSDRequestCloseConf)
      {
        eraseUssdLock(dialog.get(), __func__);
        DropMapDialog(dialog.get());
      }else if(dialog->state == MAPST_WaitSubmitUSSDNotifyConf)
      {
        if ( cmd->get_resp()->get_status() != 0 )
        {
          eraseUssdLock(dialog.get(), __func__);
          dialog->dropChain = true;
          CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
          DropMapDialog(dialog.get());
        } else
        {
          dialog->state = MAPST_ReadyNextUSSDCmd;
          SmscCommand cmd_c;
          {
            MutexGuard mg(dialog->mutex);
            if( !dialog->chain.empty() )
            {
              cmd_c = dialog->chain.front();
              dialog->chain.pop_front();
              __map_trace2__("%s found chained USSD deliver for that dialog", __func__);
            }
          }
          if(cmd_c.IsOk())
          {
            MAPIO_PutCommand(cmd_c, 0);
          }
        }
      }else if(dialog->state == MAPST_WaitSubmitUSSDNotifyCloseConf)
      {
        eraseUssdLock(dialog.get(), __func__);
        DropMapDialog(dialog.get());
      }else
      {
        throw MAPDIALOG_BAD_STATE(
          FormatText("putCommand: bad state %d, did 0x%x, SMSC.did 0x%x", dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
      }
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,dialog_ssn,rinst);
}

static inline
int DoProvErrorProcessing(ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( provErrCode_p != 0 )
  {
    if( Status::isErrorPermanent( Status::MAP_PROVIDER_ERR_BASE+*provErrCode_p ) )
    {
      return MAKE_ERRORCODE(CMD_ERR_FATAL,Status::MAP_PROVIDER_ERR_BASE+*provErrCode_p);
    } else {
      return MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAP_PROVIDER_ERR_BASE+*provErrCode_p);
    }
  }
  return 0;
}

static inline
int DoMAPErrorProcessor(
  UCHAR_T errorCode,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( errorCode != 0 )
  {
    if( Status::isErrorPermanent( Status::MAP_ERR_BASE+errorCode ) )
    {
      return MAKE_ERRORCODE(CMD_ERR_FATAL,Status::MAP_ERR_BASE+errorCode);
    } else {
      return MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAP_ERR_BASE+errorCode);
    }
  }
  return DoProvErrorProcessing(provErrCode_p);
}

//no longer needed
#if 0
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
    pair<I,I> range = x_map.equal_range(s_key);
    if ( range.first == range.second ) return ET96MAP_E_OK;
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

#endif


#ifdef EIN_HD
#define CALLBACK_DECL extern "C"
#else
#define CALLBACK_DECL extern "C"
#endif

CALLBACK_DECL
USHORT_T Et96MapOpenConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_HEREISNO_ID( FormatText("MAP::dialog 0x%x is not present",dialogid_map));
    }
    __require__(dialog->ssn == localSsn);
    if( respondingAddr_sp ) {
      char tbuf[256];
      SS7ToText( respondingAddr_sp, tbuf );
      __map_trace2__("%s: dialogid 0x%x (state %d) DELIVERY_SM %s resp from %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str(), tbuf);
    } else {
      __map_trace2__("%s: dialogid 0x%x (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    }
    /*  openResult */
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
      if( openResult == ET96MAP_RESULT_NOT_OK)
      {
        int err=DoProvErrorProcessing(provErrCode_p);
        if(err)
        {
          if(GET_STATUS_TYPE(err)==CMD_ERR_FATAL)
          {
            throw MAPDIALOG_FATAL_ERROR(
                FormatText("MAP::%s connection opening error, reason %d",__func__,GET_STATUS_CODE(err)),
                GET_STATUS_CODE(err));
          }else
          {
            throw MAPDIALOG_TEMP_ERROR(
                FormatText("MAP::%s connection opening error, reason %d",__func__,GET_STATUS_CODE(err)),
                GET_STATUS_CODE(err));
          }
        }
        throw MAPDIALOG_FATAL_ERROR(
          FormatText("MAP::%s connection opening error, reason %d",__func__,refuseReason_p?*refuseReason_p:0),
          refuseReason_p?(Status::MAP_REFUSE_REASON_BASE+*refuseReason_p):Status::MAP_REFUSE_REASON_NO_REASON);
      }
      dialog->state = MAPST_WaitFwdMOConf;
      break;
    case MAPST_WaitUSSDReqOpenConf:
    case MAPST_WaitUSSDNotifyOpenConf:
      if( openResult == ET96MAP_RESULT_NOT_OK)
      {
        eraseUssdLock(dialog.get(), __func__);
        int err=DoProvErrorProcessing(provErrCode_p);
        if(err)
        {
          if(GET_STATUS_TYPE(err)==CMD_ERR_FATAL)
          {
            throw MAPDIALOG_FATAL_ERROR(
                FormatText("MAP::%s connection opening error, reason %d",__func__,GET_STATUS_CODE(err)),
                GET_STATUS_CODE(err));
          }else
          {
            throw MAPDIALOG_TEMP_ERROR(
                FormatText("MAP::%s connection opening error, reason %d",__func__,GET_STATUS_CODE(err)),
                GET_STATUS_CODE(err));
          }
        }
        throw MAPDIALOG_FATAL_ERROR(
          FormatText("MAP::%s connection opening error, reason %d",__func__,refuseReason_p?*refuseReason_p:0),
          refuseReason_p?(Status::MAP_REFUSE_REASON_BASE+*refuseReason_p):Status::MAP_REFUSE_REASON_NO_REASON);
      }
      break;
    case MAPST_RInfoFallBack:
    case MAPST_WaitSpecOpenConf:
    case MAPST_WaitOpenConf:
    case MAPST_ImsiWaitOpenConf:
    case MAPST_WaitUssdAtiOpenConf:
    case MAPST_WaitNIUssdAtiOpenConf:
      if ( openResult == ET96MAP_RESULT_NOT_OK )
      {
        if ( refuseReason_p && *refuseReason_p == ET96MAP_APP_CONTEXT_NOT_SUPP )
        {
          __map_trace2__("%s: context unsupported , version %d",__func__,dialog->version);
          if ( dialog->version != 1 )
          {
            --dialog->version;
            dialogid_map = RemapDialog(dialog.get(),localSsn,dialog->state==MAPST_RInfoFallBack || dialog->state==MAPST_ImsiWaitOpenConf?radtOutSRI:radtOut);
            switch ( dialog->state ) {
            case MAPST_RInfoFallBack:
            case MAPST_ImsiWaitOpenConf:
              SendRInfo(dialog.get());
              break;
            case MAPST_WaitSpecOpenConf:
            case MAPST_WaitOpenConf:
              dialog->mms = FALSE;
              SendSms(dialog.get());
              break;
            }
            break;
          }else
          {
            //dialog->state = MAPST_BROKEN;
            throw MAPDIALOG_FATAL_ERROR(
              FormatText("MAP::%s version alredy 1 and unsupported!",__func__),
              Status::MAP_REFUSE_REASON_BASE+*refuseReason_p);
          }
        }else
        {
          if(provErrCode_p && (Status::MAP_PROVIDER_REASON_BASE+*provErrCode_p)==Status::MAP_DIALOGUE_TIMEOUT)
          {
            dialog->noRespFromPeer=true;
          }
          int err=DoProvErrorProcessing(provErrCode_p);
          if(err)
          {
            if(GET_STATUS_TYPE(err)==CMD_ERR_FATAL)
            {
              throw MAPDIALOG_FATAL_ERROR(
                  FormatText("MAP::%s connection opening error, reason %d",__func__,GET_STATUS_CODE(err)),
                  GET_STATUS_CODE(err));
            }else
            {
              throw MAPDIALOG_TEMP_ERROR(
                  FormatText("MAP::%s connection opening error, reason %d",__func__,GET_STATUS_CODE(err)),
                  GET_STATUS_CODE(err));
            }
          }
          throw MAPDIALOG_FATAL_ERROR(
            FormatText("MAP::%s connection opening error, reason %d",__func__,refuseReason_p?*refuseReason_p:0),
            refuseReason_p?(Status::MAP_REFUSE_REASON_BASE+*refuseReason_p):Status::MAP_REFUSE_REASON_NO_REASON);
        }
      }
      /*     */
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
      case MAPST_WaitUssdAtiOpenConf:
        dialog->state = MAPST_WaitUssdAtiConf;
        break;
      case MAPST_WaitNIUssdAtiOpenConf:
        dialog->state = MAPST_WaitNIUssdAtiConf;
        break;
      }
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d",__func__,dialog->state));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

static USHORT_T  Et96MapVxSendRInfoForSmConf_Impl(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  EINSS7INSTANCE_T rinst,
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,rinst));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn == localSsn);
    dialogid_smsc = dialog->dialogid_smsc;

    dialog->routeErr = 0;

    if ( errorSendRoutingInfoForSm_sp )
    {
      if(errorSendRoutingInfoForSm_sp->errorCode == 27 )
      {
        dialog->subscriberAbsent = true;
        if( version == 1 && mwdSet && *mwdSet == ET96MAP_ADDRESS_ALREADY_IN_FILE )
        {
          dialog->mwdStatus.mnrf = 1;
        }
        dialog->routeErr = MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAP_ERR_BASE+27);
      } else
      {
        dialog->routeErr = DoMAPErrorProcessor(errorSendRoutingInfoForSm_sp->errorCode,0);
      }
    } else if( provErrCode_p )
    {
      dialog->routeErr = DoMAPErrorProcessor(0,provErrCode_p);
    }
    __map_trace2__("%s: dialogid 0x%x (state %d) %s, err=%d",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str(),GET_STATUS_CODE(dialog->routeErr));
    switch( dialog->state ){
    case MAPST_WaitRInfoConf:
      if ( dialog->routeErr ) {
        dialog->s_imsi = "";
        dialog->s_msc = "";
        dialog->state = MAPST_WaitRInfoClose;
        break;
      }
    case MAPST_ImsiWaitRInfo:
      {
        if ( dialog->routeErr ) {
          if(errorSendRoutingInfoForSm_sp &&
             (
               errorSendRoutingInfoForSm_sp->errorCode == 13 ||  // call barred
               errorSendRoutingInfoForSm_sp->errorCode == 11     // teleservice not provisioned
             ) &&
             MapDialogContainer::getAllowCallBarred() == true &&
             dialog->state == MAPST_ImsiWaitRInfo) {
            // normal situation no error
            // smsc will accept message from abonent
            __map_trace2__("%s: call barred but allowed", __func__);
            if(errorSendRoutingInfoForSm_sp->errorCode == 13)
            {
              dialog->callbarred=true;
            }
            if(errorSendRoutingInfoForSm_sp->errorCode == 11)
            {
              dialog->teleservicenotprov=true;
            }
            dialog->s_imsi = "";
            dialog->s_msc = "";
            dialog->routeErr = 0;
            dialog->state = MAPST_ImsiWaitCloseInd;
          } else {
            dialog->callbarred=false;
            dialog->teleservicenotprov=false;
            dialog->s_imsi = "";
            dialog->s_msc = "";
            dialog->state = MAPST_ImsiWaitCloseInd;
          }
          break;
        }
        else {
          // extract msc number
          if( version == 1 ) {
            if( locationInfo_sp != 0 ) {
              dialog->s_msc = LocationInfoToString(locationInfo_sp).c_str();
              __map_trace2__( "LocationInfo addr type: %s address: %s", locationInfo_sp->typeOfNumber==0x01?"roaming":"msc", dialog->s_msc.c_str() );
              mkSS7GTAddress( &dialog->destMscAddr, locationInfo_sp, 8 );
            } else {
              dialog->s_msc = "";
            }
          } else {
            if( mscNumber_sp != 0 ) {
              dialog->s_msc = MscToString(mscNumber_sp).c_str();
              mkSS7GTAddress( &dialog->destMscAddr, mscNumber_sp, 8 );
            } else {
              dialog->s_msc = "";
            }
          }
          if( imsi_sp != 0 ) {
            dialog->s_imsi = ImsiToString(imsi_sp).c_str();
            dialog->smRpDa.typeOfAddress = ET96MAP_ADDRTYPE_IMSI;
            dialog->smRpDa.addrLen = imsi_sp->imsiLen;
            memcpy( dialog->smRpDa.addr, imsi_sp->imsi, imsi_sp->imsiLen );
          } else {
            dialog->s_imsi = "";
          }
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
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV1SendRInfoForSmConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
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
    localSsn,dialogueId,INSTARG0(rinst),invokeId,imsi_sp,0,lmsi_sp,errorSendRoutingInfoForSm_sp,provErrCode_p,
    1,mwdSet,locationInfo_sp);
}

CALLBACK_DECL
USHORT_T  Et96MapV2SendRInfoForSmConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T *imsi_sp,
  ET96MAP_ADDRESS_T *mscNumber_sp,
  ET96MAP_LMSI_T *lmsi_sp,
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  return Et96MapVxSendRInfoForSmConf_Impl(
    localSsn,dialogueId,INSTARG0(rinst),invokeId,imsi_sp,mscNumber_sp,lmsi_sp,errorSendRoutingInfoForSm_sp,provErrCode_p,
    2,0,0);
}

CALLBACK_DECL
USHORT_T Et96MapCloseInd(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_USERDATA_T *specificInfo_sp,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,INSTARG0(rinst)));
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
    case MAPST_WaitNIUssdAtiClose:
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
        if ( !dialog->routeErr )
        {
          if( dialog->sms.get()->hasIntProperty( Tag::SMPP_USSD_SERVICE_OP ) )
          {
            if(dialog->useAtiAfterSri && dialog->state!=MAPST_WaitNIUssdAtiClose)
            {
              MapDialogContainer::getInstance()->reAssignDialog(dialog->instanceId,dialogueId,dialog->ssn,USSD_SSN,radtOutSRI); // This is for network initiated sessions
              mkMapAddress( &dialog->m_scAddr, USSD_ADDRESS().c_str(), (unsigned)USSD_ADDRESS().length() );
              mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, USSD_SSN );
              mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, HLR_SSN );
              makeAtiRequest(dialog.get());
              break;
            }
            MapDialogContainer::getInstance()->reAssignDialog(dialog->instanceId,dialogueId,dialog->ssn,USSD_SSN,radtNIUSSD); // This is for network initiated sessions
            int serviceOp = dialog->sms.get()->getIntProperty( Tag::SMPP_USSD_SERVICE_OP );
            if( serviceOp == USSD_USSR_REQ || serviceOp == USSD_USSN_REQ ||
                serviceOp == USSD_USSR_REQ_LAST || serviceOp == USSD_USSN_REQ_LAST ||
                serviceOp == USSD_USSR_REQ_VLR || serviceOp == USSD_USSR_REQ_VLR_LAST ||
                serviceOp == USSD_USSN_REQ_VLR || serviceOp == USSD_USSN_REQ_VLR_LAST)
            {
              DoUSSDRequestOrNotifyReq(dialog.get());
            } else
            {
              throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::USSD_SERVICE_OP=%d not allowed to create dialog",serviceOp),Status::INVOPTPARAMVAL);
            }
          } else
          {
            MapDialogContainer::getInstance()->reAssignDialog(dialog->instanceId,dialogueId,dialog->ssn,localSsn,radtOut);
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
          SendErrToSmsc(dialog.get(),dialog->routeErr);
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
      dialog->state = MAPST_CLOSED;
      SendSubmitCommand(dialog.get());
      CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
      eraseUssdLock(dialog.get(), __func__);
      DropMapDialog(dialog.get());
      break;
    case MAPST_WaitUSSDReqDelim:
      dialog->state = MAPST_WaitSubmitUSSDRequestCloseConf;
      SendSubmitCommand(dialog.get());
      break;
    case MAPST_WaitUssdAtiClose:
    case MAPST_ImsiWaitCloseInd:
      if(!dialog->isQueryAbonentStatus)
      {
        dialog->associate->hlrVersion = dialog->hlrVersion;
        ContinueImsiReq(dialog->associate,dialog->s_imsi,dialog->s_msc,dialog->routeErr);
      }else
      {
        SendAbonentStatusToSmsc(dialog.get(),dialog->QueryAbonentCommand->get_abonentStatus().status);
      }
      dialog->state = MAPST_END;
      DropMapDialog(dialog.get());
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__func__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapUAbortInd (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_USER_REASON_T *userReason_p,
  ET96MAP_DIAGNOSTIC_INFO_T *diagInfo_p,
  ET96MAP_USERDATA_T *specificInfo_sp,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
    {
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    }
    dialogid_smsc = dialog->dialogid_smsc;
    dialog->id_opened = false;
    __map_trace2__("%s: dialogid 0x%x userReason 0x%x",__func__,dialogid_map,userReason_p?*userReason_p:-1);
    int err=MAKE_COMMAND_STATUS(CMD_ERR_TEMP,Status::MAP_USER_REASON_BASE+(userReason_p?*userReason_p:-1));
    TryDestroyDialog(dialogid_map,true,err,localSsn,INSTARG0(rinst));
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapPAbortInd(
  ET96MAP_LOCAL_SSN_T localSsn INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_PROV_REASON_T provReason,
  ET96MAP_SOURCE_T source,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  __map_trace2__("%s: dialogid 0x%x source=%d provReason=0x%x",__func__,dialogueId,source,provReason);
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() ) {
      __map_trace2__("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId);
    }else{
      __require__(dialog->ssn==localSsn);
      dialogid_smsc = dialog->dialogid_smsc;
      dialog->id_opened = false;
      if( dialog->isUSSD )
      {
        if(*dialog->sms->getSourceSmeId() && Smsc::getInstance().getSmeInfo(dialog->sms->getSourceSmeId()).hasFlag(sfSmppPlus))
        {
          SMS& orgsms=*dialog->sms.get();
          SMS sms=orgsms;
          Descriptor desc;
          desc.setImsi((uint8_t)dialog->s_imsi.length(),dialog->s_imsi.c_str());
          desc.setMsc((uint8_t)dialog->s_msc.length(),dialog->s_msc.c_str());
          sms.setOriginatingDescriptor(desc);
          sms.setOriginatingAddress(orgsms.getDestinationAddress());
          sms.setDestinationAddress(orgsms.getOriginatingAddress());
          sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_USSREL_IND);
          sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
          sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
          sms.setMessageReference(0);
          sms.setStrProperty(Tag::SMSC_SCCP_OA,dialog->origAddress.c_str());
          sms.setStrProperty(Tag::SMSC_SCCP_DA,dialog->destAddress.c_str());
          if(sms.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
          {
            sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,"",0);
            sms.setIntProperty(Tag::SMPP_SM_LENGTH,0);
          }else
          {
            sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,"",0);
          }

          MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
          uint32_t dialogid2_smsc=(0xff<<24)|(((unsigned)dialog->ssn)<<16)|dialog->dialogid_map;
          SmscCommand cmd = SmscCommand::makeSumbmitSm(sms,dialogid2_smsc);
          proxy->putIncomingCommand(cmd);
        }
        eraseUssdLock(dialog.get(), __func__);
      }
      __map_warn2__("PABORT: dlg=0x%x %s %s",dialogid_map,dialog->isUSSD?"ussd":"sms",RouteToString(dialog.get()).c_str());
      TryDestroyDialog(dialogid_map,true,MAKE_ERRORCODE(Status::isErrorPermanent( Status::MAP_PROVIDER_REASON_BASE+provReason )?CMD_ERR_FATAL:CMD_ERR_TEMP,Status::MAP_PROVIDER_REASON_BASE+provReason),localSsn,INSTARG0(rinst));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapOpenInd (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
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
  DialogRefGuard dialog;
  try{
    __map_trace2__("%s: dialog 0x%x ctx=%d ver=%d dstref=%p orgref=%p",__func__,dialogueId,appContext_sp->acType,appContext_sp->version,destRef_sp,origRef_sp );
    dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));//,appContext_sp->version));
    if( appContext_sp->version == 1 && appContext_sp->acType == ET96MAP_NETWORK_FUNCTIONAL_SS_CONTEXT && !MapDialogContainer::getUssdV1Enabled())
    {
      // reject USSD request version 1
      __map_trace2__("%s: aborting USSD v1 dialog 0x%x", __func__, dialogueId );
      warnMapReq( Et96MapUAbortReq( localSsn INSTDLGARG(dialog), dialogueId, 0, 0, 0, 0 ), __func__);
      DropMapDialog(dialog.get());
      return ET96MAP_E_OK;
    }
    __require__(dialog->ssn==localSsn);
    dialog->hasIndAddress = false;
    if ( specificInfo_sp!=0 && specificInfo_sp->specificInfoLen >= 3 )
    {
      unsigned char* ptr=specificInfo_sp->specificData;
      unsigned len=specificInfo_sp->specificInfoLen;
      while(len>3)
      {
        if ( ptr[0] == 0x82 )
        {
          unsigned addrLen = ptr[1];
          if ( (((unsigned)ptr[addrLen+1] >> 4)&0x0f == 0xf ) )
            addrLen = (addrLen-1)*2-1;
          else
            addrLen = (addrLen-1)*2;
          memcpy(&dialog->m_msAddr,ptr+1,ptr[1]+1);
          dialog->m_msAddr.addressLength = addrLen;
          dialog->hasIndAddress = true;
        }
        if(ptr[0]==0x83)
        {
          ET96MAP_ADDRESS_T msc;
          Address addr;
          unsigned addrLen = ptr[1];
          if ( (((unsigned)ptr[addrLen+1] >> 4)&0x0f == 0xf ) )
            addrLen = (addrLen-1)*2-1;
          else
            addrLen = (addrLen-1)*2;
          memcpy(&msc,ptr+1,ptr[1]+1);
          msc.addressLength = addrLen;
          ConvAddrMSISDN2Smc(&msc,&addr);
          dialog->s_msc=addr.toString().c_str();
        }
        len-=ptr[1]+2;
        ptr+=ptr[1]+2;
      }
    }
#ifdef MAP_R12
    if(destRef_sp)
    {
      ConvAddrIMSIorMSISDN2String(destRef_sp,dialog->s_imsi);
    }
#else
    if(destRef_sp)
    {
      Address imsi;
      ConvAddrIMSI2Smc(destRef_sp,&imsi);
      dialog->s_imsi=imsi.value;
    }
#endif
    dialog->origAddress=SS7AddressToString(ss7OrigAddr_sp).c_str();
    dialog->destAddress=SS7AddressToString(ss7DestAddr_sp).c_str();
    dialog->state = MAPST_WaitSms;
  }
  catch(exception& e)
  {
    __map_warn2__("%s: dialogid 0x%x <exception>:%s",__func__,dialogueId,e.what());
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    if(!dialog.isnull())
    {
      MapDialogContainer::getInstance()->dropDialog(dialog.get());
    }
    warnMapReq( Et96MapOpenResp(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0), __func__);
    warnMapReq( Et96MapCloseReq(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
  }
  return ET96MAP_E_OK;
}

static USHORT_T Et96MapVxForwardSmMOInd_Impl (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  EINSS7INSTANCE_T rinst,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp,
  unsigned version)
{
  bool open_confirmed = false;
  try{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,rinst));
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
      DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,rinst));
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
        __require__(dialog->ssn==localSsn);
      }
      checkMapReq( Et96MapOpenResp(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0), __func__);
    }
    ET96MAP_ERROR_FORW_SM_MO_T moResp;
    memset( &moResp, 0, sizeof(ET96MAP_ERROR_FORW_SM_MO_T) );
    moResp.errorCode = ET96MAP_UE_SYS_FAILURE;
    if( version == 3 ) {
        ET96MAP_SM_RP_UI_T ui;
        ui.signalInfoLen=0;
        checkMapReq( Et96MapV3ForwardSmMOResp( localSsn INSTARG(rinst), dialogueId, invokeId, &ui, &moResp), __func__);
    } else if( version == 2 ) {
        checkMapReq( Et96MapV2ForwardSmMOResp( localSsn INSTARG(rinst), dialogueId, invokeId, &moResp), __func__);
    }else if ( version == 1 ) {
        checkMapReq( Et96MapV1ForwardSmMOResp( localSsn INSTARG(rinst), dialogueId, invokeId, &moResp), __func__);
    }
  }
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV1ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp)
{
  return Et96MapVxForwardSmMOInd_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,smRpDa_sp,smRpOa_sp,smRpUi_sp,1);
}

CALLBACK_DECL
USHORT_T Et96MapV2ForwardSmMOConf(ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
                                ET96MAP_DIALOGUE_ID_T dialogueId,
                                ET96MAP_INVOKE_ID_T invokeId,
                                ET96MAP_ERROR_FORW_SM_MO_T* errorForwardSMmo_sp,
                                ET96MAP_PROV_ERR_T* provErrCode_p) {
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x  (state %d) forward %s code: %d, provider: %d",__func__,dialog->dialogid_map,dialog->state,
                   RouteToString(dialog.get()).c_str(), (int)(errorForwardSMmo_sp?errorForwardSMmo_sp->errorCode:0),
                   (int)(provErrCode_p?*provErrCode_p:0));

    int err=DoMAPErrorProcessor( errorForwardSMmo_sp?errorForwardSMmo_sp->errorCode:0, provErrCode_p );
    if(err)
    {
      SendErrToSmsc(dialog.get(),err);
      if( GET_STATUS_CODE(err) == Status::MAP_NO_RESPONSE_FROM_PEER )
      {
        dialog->dropChain = true;
      }
      dialog->dialogid_smsc = 0; //
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
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}


CALLBACK_DECL
USHORT_T Et96MapV2ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp)
{
  return Et96MapVxForwardSmMOInd_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,smRpDa_sp,smRpOa_sp,smRpUi_sp,2);
}

CALLBACK_DECL
USHORT_T Et96MapV3ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp,
  ET96MAP_IMSI_T     *imsi_sp)
{
  return Et96MapVxForwardSmMOInd_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,smRpDa_sp,smRpOa_sp,smRpUi_sp,3);
}

static USHORT_T Et96MapVxForwardSmMTConf_Impl (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  EINSS7INSTANCE_T rinst,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p,
  unsigned version)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,rinst));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("%s: dialog 0x%x is not present",__func__,_di));
    }
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x  (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state, RouteToString(dialog.get()).c_str());

    int err=DoMAPErrorProcessor( errorForwardSMmt_sp?errorForwardSMmt_sp->errorCode:0, provErrCode_p );

    if ( provErrCode_p && *provErrCode_p == ET96MAP_NO_RESPONSE_FROM_PEER )
    {
      dialog->noRespFromPeer=true;
    }
    if ( errorForwardSMmt_sp )
    {
      if ( errorForwardSMmt_sp->errorCode == 27 )
      {
        dialog->subscriberAbsent = true;
      }
      if ( errorForwardSMmt_sp->errorCode == 32 ) /*delivery error*/
      {
        if ( errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == FAILURE_MT_NS::ET96MAP_MEM_CAPACITY_EXCEEDED )
        {
          dialog->memoryExceeded = true;
          err=MAKE_COMMAND_STATUS(CMD_ERR_TEMP,Status::MSMEMCAPACITYFULL);
        }
        /*
        else if ( errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == FAILURE_MT_NS::ET96MAP_PROTOCOL_ERROR
            || errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason == FAILURE_MT_NS::ET96MAP_MO_SERVICE_CENTER_CONGESTION )
        {
          __map_trace2__("%s: Delivery failure reason 0x%x",__func__,errorForwardSMmt_sp->u.smDeliveryFailureReason_s.reason);
        }
        */
      }
    }
    if(err)
    {
      __map_trace2__("%s: dlgId=%x, err=%d", __func__,dialog->dialogid_map,GET_STATUS_CODE(err));
      SendErrToSmsc(dialog.get(),err);
      dialog->dialogid_smsc = 0; //
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
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV1ForwardSmMT_MOConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return Et96MapVxForwardSmMTConf_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,errorForwardSMmt_sp,provErrCode_p,1);
}

CALLBACK_DECL
USHORT_T Et96MapV2ForwardSmMTConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return Et96MapVxForwardSmMTConf_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,errorForwardSMmt_sp,provErrCode_p,2);
}

CALLBACK_DECL
USHORT_T Et96MapV3ForwardSmMTConf(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_UI_T *smRpUi_sp,
  ET96MAP_EXTENSIONDATA_T *extension_sp,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return Et96MapVxForwardSmMTConf_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,errorForwardSMmt_sp,provErrCode_p,3);
}

CALLBACK_DECL
USHORT_T Et96MapDelimiterInd(
                             ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
                             ET96MAP_DIALOGUE_ID_T dialogueId,
                             UCHAR_T priorityOrder)
{
  bool open_confirmed = false;
  ET96MAP_REFUSE_REASON_T reason;
  try{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
                            FormatText("MAP::%s dialog 0x%x is not present",__func__,dialogueId));
    }
    __require__(dialog->ssn==localSsn);
    __map_trace2__("%s: dialogid 0x%x (state %d) %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    switch( dialog->state )
    {
      case MAPST_WaitSms:
      {
        dialog->state = MAPST_WaitSmsMOInd;
        reason = ET96MAP_NO_REASON;
        smsc::system::mapio::MapLimits& ml=smsc::system::mapio::MapLimits::getInstance();
        reason = ET96MAP_NO_REASON;
        ET96MAP_SS7_ADDR_T* addrPtr=0;
        if(ml.isSmsOpenRespRealAddr())
        {
          addrPtr=GetScAddr();
          char addrBuf[32];
          SS7ToText(addrPtr,addrBuf);
          __map_trace2__("using real resp addr for openresp:%s",dialog->subsystem.c_str(),addrBuf);
        }
        checkMapReq( Et96MapOpenResp(dialog->ssn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_OK,&reason,addrPtr,0,0), __func__);
        checkMapReq( Et96MapDelimiterReq(dialog->ssn INSTDLGARG(dialog),dialogueId,0,0), __func__);
        open_confirmed = true;
      }break;
      case MAPST_WaitSmsMODelimiter2:
        reason = ET96MAP_NO_REASON;
        checkMapReq( Et96MapOpenResp(dialog->ssn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0), __func__);
        dialog->state = MAPST_WaitImsiReq;
        PauseOnImsiReq(dialog.get());
        break;
      case MAPST_WaitUssdDelimiter:
      {
        smsc::system::mapio::MapLimits& ml=smsc::system::mapio::MapLimits::getInstance();
        reason = ET96MAP_NO_REASON;
        ET96MAP_SS7_ADDR_T* addrPtr=0;
        if(ml.isOpenRespRealAddr(dialog->subsystem))
        {
          addrPtr=GetUSSDAddr();
          char addrBuf[32];
          SS7ToText(addrPtr,addrBuf);
          __map_trace2__("using real resp addr for ussd code %s:%s",dialog->subsystem.c_str(),addrBuf);
        }
        checkMapReq( Et96MapOpenResp(dialog->ssn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_OK,&reason,addrPtr,0,0), __func__);
        __map_trace2__("subsystem=%s",dialog->subsystem.c_str());
        if(ml.isNoSRIUssd(dialog->subsystem) ||
            ((!dialog->s_imsi.empty() && !dialog->s_msc.empty()) && ml.isCondSRIUssd(dialog->subsystem))
          )
        {
          dialog->noSri=true;
          dialog->state = MAPST_WaitSubmitCmdConf;
          SendSubmitCommand(dialog.get());
        }else if(ml.isATIUssd(dialog->subsystem))
        {
          PauseOnAtiReq(dialog.get());
        }else
        {
          //dialog->state = MAPST_WaitUssdImsiReq;
          PauseOnImsiReq(dialog.get());
        }
      }break;
      case MAPST_WaitUssdV1Delimiter:
        reason = ET96MAP_NO_REASON;
        checkMapReq( Et96MapOpenResp(dialog->ssn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0), __func__);
        if( MapDialogContainer::getUssdV1UseOrigEntityNumber() ) {
          dialog->state = MAPST_WaitUssdImsiReq;
          PauseOnImsiReq(dialog.get());
        } else {
          dialog->state = MAPST_WaitSubmitCmdConf;
          SendSubmitCommand(dialog.get());
        }
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
        CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
        DropMapDialog(dialog.get());
        break;
      case MAPST_WaitUSSDReqDelim:
        dialog->state = MAPST_WaitSubmitUSSDRequestConf;
        SendSubmitCommand(dialog.get());
        if(dialog->lastUssdMessage)
        {
          CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
          eraseUssdLock(dialog.get(), __func__);
          DropMapDialog(dialog.get());
        }
        break;
      case MAPST_WaitUSSDReqClose:
        CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
        eraseUssdLock(dialog.get(), __func__);
        DropMapDialog(dialog.get());
        break;
      case MAPST_WaitUSSDNotifyClose:
        dialog->state = MAPST_WaitSubmitUSSDNotifyConf;
        if(dialog->lastUssdMessage)
        {
          CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
          eraseUssdLock(dialog.get(), __func__);
          DropMapDialog(dialog.get());
        }else
        {
          SendSubmitCommand(dialog.get());
        }
        break;
      case MAPST_WaitUSSDNotifyCloseErr:
        CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
        eraseUssdLock(dialog.get(), __func__);
        DropMapDialog(dialog.get());
        break;
      case MAPST_MapNoticed:
        reason = ET96MAP_NO_REASON;
        checkMapReq( Et96MapOpenResp(dialog->ssn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_OK,&reason,0,0,0), __func__);
        checkMapReq( Et96MapCloseReq (dialog->ssn INSTDLGARG(dialog),dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
        dialog->state = MAPST_END;
        DropMapDialog(dialog.get());
        break;
      case MAPST_ABORTED:
        checkMapReq( Et96MapCloseReq(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
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
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( !open_confirmed )
    {
      reason = ET96MAP_NO_REASON;
      //MapDialogContainer::getInstance()->dropDialog(dialogueId);
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
        __require__(dialog->ssn==localSsn);
        warnMapReq( Et96MapOpenResp(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0), __func__);
        warnMapReq( Et96MapCloseReq(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
        DropMapDialog(dialog.get());
      }
    } else {
      if(!dialog.isnull())
      {
        TryDestroyDialog(dialogueId,dialog->dialogid_smsc!=0,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE),localSsn,INSTARG0(rinst));
      }
    }
  }
  catch(...)
  {
    __map_trace2__("%s: dialogid 0x%x <exception>: ...",__func__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( !open_confirmed ){
      reason = ET96MAP_NO_REASON;
      //MapDialogContainer::getInstance()->dropDialog(dialogueId);
      if ( !dialog.isnull() ) {
        dialog->state = MAPST_ABORTED;
        __require__(dialog->ssn==localSsn);
        warnMapReq( Et96MapOpenResp(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0), __func__);
        warnMapReq( Et96MapCloseReq(localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
        DropMapDialog(dialog.get());
      }
    } else {
      if(!dialog.isnull())
      {
        TryDestroyDialog(dialogueId,dialog->dialogid_smsc!=0,MAKE_ERRORCODE(CMD_ERR_TEMP,Status::MAPINTERNALFAILURE),localSsn,INSTARG0(rinst));
      }
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

static void ContinueImsiReq(MapDialog* dialog,const String32& s_imsi,const String32& s_msc, const unsigned routeErr )
{
  using namespace smsc::system::mapio;
  __map_trace2__("%s: ismsi %s, msc: %s, state: %d, code: %d",__func__,s_imsi.c_str(),s_msc.c_str(), dialog->state, routeErr);
  static smsc::logger::Logger* fraudLog=smsc::logger::Logger::getInstance("map.fraud");
  if ( dialog->state == MAPST_END ){// already closed
    return;
  }
  if ( routeErr != 0 )
  {
    if( dialog->state == MAPST_WaitUssdImsiReq) {
      DoUSSRUserResponceError(SmscCommand(), dialog); // send system failure
    } else {
      ResponseMO(dialog,9);
      dialog->state = MAPST_WaitSubmitCmdConf;
      CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
      DropMapDialog(dialog);
    }
  }
  else
  {
    if(FraudControl::getInstance()->enabledCheck && dialog->sms.get())
    {
      if(s_msc.length() && dialog->origAddress.length() &&
         !dialog->isUSSD &&
         !FixedAddrCompare(dialog->origAddress,s_msc,FraudControl::getInstance()->ignoreTail))
      {
        if(!FraudControl::getInstance()->checkWhiteList(dialog->origAddress.c_str()))
        {
          smsc_log_warn(fraudLog,"FRAUD:dlgId=0x%x, ca=%s, msc=%s, oa=%s, da=%s",dialog->dialogid_map,dialog->origAddress.c_str(),s_msc.c_str(),
                        dialog->sms->getOriginatingAddress().toString().c_str(),
                        dialog->sms->getDestinationAddress().toString().c_str());
          if(FraudControl::getInstance()->enableReject)
          {
            smsc_log_info(fraudLog,"REJECTED:0x%x",dialog->dialogid_map);
            ResponseMO(dialog,9);
            dialog->state = MAPST_WaitSubmitCmdConf;
            CloseMapDialog(dialog->dialogid_map,dialog->ssn,dialog->instanceId);
            DropMapDialog(dialog);
            return;
          }
        }
      }
    }
    dialog->state = MAPST_WaitSubmitCmdConf;
    dialog->s_imsi = s_imsi;
    dialog->s_msc = s_msc;
    SendSubmitCommand(dialog);
  }
}

static void PauseOnAtiReq(MapDialog* map)
{
  unsigned localSsn = 0;
  DialogRefGuard dialog(MapDialogContainer::getInstance()->createDialogImsiReq(USSD_SSN,map));
  if (dialog.isnull()) throw runtime_error(
    FormatText("MAP::%s can't create dialog",__func__));
  unsigned dialogid_map = dialog->dialogid_map;
  localSsn = dialog->ssn;
  MAP_TRY{
    if ( map->sms.get() == 0 )
    {
      throw runtime_error(
        FormatText("MAP::%s has no SMS",__func__));
    }
    if ( !map->isUSSD )
    {
      mkMapAddress( &dialog->m_msAddr, map->sms->getOriginatingAddress() );
    }
    else
    {
      if (!map->hasIndAddress )
        throw runtime_error("MAP::%s MAP.did:{0x%x} has no originating address");
      dialog->m_msAddr = map->m_msAddr;
    }
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ USSD_ADDRESS().c_str(), (unsigned)USSD_ADDRESS().length() );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, HLR_SSN );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, USSD_SSN );
    makeAtiRequest(dialog.get());
  }MAP_CATCH(dialogid_map,0,localSsn,dialog->instanceId);
}

static void PauseOnImsiReq(MapDialog* map)
{
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
      mkMapAddress( &dialog->m_msAddr, map->sms->getOriginatingAddress() );
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
    mkMapAddress( &dialog->m_scAddr, /*"79029869999"*/ SC_ADDRESS().c_str(), (unsigned)SC_ADDRESS().length() );
    mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
    mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
//    __map_trace2__("MAP::%s: Query HLR AC version",__func__);
    //dialog->mshlrAddr = map->mshlrAddr;
    dialog->state = MAPST_ImsiWaitACVersion;
    QueryHlrVersion(dialog.get());
  }MAP_CATCH(dialogid_map,0,localSsn,dialog->instanceId);
}

static String32 GetUSSDSubsystem(
  const char* text,
  unsigned length)
{
  const char* p = text;
  const char* pEnd = text+length;
  for ( ; p < pEnd; ++p ) if ( (*p != '#') && (*p != '*') ) break;
  const char* sBegin = p;
  for ( ; p < pEnd; ++p ) if ( (*p == '#') || (*p == '*') ) break;
  const char* sEnd = p;
  String32 rv;
  rv.assign(sBegin,sEnd-sBegin);
  return rv;
}

static char getNextUcs2Char(const char* ptr)
{
  short rv;
  memcpy(&rv,ptr,2);
  rv=htons(rv);
  return (char)rv;
}

static String32 GetUSSDSubsystemUCS2(
  const char* text,
  unsigned length)
{
  const char* p = text;
  const char* pEnd = text+length;
  for ( ; p < pEnd; p+=2 )
  {
    char c=getNextUcs2Char(p);
    if ( (c != '#') && (c != '*') )
    {
      break;
    }
  }
  const char* sBegin = p;
  for ( ; p < pEnd; p+=2 )
  {
    char c=getNextUcs2Char(p);
    if ( (c == '#') || (c == '*') )
    {
      break;
    }
  }
  const char* sEnd = p;
  String32 rv;
  p=sBegin;
  unsigned i=0;
  for(;p<=sEnd && i<31;p+=2,i++)
  {
    rv[i]=getNextUcs2Char(p);
  }
  rv[i]=0;
  return rv;
}

static unsigned GetUSSDRequestStringUCS2(
  const char* text,
  unsigned length,
  char* out,
  UssdParsingMode upm)
{
  if(upm==upmNever || (upm==upmOnlyStar && getNextUcs2Char(text)=='#'))
  {
    memcpy(out,text,length);
    return length;
  }
  const char* p = text;
  const char* pEnd = text+length-2;
  for ( ; p < pEnd; pEnd-=2 )
  {
    char c=getNextUcs2Char(pEnd);
    if ( (c == '#') || (c == '*') )
    {
      break;
    }
  }
  if( pEnd == p ) throw runtime_error("No trailing # or * found in USSD request string");
  for ( ; p < pEnd; p+=2 )
  {
    char c=getNextUcs2Char(p);
    if ( (c != '#') && (c != '*') )
    {
      break;
    }
  }
  for ( ; p < pEnd; p+=2 )
  {
    char c=getNextUcs2Char(p);
    if ( (c == '#') || (c == '*') )
    {
      break;
    }
  }
  if( p == pEnd )
  {
    return 0;
  }
  else
  {
    p+=2;
    length=(unsigned)(pEnd-p);
    memcpy(out,p,length);
    return length;
  }
}


static unsigned GetUSSDRequestString(
  const char* text,
  unsigned length,
  char* out,
  UssdParsingMode upm)
{
  if(upm==upmNever || (upm==upmOnlyStar && *text=='#'))
  {
    memcpy(out,text,length);
    return length;
  }
  const char* p = text;
  const char* pEnd = text+length-1;
  for ( ; p < pEnd; --pEnd ) if ( (*pEnd == '#') || (*pEnd == '*') ) break;
  if( pEnd == p ) throw runtime_error("No trailing # or * found in USSD request string");
  for ( ; p < pEnd; ++p ) if ( (*p != '#') && (*p != '*') ) break;
  const char* sBegin = p;
  for ( ; p < pEnd; ++p ) if ( (*p == '#') || (*p == '*') ) break;
  if( p == pEnd )
  {
    return 0;
  }
  else
  {
    p++;
    length=(unsigned)(pEnd-p);
    memcpy(out,p,length);
    return length;
  }
}

CALLBACK_DECL
USHORT_T Et96MapV2ProcessUnstructuredSSRequestInd(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_USSD_DATA_CODING_SCHEME_T ussdDataCodingScheme,
  ET96MAP_USSD_STRING_T ussdString_s,
  ET96MAP_ADDRESS_T *msisdn_sp)
{
  unsigned __dialogid_map = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
    {
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    }
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    dialog->ussdSequence=0;
    __dialogid_map = dialogueId;
    dialog->invokeId = invokeId;
    dialog->origInvokeId = invokeId;
    if( msisdn_sp != 0 )
    {
      dialog->m_msAddr.addressLength = msisdn_sp->addressLength;
      dialog->m_msAddr.typeOfAddress = msisdn_sp->typeOfAddress;
      memcpy(dialog->m_msAddr.address, msisdn_sp->address,ET96MAP_ADDRESS_LEN);
      dialog->hasIndAddress = true;
      __map_trace2__("MAP::%s has msisdn %d.%d", __func__,dialog->m_msAddr.addressLength, dialog->m_msAddr.typeOfAddress );
    }

    String32 subsystem;
    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    Address src_addr;
    ConvAddrMap2Smc((const MAP_SMS_ADDRESS*)&dialog->m_msAddr,&src_addr);
    sms.setOriginatingAddress(src_addr);

    sms.setStrProperty(Tag::SMSC_SCCP_OA,dialog->origAddress.c_str());
    sms.setStrProperty(Tag::SMSC_SCCP_DA,dialog->destAddress.c_str());

    //sms.setIntProperty(Tag::SMSC_ORIGINAL_DC, ussdDataCodingScheme );
    unsigned esm_class = 2; // Transaction mode
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);

    UCHAR_T udhPresent, msgClassMean, msgClass;
    unsigned dataCoding = (unsigned)convertCBSDatacoding2SMSC(ussdDataCodingScheme, &udhPresent, &msgClassMean, &msgClass);
    if( dataCoding == smsc::smpp::DataCoding::SMSC7BIT )
    {
      MicroString ms;
      char outStr[256];
      unsigned chars = ussdString_s.ussdStrLen*8/7;
      Convert7BitToSMSC7Bit(ussdString_s.ussdStr,chars/*ussdString_s.ussdStrLen*/,&ms,0);
      subsystem = GetUSSDSubsystem(ms.bytes,ms.len);
      unsigned outLen = GetUSSDRequestString(ms.bytes, ms.len,outStr,MapLimits::getInstance().getUssdParsing(subsystem.c_str()));
      sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,outStr,outLen);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,(uint32_t)outLen);
    } else if(dataCoding == smsc::smpp::DataCoding::UCS2)
    {
      char outStr[256];
      subsystem = GetUSSDSubsystemUCS2((char*)ussdString_s.ussdStr,ussdString_s.ussdStrLen);
      unsigned outLen = GetUSSDRequestStringUCS2((char*)ussdString_s.ussdStr,ussdString_s.ussdStrLen,outStr,MapLimits::getInstance().getUssdParsing(subsystem.c_str()));
      sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
      if(!HSNS_isEqual())
      {
        UCS_ntohs(outStr,outStr,outLen,0);
      }
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,outStr,outLen);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,(uint32_t)outLen);
    }else
    {
      __map_warn2__("DCS 0x%02X received in Et96MapV2ProcessUnstructuredSSRequestInd", ussdDataCodingScheme );
      throw runtime_error("Datacoding other then GSM7bit is not supported in Et96MapV2ProcessUnstructuredSSRequestInd");
    }
    dialog->ussdMrRef = MakeMrRef();
    __map_trace2__("%s: dialogid 0x%x invokeid=%d request encoding 0x%x length %d subsystem %s mr=%x",__func__,dialogueId,invokeId,ussdDataCodingScheme,ussdString_s.ussdStrLen,subsystem.c_str(),dialog->ussdMrRef);
    dialog->subsystem=".5.0.ussd:";
    dialog->subsystem += subsystem.c_str();
    Address dest_addr = Address(dialog->subsystem.c_str());
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(dest_addr);
    dialog->AssignSms(_sms.release());
    dialog->state = MAPST_WaitUssdDelimiter;
    //dialog->ussdSequence = NextSequence();
    dialog->sms->setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_IND);
    dialog->sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    dialog->invokeId = invokeId;
  }MAP_CATCH(__dialogid_map,0,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV2UnstructuredSSRequestConf(
    ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_USSD_DATA_CODING_SCHEME_T *ussdDataCodingScheme_p,
    ET96MAP_USSD_STRING_T *ussdString_sp,
    ET96MAP_ERROR_UNSTRUCTURED_SS_REQUEST_T *errorUnstructuredSSReq_sp,
    ET96MAP_PROV_ERR_T *provErrCode_p)
{
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    int err=DoMAPErrorProcessor(errorUnstructuredSSReq_sp?errorUnstructuredSSReq_sp->errorCode:0,provErrCode_p);
    if(err)
    {
      __map_trace2__( "%s: dlgId=%x, err=%d", __func__, dialog->dialogid_map,GET_STATUS_CODE(err) );
      SendErrToSmsc(dialog.get(),err);
      dialog->state = MAPST_WaitUSSDReqClose;
      return ET96MAP_E_OK;
    }
    //    USSDRequestReq
    SendOkToSmsc(dialog.get());

    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    Address originator = Address(dialog->subsystem.c_str());
    Address src_addr;
    ConvAddrMap2Smc((const MAP_SMS_ADDRESS*)&dialog->m_msAddr,&src_addr);
    sms.setOriginatingAddress(src_addr);
    sms.setStrProperty(Tag::SMSC_SCCP_OA,dialog->origAddress.c_str());
    sms.setStrProperty(Tag::SMSC_SCCP_DA,dialog->destAddress.c_str());

    UCHAR_T udhPresent, msgClassMean, msgClass;
    if( ussdDataCodingScheme_p && ussdString_sp && ussdString_sp->ussdStrLen)
    {
      //sms.setIntProperty(Tag::SMSC_ORIGINAL_DC, *ussdDataCodingScheme_p );
      unsigned dataCoding = (unsigned)convertCBSDatacoding2SMSC(*ussdDataCodingScheme_p, &udhPresent, &msgClassMean, &msgClass);
#ifdef USSDUCS2FIX
      if(dataCoding==smsc::smpp::DataCoding::UCS2)
      {
        uint8_t firstOctet=ussdString_sp->ussdStr[0];
        if((firstOctet&0xf0)!=0)
        {
          dataCoding=smsc::smpp::DataCoding::SMSC7BIT;
        }
      }
#endif
      if( dataCoding == smsc::smpp::DataCoding::SMSC7BIT )
      {
        MicroString ms;
        unsigned chars = ussdString_sp->ussdStrLen*8/7;
        Convert7BitToSMSC7Bit(ussdString_sp->ussdStr,chars,&ms,0);
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,ms.bytes,ms.len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
        sms.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
      } else
      {
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)(ussdString_sp->ussdStr),ussdString_sp->ussdStrLen);
        if(!HSNS_isEqual() && dataCoding==8)
        {
          unsigned msgLen;
          char* msg=(char*)sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&msgLen);
          UCS_ntohs(msg,msg,msgLen,0);
        }
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
    if(dialog->lastUssdMessage)
    {
      sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_USSR_CONF_LAST);
    }else
    {
      sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_USSR_CONF);
    }
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    dialog->AssignSms(_sms.release());
    dialog->state = MAPST_WaitUSSDReqDelim;
    dialog->invokeId = invokeId;
  }MAP_CATCH(dialogueId,0,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

//_CB(MapUssdNotifyConf)

CALLBACK_DECL
USHORT_T Et96MapV2UnstructuredSSNotifyConf(
    ET96MAP_LOCAL_SSN_T localSsn INSTANCEIDARGDEF(rinst),
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_ERROR_USSD_NOTIFY_T *errorUssdNotify_sp,
    ET96MAP_PROV_ERR_T *provErrCode_p)
{
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
      throw runtime_error(
        FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    __map_trace2__("%s: dialogid 0x%x notify confirmation.",__func__,dialogueId);
    if( dialog->state == MAPST_WaitUSSDErrorClose ) {
      dialog->state = MAPST_WaitUSSDNotifyCloseErr;
      return ET96MAP_E_OK;
    }
    int err=DoMAPErrorProcessor(errorUssdNotify_sp?errorUssdNotify_sp->errorCode:0,provErrCode_p);
    if(err)
    {
      __map_trace2__( "%s: dlgId=%x err=%d", __func__, dialog->dialogid_map,GET_STATUS_CODE(err));
      SendErrToSmsc(dialog.get(), err );
      dialog->state = MAPST_WaitUSSDNotifyCloseErr;
      return ET96MAP_E_OK;
    }
    //    USSDNotifyReq
    SendOkToSmsc(dialog.get());
    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    Address originator = Address(dialog->subsystem.c_str());
    Address src_addr;
    ConvAddrMap2Smc((const MAP_SMS_ADDRESS*)&dialog->m_msAddr,&src_addr);
    sms.setOriginatingAddress(src_addr);
    sms.setStrProperty(Tag::SMSC_SCCP_OA,dialog->origAddress.c_str());
    sms.setStrProperty(Tag::SMSC_SCCP_DA,dialog->destAddress.c_str());
    unsigned esm_class = 2; // Transaction mode
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(originator);
    sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_USSN_CONF);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    dialog->AssignSms(_sms.release());
    dialog->state = MAPST_WaitUSSDNotifyClose;
    dialog->invokeId = invokeId;
  }MAP_CATCH(dialogueId,0,localSsn,INSTARG0(rinst));
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
      delete [] text;
     }
    }

  mkMapAddress( &dialog->m_msAddr, addr );
  __map_trace2__("%s: addr len=%d val=%s", __func__, addr.length, addr.value );
  mkMapAddress( &dialog->m_scAddr, SC_ADDRESS().c_str(), (unsigned)SC_ADDRESS().length() );
  mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
  mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
  SmscCommand cmd = SmscCommand::makeHLRAlert(addr);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
}

static void ResponseAlertSC(MapDialog* dialog)
{
  ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
  checkMapReq( Et96MapOpenResp(dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,ET96MAP_RESULT_OK,&reason,0,0,0), __func__);
  if ( dialog->version == 2 ){
    checkMapReq( Et96MapV2AlertSCResp(dialog->ssn INSTDLGARG(dialog),dialog->dialogid_map,dialog->invokeId,0), __func__);
  }else if ( dialog->version == 1 ){
    // nothing
  }else throw runtime_error("MAP::ResponseAlertSC:unsupported dialog version");
}

CALLBACK_DECL
USHORT_T Et96MapVxAlertSCInd_Impl(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  EINSS7INSTANCE_T rinst,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ADDRESS_T *sCA_sp,
  unsigned version)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,rinst));
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
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,rinst);
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV2AlertSCInd(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ADDRESS_T *sCA_sp )
{
  return Et96MapVxAlertSCInd_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,msisdnAlert_sp,sCA_sp,2);
}

CALLBACK_DECL
USHORT_T Et96MapV1AlertSCInd(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ADDRESS_T *sCA_sp )
{
  return Et96MapVxAlertSCInd_Impl(localSsn,dialogueId,INSTARG0(rinst),invokeId,msisdnAlert_sp,sCA_sp,1);
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

CALLBACK_DECL
USHORT_T Et96MapV2InformSCInd (
  ET96MAP_LOCAL_SSN_T   localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T   invokeId,
  ET96MAP_ADDRESS_T     *msisdnAlert_sp,
  ET96MAP_MWD_STATUS_T  *mwdStatus_sp)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
    {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",_di));
    }
    __require__(dialog->ssn==localSsn);
    dialogid_smsc = dialog->dialogid_smsc;
    __map_trace2__("%s: dialogid 0x%x (state %d) DELIVERY_SM %s",__func__,dialog->dialogid_map,dialog->state,RouteToString(dialog.get()).c_str());
    if( msisdnAlert_sp && smsc::logger::_map_cat->isDebugEnabled() )
    {
       int len = (msisdnAlert_sp->addressLength+1)/2+2;
       char *text = new char[len*4+1];
       unsigned char* buf = (unsigned char*)msisdnAlert_sp;
       int k = 0;
       for ( int i=0; i<len; i++)
       {
         k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
       }
       text[k]=0;
       __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, "msisdnAlert_s: %s",text);
       delete [] text;
    }
    if( mwdStatus_sp && smsc::logger::_map_cat->isDebugEnabled() )
    {
       int len = sizeof(ET96MAP_MWD_STATUS_T);
       char *text = new char[len*4+1];
       unsigned char* buf = (unsigned char*)mwdStatus_sp;
       int k = 0;
       for ( int i=0; i<len; i++)
       {
         k+=sprintf(text+k,"%02x ",(unsigned)buf[i]);
       }
       text[k]=0;
       __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, "mwdStatus_s: %s",text);
       delete [] text;
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
  }MAP_CATCH(dialogid_map,dialogid_smsc,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

static bool NeedNotifyHLR(MapDialog* dialog)
{
  if( dialog->isUSSD || dialog->hlrWasNotified || dialog->hlrVersion == 0 ) return false;
  if( dialog->wasDelivered ) {
    if( dialog->hasMwdStatus ) return true;
  } else {
    if( dialog->hasMwdStatus ) {
      if( !(unsigned)dialog->mwdStatus.mnrf && (unsigned)dialog->subscriberAbsent ) return true;
      if( !(unsigned)dialog->mwdStatus.mcef && (unsigned)dialog->memoryExceeded ) return true;
    } else {
      if( dialog->subscriberAbsent || dialog->memoryExceeded ) return true;
    }
  }
  return false;
}
static void NotifyHLR(MapDialog* dialog)
{
  MapDialogContainer::getInstance()->reAssignDialog(dialog->instanceId,dialog->dialogid_map,dialog->ssn,SSN,radtOutSRI);
  dialog->id_opened = false;
  dialog->hlrWasNotified = true;
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
    __map_trace2__("%s: no way! isUSSD=%s hlrWasNotified=%s hlrVersion=%d wasDelivered=%s routeErr=%x hasMwd=%s subscriberAbsent=%s mwdStatus.mnrf=%x memoryExceeded=%s mwdStatus.mcef=%x",__func__,
        dialog->isUSSD?"Y":"N",
        dialog->hlrWasNotified?"Y":"N",
        dialog->hlrVersion,
        dialog->wasDelivered?"Y":"N",
        dialog->routeErr,
        dialog->hasMwdStatus?"Y":"N",
        dialog->subscriberAbsent?"Y":"N",
        dialog->mwdStatus.mnrf,
        dialog->memoryExceeded?"Y":"N",
        dialog->mwdStatus.mcef
    );

    //return; // Opps, strange way
    throw MAPDIALOG_ERROR(0,"no way");
  }

  checkMapReq( Et96MapOpenReq( SSN INSTDLGARG(dialog), dialog_id, &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 ), __func__);
  dialog->id_opened = true;

  //  !!!!
  if ( dialog->hlrVersion != 2 && dialog->hlrVersion != 1 ) dialog->hlrVersion = 2;
  // !!!!

  if ( dialog->hlrVersion == 2) {
    checkMapReq( Et96MapV2ReportSmDelStatReq( SSN INSTDLGARG(dialog),dialog->dialogid_map,1,&dialog->m_msAddr,&dialog->m_scAddr,deliveryOutcom), __func__);
  }else if ( dialog->hlrVersion == 1 ) {
    checkMapReq( Et96MapV1ReportSmDelStatReq( SSN INSTDLGARG(dialog),dialog->dialogid_map,1,&dialog->m_msAddr,&dialog->m_scAddr), __func__);
  } else
    throw runtime_error(
    FormatText("MAP::%s bad protocol version 0x%x",__func__,dialog->hlrVersion));
  checkMapReq( Et96MapDelimiterReq(SSN INSTDLGARG(dialog),dialog->dialogid_map,0,0), __func__);
  dialog->state = MAPST_WaitDelRepConf;
}

CALLBACK_DECL
USHORT_T Et96MapV2ReportSmDelStatConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ADDRESS_T *msisdnAlert_sp,
  ET96MAP_ERROR_REPORT_SM_DEL_STAT_T *errorReportSmDelStat_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  // nothig;
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV1ReportSmDelStatConf (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_REPORT_SM_DEL_STAT_T *errorReportSmDelStat_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  // nothig;
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapNoticeInd (
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_DIALOGUE_ID_T relDialogueId,
  ET96MAP_DIAGNOSTIC_PROBLEM_T diagProblem,
  ET96MAP_RETURNED_OPERATION_T *retOperation_sp)
{
  __map_trace2__("%s: dialogid 0x%x relDialogid 0x%x diagProblem %d retop 0x%x", __func__,
                dialogueId,relDialogueId,diagProblem,retOperation_sp );
  DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
  if ( !dialog.isnull() ) {
    if( dialog->state == MAPST_WaitSms ) {
      dialog->state = MAPST_MapNoticed;
    } else {
      __map_warn2__("%s: dialogid 0x%x unknown state %d", __func__, dialogueId, dialog->state);
    }
  } else {
    warnMapReq( Et96MapCloseReq (localSsn INSTDLGARG(dialog),dialogueId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
  }
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV1BeginSubscriberActivityInd(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T imsi_s,
  ET96MAP_ADDRESS_T originatingEntityNumber_s)
{
  unsigned __dialogid_map = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
      throw runtime_error(
                          FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    Address addr;
    ConvAddrMSISDN2Smc(&originatingEntityNumber_s,&addr);
    Address imsi;
    ConvAddrIMSI2Smc(&imsi_s,&imsi);

    auto_ptr<SMS> _sms ( new SMS() );
    SMS& sms = *_sms.get();
    if( MapDialogContainer::getUssdV1UseOrigEntityNumber() ) {
      sms.setOriginatingAddress(addr);
    } else {
      sms.setOriginatingAddress(imsi);
    }
    dialog->AssignSms(_sms.release());
  }MAP_CATCH(__dialogid_map,0,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

CALLBACK_DECL
USHORT_T Et96MapV1ProcessUnstructuredSSDataInd(
  ET96MAP_LOCAL_SSN_T localSsn  INSTANCEIDARGDEF(rinst),
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SS_USER_DATA_T ssUserData_s)
{
  unsigned __dialogid_map = 0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
      throw runtime_error(
                          FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    __require__(dialog->ssn==localSsn);
    dialog->isUSSD = true;
    __dialogid_map = dialogueId;
    dialog->invokeId = invokeId;
    dialog->origInvokeId = invokeId;

    SMS& sms = *dialog->sms.get();
    //sms.setIntProperty(Tag::SMSC_ORIGINAL_DC, smsc::smpp::DataCoding::LATIN1 );
    unsigned esm_class = 2; // Transaction mode
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    String32 subsystem = GetUSSDSubsystem((const char*)ssUserData_s.ssUserDataStr,ssUserData_s.ssUserDataStrLen);
    char outStr[256];
    unsigned outLen = GetUSSDRequestString((const char*)ssUserData_s.ssUserDataStr,ssUserData_s.ssUserDataStrLen,outStr,MapLimits::getInstance().getUssdParsing(subsystem.c_str()));
    sms.setIntProperty(Tag::SMPP_DATA_CODING,smsc::smpp::DataCoding::LATIN1);
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,outStr,outLen);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,(uint32_t)outLen);
    dialog->ussdMrRef = MakeMrRef();

    __map_trace2__("%s: dialogid 0x%x invokeid=%d request length %d subsystem %s mr=%x",__func__,dialogueId,invokeId,ssUserData_s.ssUserDataStrLen,subsystem.c_str(),dialog->ussdMrRef);
    dialog->subsystem=".5.0.ussd:";
    dialog->subsystem += subsystem.c_str();
    Address dest_addr = Address(dialog->subsystem.c_str());
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
    sms.setMessageReference(0);
    sms.setDestinationAddress(dest_addr);
    sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_IND);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,dialog->ussdMrRef);
    dialog->state = MAPST_WaitUssdV1Delimiter;
    dialog->invokeId = invokeId;
  }MAP_CATCH(__dialogid_map,0,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

void makeAtiRequest(MapDialog* dlg)
{
  bool success = false;
  unsigned localSsn = 0;
  MapDialog* dialog=dlg;
  if (!dialog) throw runtime_error(
    FormatText("MAP::%s null dialog",__func__));
  unsigned dialogid_map = dialog->dialogid_map;
  localSsn = dialog->ssn;
  MAP_TRY{
    //mkMapAddress( &dialog->m_scAddr, USSD_ADDRESS().c_str(), (unsigned)USSD_ADDRESS().length() );
    //mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, USSD_SSN );
    //mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, HLR_SSN );
    if(dialog->useAtiAfterSri)
    {
      dialog->state = MAPST_WaitNIUssdAtiOpenConf;
    }else
    {
      dialog->state = MAPST_WaitUssdAtiOpenConf;
    }

    ET96MAP_APP_CNTX_T appContext;
    appContext.acType = ET96MAP_ANY_TIME_INFO_ENQUIRY_CONTEXT;
    appContext.version=VERSION_NS::ET96MAP_VERSION_3;
    memset(&dialog->mwdStatus,0,sizeof(dialog->mwdStatus));
    dialog->memoryExceeded = false;
    dialog->subscriberAbsent = false;
    dialog->version=3;

    unsigned dialog_id = dialog->dialogid_map;
    __map_trace2__("%s: dlg 0x%x ssn:%d",__func__,dialog_id,dialog->ssn);
    checkMapReq( Et96MapOpenReq(
      dialog->ssn INSTDLGARG(dialog), dialog_id,
      &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 ), __func__);

    dialog->id_opened = true;

    BOOLEAN_T ssReq=dialog->isQueryAbonentStatus?TRUE:FALSE;

    checkMapReq( Et96MapV3AnyTimeInterrogationReq(dialog->ssn INSTDLGARG(dialog), dialog_id, 0,
        &dialog->m_scAddr, //ET96MAP_ADDRESS_T       *gsmSCF_sp,
        0,                 //ET96MAP_IMSI_T          *imsi_sp,
        &dialog->m_msAddr, //ET96MAP_ADDRESS_T       *msisdn_sp,
        TRUE,              //BOOLEAN_T               locationInfo,
        ssReq,             //BOOLEAN_T               subscriberState,
        TRUE,              //BOOLEAN_T               currentLocation,
        0,                 //ET96MAP_DOMAIN_TYPE_T   *requestedDomain_p,
        FALSE,             //BOOLEAN_T               imei,
        FALSE,             //BOOLEAN_T               msClassmark,
#ifdef ET96MAP_INTERFACE_VERSION_R14
        FALSE,             //BOOLEAN_T               mnpRequestedInfo,
#endif
        0,                 //ET96MAP_EXTENSIONDATA_T *requestedInfoExtension_sp,
        0                  //ET96MAP_EXTENSIONDATA_T *anyTimeInterrogationExtension_sp
         ), __func__);
    checkMapReq( Et96MapDelimiterReq(dialog->ssn INSTDLGARG(dialog), dialog_id, 0, 0 ), __func__);

  }MAP_CATCH(dialogid_map,0,localSsn,dialog->instanceId);
}

CALLBACK_DECL
USHORT_T Et96MapV3AnyTimeInterrogationConf(
    ET96MAP_LOCAL_SSN_T                    localSsn INSTANCEIDARGDEF(rinst),
    ET96MAP_DIALOGUE_ID_T                  dialogueId,
    ET96MAP_INVOKE_ID_T                    invokeId,
    ET96MAP_AGE_OF_LOCATION_INFO_T         *ageOfLocationInfo_p,
    ET96MAP_ATI_GEOGRAPHICAL_INFO_T        *geographicalInformation_sp,
    ET96MAP_ADDRESS_T                      *vlrNumber_sp,
    ET96MAP_LOCATIONNUMBER_T               *locationNumber_sp,
    ET96MAP_CELLID_OR_SERVICE_OR_LAI_T     *cellIdOrServiceOrLai_sp,
    BOOLEAN_T                              saiPresent,
    ET96MAP_SELECTED_LSA_ID_T              *selectedLSAId_sp,
    ET96MAP_ADDRESS_T                      *mscNumber_sp,
    ET96MAP_GEODETICINFO_T                 *geodeticInfo_sp,
    BOOLEAN_T                              currentLocationRetrived,
    ET96MAP_ATI_SUBSCRIBER_STATE_T         *subscriberState_sp,
    ET96MAP_CELLID_OR_SERVICE_OR_LAI_T     *gprsCellIdOrServiceOrLai_sp,
    ET96MAP_RA_IDENTITY_T                  *gprsRouteingAreaId_sp,
    ET96MAP_ATI_GEOGRAPHICAL_INFO_T        *gprsGeographicalInformation_sp,
    ET96MAP_ADDRESS_T                      *gprsSgsnNumber_sp,
    ET96MAP_SELECTED_LSA_ID_T              *gprsSelectedLSAId_sp,
    BOOLEAN_T                              gprsSaiPresent,
    ET96MAP_GEODETICINFO_T                 *gprsGeodeticInfo_sp,
    BOOLEAN_T                              gprsCurrentLocationRetrived,
    ET96MAP_AGE_OF_LOCATION_INFO_T         *gprsAgeOfLocationInfo_p,
    ET96MAP_ATI_PS_SUBSCRIBER_STATE_T      *psSubscriberState_sp,
    ET96MAP_IMEI_T                         *imei_sp,
    ET96MAP_MS_CLASSMARK2_T                *msClassmark2_sp,
    ET96MAP_GPRS_MS_CLASS_T                *gprsMsClass_sp,
#ifdef ET96MAP_INTERFACE_VERSION_R14
    ET96MAP_MNP_INFO_RES_T                 *mnpInfoRes_sp,
#endif
    ET96MAP_EXTENSIONDATA_T                *locationInfoExtension_sp,
    ET96MAP_EXTENSIONDATA_T                *locationInfoGprsExtension_sp,
    ET96MAP_EXTENSIONDATA_T                *subscriberInfoExtension_sp,
    ET96MAP_EXTENSIONDATA_T                *anyTimeInterrogationExtension_sp,
    ET96MAP_ERROR_ANY_TIME_INTERROGATION_T *errorAnyTimeInterrogation_sp,
    ET96MAP_PROV_ERR_T                     *provError_p)
{
  int dialogid_map=0;
  MAP_TRY{
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogueId,localSsn,INSTARG0(rinst)));
    if ( dialog.isnull() )
    {
      throw runtime_error(FormatText("MAP::%s MAP.did:{0x%x} is not present",__func__,dialogueId));
    }
    dialogid_map=dialogueId;
    __map_trace2__("%s: dlg 0x%x ssn:%d, vlr=%p, msc=%p",__func__,dialogid_map,dialog->ssn,vlrNumber_sp,mscNumber_sp);
    if(errorAnyTimeInterrogation_sp)
    {
      dialog->routeErr = DoMAPErrorProcessor(errorAnyTimeInterrogation_sp->errorCode,0);
    }else
    if(provError_p)
    {
      dialog->routeErr = DoMAPErrorProcessor(0,provError_p);
    }else
    {
      ET96MAP_ADDRESS_T* addr=vlrNumber_sp;
      if(!addr)
      {
        addr=mscNumber_sp;
      }
      if(addr)
      {
        if(dialog->useAtiAfterSri)
        {
          mkSS7GTAddress(&dialog->destMscAddr,addr,USSD_SSN);
        }
        Address vlr;
        ConvAddrMSISDN2Smc(addr,&vlr);
        dialog->s_msc=vlr.value;
      }
    }
#ifdef EIN_HD
#define STATE_NS
#else
#define STATE_NS ET96MAP_ATI_SUBSCRIBER_STATE_T
#endif
    if(dialog->isQueryAbonentStatus)
    {
      int status=AbonentStatus::UNKNOWNVALUE;
      if(subscriberState_sp)
      {
        switch(subscriberState_sp->choiceId)
        {
          case STATE_NS::ET96MAP_ATI_ASSUMED_IDLE:
          case STATE_NS::ET96MAP_ATI_CAMEL_BUSY:status=AbonentStatus::ONLINE;break;
          case STATE_NS::ET96MAP_ATI_NET_DET_NOT_REACHABLE:status=AbonentStatus::OFFLINE;break;
          case STATE_NS::ET96MAP_ATI_NOT_PROVIDED_FROM_VLR:status=AbonentStatus::UNKNOWNVALUE;break;
        }
      }
      //SendAbonentStatusToSmsc(dialog.get(),status);
      dialog->QueryAbonentCommand->get_abonentStatus().status=status;
    }
    if(dialog->state==MAPST_WaitUssdAtiConf)
    {
      dialog->state=MAPST_WaitUssdAtiClose;
    }else if(dialog->state==MAPST_WaitNIUssdAtiConf)
    {
      dialog->state=MAPST_WaitNIUssdAtiClose;
    }else
    {
      throw Exception("Unexpected state %d in %s",dialog->state,__func__);
    }
  }MAP_CATCH(dialogid_map,0,localSsn,INSTARG0(rinst));
  return ET96MAP_E_OK;
}

#else

//#include "MapDialog_spcific.cxx"
#include "smeman/smsccmd.h"
#include "MapIoTask.h"
using namespace smsc::smeman;
void MAPIO_PutCommand(const SmscCommand& cmd )
{
  if ( cmd->get_commandId() == QUERYABONENTSTATUS )
  {
    SmscCommand xcmd = SmscCommand::makeQueryAbonentStatusResp(cmd->get_abonentStatus(),AbonentStatus::ONLINE,0,"","");
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(xcmd);
  }
}

#endif
