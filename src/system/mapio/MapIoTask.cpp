#include "MapIoTask.h"
#include "logger/Logger.h"
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include "core/buffers/TmpBuf.hpp"
#ifdef SNMP
#include "snmp/SnmpAgent.hpp"
#include "system/snmp/SnmpCounter.hpp"
#endif


#define MAXENTRIES 8192

USHORT_T MY_USER_ID=USER01_ID;

using namespace std;

Mutex mapMutex;

#ifdef USE_MAP

#if !(EINSS7_THREADSAFE == 1)
#error "EINSS7_THREADSAFE REQUIRED!"
#endif


#ifdef EIN_HD
#define CONNINSTARG(arg) ,arg
#else
#define CONNINSTARG
#endif

//#define SMSC_FORWARD_RESPONSE 0x001

//unsigned __global_bind_counter = 0;
int bindTimer = 0;
//static unsigned __pingPongWaitCounter = 0;
static bool MAP_dispatching = false;
static bool MAP_isAlive = false;
static bool MAP_aborting = false;
bool MAP_disconnectDetected = false;
#define CORRECT_BIND_COUNTER 2
#define MAX_BIND_TIMEOUT 15



/*
 void MAPIO_TaskACVersionNotifier()
 {
   __pingPongWaitCounter = 0;
 }
*/

//extern void MAPIO_QueryMscVersionInternal();

extern "C" {

  USHORT_T Et96MapBindConf(ET96MAP_LOCAL_SSN_T lssn INSTANCEIDARGDEF(rinst), ET96MAP_BIND_STAT_T status)
  {
    __map_warn2__("%s: confirmation received ssn=%d status=%d",__func__,lssn,status);
    if( status == 0  )
    {
      for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
      {
        if( MapDialogContainer::localSSNs[i] == lssn )
        {
          MapDialogContainer::boundLocalSSNs[i] = 1;
          __map_warn2__("%s: local ssn=%d bound",__func__,lssn);

          break;
        }
      }
    } else if( status == 1 )
    {
      __map_warn2__("Et96MapBindConf SSN %d is already bound trying to reconnect",lssn);
      MAP_disconnectDetected = true;
    } else
    {
      __map_warn2__("Et96MapBindConf SSN %d is not bound trying to reconnect",lssn);
      MAP_disconnectDetected = true;
    }
#ifdef SNMP
    const char* sid="MAP_PROXY";
    smsc::system::SnmpCounter::SmeTrapSeverities trpSvrt=smsc::system::SnmpCounter::getInstance().getSmeSeverities(sid);
    char snmpMsg[1024];
    if(MAP_disconnectDetected)
    {
      if(trpSvrt.onRegisterFailed!=-1)
      {
        sprintf(snmpMsg,"ACTIVE ESME %s registration failed (AlaramID=%s; severity=%d)",sid,sid,trpSvrt.onRegisterFailed);
        smsc::snmp::SnmpAgent::trap(sid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegisterFailed,snmpMsg);
      }
    }else
    {
      if(trpSvrt.onRegister!=-1)
      {
        if(isMapBound())
        {
          sprintf(snmpMsg,"CLEARED ESME %s registration successful (AlaramID=%s; severity=%d)",sid,sid,trpSvrt.onRegister);
          smsc::snmp::SnmpAgent::trap(sid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegister,snmpMsg);
        }
      }
    }
#endif
    return ET96MAP_E_OK;
  }

  USHORT_T Et96MapStateInd (
                           ET96MAP_LOCAL_SSN_T lssn  INSTANCEIDARGDEF(rinst),
                           UCHAR_T userState,
                           UCHAR_T affectedSSN,
                           ULONG_T affectedSPC,
                           ULONG_T localSPC)
  {
    __map_warn2__("%s: received ssn=%d user state=%d affected SSN=%d affected SPC=%d local SPC=%d",__func__,lssn,userState,affectedSSN,affectedSPC,localSPC);
    if( affectedSPC == localSPC )
    {
      if( userState == 1 )
      {
        for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
        {
          if( MapDialogContainer::localSSNs[i] == affectedSSN )
          {
            if( MapDialogContainer::boundLocalSSNs[i] )
            {
              MapDialogContainer::boundLocalSSNs[i] = 0;
              __map_warn2__("%s: SSN %d is unavailable trying to rebind",__func__,affectedSSN);
              USHORT_T result = Et96MapBindReq(MY_USER_ID, SSN INSTARG(rinst));
              if (result!=ET96MAP_E_OK) {
                __map_warn2__("%s: SSN %d Bind error 0x%hx",__func__,affectedSSN,result);
              }
            }
            break;
          }
        }
      } else {
        for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ ) {
          if( MapDialogContainer::localSSNs[i] == affectedSSN ) {
            MapDialogContainer::boundLocalSSNs[i] = 1;
            break;
          }
        }
      }
    }
    return ET96MAP_E_OK;
  }

  void Et96MapIndicationError(USHORT_T error,UCHAR_T* errString INSTANCEIDARGDEF(rinst))
  {
    if ( errString ) {
      __map_warn2__("Et96MapIndicationError: error 0x%hx text %s",error,errString);
    } else {
      __map_warn2__("Et96MapIndicationError: error 0x%hx",error);
    }
  }

} // extern "C"

void MapIoTask::connect(unsigned timeout) {
  USHORT_T result;
  __map_warn__("Connecting to MAP stack");
//  result = MsgOpen(MY_USER_ID);
  result = EINSS7CpMsgPortOpen( MY_USER_ID, TRUE);
  if ( result != RETURN_OK )
  {
    __map_warn2__("Error at MsgOpen, code 0x%hx",result);
    kill(getpid(),17);
  }
  int tries = 0;
  for(int n=0;n<MapDialogContainer::remInstCount;n++)
  {
    while( !isStopping && tries < 60 )
    {
//    result = MsgConn(MY_USER_ID,ETSIMAP_ID);
      result = EINSS7CpMsgConnInst(MY_USER_ID, ETSIMAP_ID, MapDialogContainer::remInst[n]);
      if ( result != RETURN_OK )
      {
        __map_warn2__("Error at MsgConn, code 0x%hx, sleep 1 sec and retry connect",result);
        sleep(1);
        tries++;
      } else {
        break;
      }
    }
  }
  if(isStopping)
  {
    return;
  }
  if( tries >= 60 )
  {
    __map_warn2__("MsgConn error, %d attempts failed, aborting", tries);
    kill(getpid(),17);
  }
  if( timeout > 0 ) {
    __map_warn__("pause self and wait map initialization");
    sleep(timeout);
  }

  __map_warn2__("Binding %d subsystems", MapDialogContainer::numLocalSSNs);
  bindTimer = 0;
  for(int n=0;n<MapDialogContainer::remInstCount;n++)
  for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
  {
    result = Et96MapBindReq(MY_USER_ID, MapDialogContainer::localSSNs[i] CONNINSTARG(MapDialogContainer::remInst[n]));
    if (result!=ET96MAP_E_OK)
    {
      __map_warn2__("SSN %d Inst %d Bind error 0x%hx",MapDialogContainer::localSSNs[i],MapDialogContainer::remInst[n],result);
      throw runtime_error("bind error");
    }
  }
  {
    MapDialogContainer::getInstance()->restartStatistics();
  }
}

void MapIoTask::init(unsigned timeout)
{
  USHORT_T err;
  for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
  {
    MapDialogContainer::boundLocalSSNs[i] = 0;
    MapDialogContainer::patternBoundLocalSSNs[i] = 1;
    MapDialogContainer::getInstance()->InitLSSN(MapDialogContainer::localSSNs[i]);
  }
#ifdef EIN_HD
  EINSS7CpMain_CpInit(); 
  err=EINSS7CpRegisterMPOwner(MY_USER_ID);
  if ( err != RETURN_OK)
  { 
    __map_warn2__("Error at EINSS7CpRegisterMPOwner, code 0x%hx",err); 
    throw runtime_error("MsgInit error");
  } 
  err=EINSS7CpRegisterRemoteCPMgmt(CP_MANAGER_ID, 0, (char*)MapDialogContainer::remoteMgmtAddress.c_str());
  if ( err != RETURN_OK)
  { 
    __map_warn2__("Error at EINSS7CpRegisterRemoteCPMgmt, host='%s', code 0x%hx",MapDialogContainer::remoteMgmtAddress.c_str(),err); 
    throw runtime_error("MsgInit error");
  } 
#endif  
//  __pingPongWaitCounter = 0;
  err = EINSS7CpMsgInitiate( MAXENTRIES INSTARG(MapDialogContainer::localInst[0]), FALSE );
  /*
  if( MapDialogContainer::GetNodesCount() > 1 ) {
    if( MapDialogContainer::GetNodeNumber() == 2 ) MY_USER_ID = USER06_ID;
  } else {
    err = EINSS7CpMsgInitiate( MAXENTRIES INSTARG(MapDialogContainer::localInst[0]), FALSE );
  }*/
  if ( err != RETURN_OK )
  {
    __map_warn2__("Error at MsgInit, code 0x%hx",err);
    throw runtime_error("MsgInit error");
  }
  {
    MutexGuard mapMutexGuard(mapMutex);
    connect(timeout);
  }
  __map_trace__("MAP proxy init complete");
}

void MapIoTask::disconnect()
{
#ifdef SNMP
  smsc::system::SnmpCounter::SmeTrapSeverities smeTrpSvrt=smsc::system::SnmpCounter::getInstance().getSmeSeverities("MAP_PROXY");
  if(smeTrpSvrt.onUnregister!=-1)
  {
    char buf[1024];
    const char* sysId="MAP_PROXY";
    sprintf(buf,"ACTIVE ESME %s unregistered successfully (AlaramID=%s; severity=%d)",sysId,sysId,smeTrpSvrt.onUnregister);
    smsc::snmp::SnmpAgent::trap(sysId,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)smeTrpSvrt.onUnregister,buf);
  }
#endif
  USHORT_T result;
  __map_warn__("disconnect from MAP stack");

  for(int n=0;n<MapDialogContainer::remInstCount;n++)
  for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
  {
    result = Et96MapUnbindReq(MapDialogContainer::localSSNs[i] INSTARG(MapDialogContainer::remInst[n]));
    if ( result != ET96MAP_E_OK)
    {
      __map_warn2__("error at Et96MapUnbindReq SSN=%d errcode 0x%hx",MapDialogContainer::localSSNs[i],result);
    }
    MapDialogContainer::boundLocalSSNs[i] = 0;
  }

//  result = MsgRel(MY_USER_ID,ETSIMAP_ID);
  result = EINSS7CpMsgRelInst( MY_USER_ID, ETSIMAP_ID, 0);
  if ( result != MSG_OK) {
    __map_warn2__("error at MsgRel errcode 0x%hx",result);
//    if ( !isStopping ) kill(getpid(),17);
//    return;
  }

  result = MsgClose(MY_USER_ID);
  if ( result != MSG_OK) {
    __map_warn2__("error at MsgClose errcode 0x%hx",result);
//    if ( !isStopping ) kill(getpid(),17);
//    return;
  }
  MapDialogContainer::getInstance()->DropAllDialogs();
  sleep(1);
}

void MapIoTask::deinit( bool connected )
{
  USHORT_T result;
  __map_warn__("deinitialize MAP_PROXY");
  MapDialogContainer::destroyInstance();
  if( connected ) disconnect();

  MsgExit();
}

struct ReceiveGuard{
  EventMonitor& mon;
  bool& inReceive;
  bool& isStopping;
  bool active;
  ReceiveGuard(EventMonitor& m,bool& inrec,bool& isst):mon(m),
    inReceive(inrec),isStopping(isst),active(true)
  {
    MutexGuard mg(mon);
    while(inReceive && !isStopping)
    {
      mon.wait();
    }
    inReceive=true;
  }
  ~ReceiveGuard()
  {
    if(active)
    {
      mon.Lock();
      inReceive=false;
      mon.notify();
      mon.Unlock();
    }
  }
  void Deactivate()
  {
    active=false;
  }
};

void warnMapReq(USHORT_T result, const char* func);

void MapIoTask::dispatcher()
{
  MSG_T message;
  USHORT_T result;
  EINSS7INSTANCE_T rinst=0;

  //struct timeval utime, curtime;

  //smsc::logger::Logger *time_logger = smsc::logger::Logger::getInstance("map.itime");

  message.receiver = MY_USER_ID;
  for (;;)
  {
    MAP_isAlive = true;
    if ( isStopping )
    {
      return;
    }
    MAP_dispatching = true;

    if ( isStopping )
    {
      return;
    }

    DialogRefGuard dlg;
    {
      MutexGuard mg(receiveMon);

      result = EINSS7CpMsgRecv_r(&message,1000);

      //if ( time_logger->isDebugEnabled() ) gettimeofday( &utime, 0 );

      MAP_dispatching = false;

      if ( result == MSG_TIMEOUT )
      {
        if (MAP_disconnectDetected && !isStopping)
        {
          MutexGuard mapMutexGuard(mapMutex);
          if(MAP_disconnectDetected)
          {
            disconnect();
            connect();
            MAP_disconnectDetected = false;

          }
          continue;
        }
        if( memcmp( MapDialogContainer::boundLocalSSNs, MapDialogContainer::patternBoundLocalSSNs, MapDialogContainer::numLocalSSNs*sizeof(int)) == 0 ) continue;
        __map_warn2__("MAP:: check binders %d", bindTimer);
        if ( ++bindTimer <= MAX_BIND_TIMEOUT ) continue;
        __map_warn2__("MAP:: not all binders binded in %d seconds. Restarting...", MAX_BIND_TIMEOUT);
        if ( !isStopping )
        {
          MAP_disconnectDetected=true;
          MutexGuard mapMutexGuard(mapMutex);
          if(MAP_disconnectDetected)
          {
            disconnect();
            connect();
            MAP_disconnectDetected = false;
          }
        }
        continue;
      }
      if ( result == MSG_BROKEN_CONNECTION && !isStopping)
      {
        __map_warn2__("Broken connection %d", result);
        MAP_disconnectDetected=true;
        MutexGuard mapMutexGuard(mapMutex);
        if(MAP_disconnectDetected)
        {
          disconnect();
          connect();
          MAP_disconnectDetected = false;
        }
        continue;
      }
      if ( result != MSG_OK )
      {
        __map_warn2__("Error at MsgRecv with code %d",result);
        if ( !(MAP_aborting || isStopping) )
        {
  //        MAP_aborting = true;
  //        abort();
          MAP_disconnectDetected=true;
          MutexGuard mapMutexGuard(mapMutex);
          if(MAP_disconnectDetected)
          {
            disconnect();
            connect();
            MAP_disconnectDetected = false;
          }
        }
        continue;
      }

      __map_trace2__("MsgRecv receive msg with receiver 0x%hx sender 0x%hx prim 0x%hx size %d",message.receiver,message.sender,message.primitive,message.size);
      if ( smsc::logger::_mapmsg_cat->isDebugEnabled() && message.size <= 2048)
      {
        char text[8193];
        int k = 0;
        for ( int i=0; i<message.size; i++)
        {
          k+=sprintf(text+k,"%02x ",(unsigned)message.msg_p[i]);
        }
        text[k]=0;
        __log2__(smsc::logger::_mapmsg_cat,smsc::logger::Logger::LEVEL_DEBUG, "MsgRecv msg: %s",text);
      }
      if ( message.primitive == 0x8b && message.msg_p[6] >= 0x04 )
      {
        __map_trace__("MsgRecv hatching msg to reset priority order " );
        message.msg_p[6] = 0;
      } else if ( message.primitive == 0x8d && message.msg_p[4] >= 0x04 )
      {
        __map_trace__("MsgRecv hatching msg to reset priority order " );
        message.msg_p[4] = 0;
      }
      /*
      else if ( message.primitive == 0xa3 && message.size == 8 && message.msg_p[5] == 0x22 )
      {
        __map_trace__("MsgRecv hatching msg to fix sysfailure cause in ForwardMTConf " );
        message.msg_p[5] = 0x24;
      } else if ( message.primitive == 0x9f && message.size == 11 && message.msg_p[8] == 0x22 )
      {
        __map_trace__("MsgRecv hatching msg to fix sysfailure cause in SendRinfoForSmConf " );
        message.msg_p[8] = 0x24;
      }
      */

      if(message.primitive!=MAP_BIND_CONF && message.primitive!=MAP_STATE_IND &&
         message.primitive!=MAP_GET_AC_VERSION_CONF)
      {
        ET96MAP_DIALOGUE_ID_T dlgId=((ET96MAP_DIALOGUE_ID_T)message.msg_p[2])|(((ET96MAP_DIALOGUE_ID_T)message.msg_p[3])<<8);
          //(message.msg_p[2]<<8)| message.msg_p[3];
        ET96MAP_LOCAL_SSN_T lssn=message.msg_p[1];
#ifdef EIN_HD
        rinst=message.remoteInstance;
#endif
        if(message.primitive==MAP_OPEN_IND)
        {
          try{
            bool isUSSD=message.msg_p[4]==0x13 || message.msg_p[4]==0x14;
            dlg.assign(MapDialogContainer::getInstance()->createLockedDialog(dlgId,lssn,rinst,message.msg_p[5],isUSSD));
          }
          catch(std::exception& e)
          {
            __map_warn2__("%s: dialogid 0x%x %s",__func__,dlgId,e.what());
            ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
            warnMapReq( Et96MapOpenResp(lssn INSTARG(rinst),dlgId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0), __func__);
            warnMapReq( Et96MapCloseReq(lssn INSTARG(rinst),dlgId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
            // dailog limit reached - have to drop message
            EINSS7CpReleaseMsgBuffer(&message);
            continue;
          }
        }else
        {
          dlg.assign(MapDialogContainer::getInstance()->getLockedDialogOrEnqueue(dlgId,lssn,rinst,message));
          if(dlg.isnull())
          {
            continue;
          }
        }
      }

    }

    handleMessage(message);
    EINSS7CpReleaseMsgBuffer(&message);

    if(!dlg.isnull())
    {
      for(;;)
      {
        MSG_T msg;
        {
          MutexGuard dlgMg(dlg->mutex);
          if(!dlg->cmdQueue.Count() || dlg->isDropping)
          {
            dlg->isLocked=false;
            break;
          }
          dlg->cmdQueue.Pop(msg);
        }
        handleMessage(msg);
        EINSS7CpReleaseMsgBuffer(&msg);
      }
    }

    if (MAP_disconnectDetected && !isStopping)
    {
      MutexGuard mapMutexGuard(mapMutex);
      if(MAP_disconnectDetected)
      {
        disconnect();
        connect();
        MAP_disconnectDetected = false;
      }
    }
  }
}

void MapIoTask::handleMessage(MSG_T& message)
{
  //__map_trace2__("MAPIO::Handled msg:p=%d,sz=%d,buf=%p",message.primitive,message.size,message.msg_p);
  /*
  if(smsc::logger::_map_cat->isDebugEnabled())
  {
    smsc::core::buffers::TmpBuf<char,1024> text(message.size*4+1);
    int k = 0;
    for ( int i=0; i<message.size; i++)
    {
      k+=sprintf(text.get()+k,"%02x ",(unsigned)message.msg_p[i]);
    }
    __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, "MAPIO:LMessage dump: %s",text.get());
  }
  */
  USHORT_T map_result;
  __require__(message.size!=0);
  try {
    //MapDialogContainer::getInstance()->mapPacketReceived();
    if( message.primitive == 0x88 )
    {
      // MapOpenInd
      const int destAddrPos = 6;
      const int destRefPos = destAddrPos+message.msg_p[destAddrPos]+1;
      const int orgAddrPos = destRefPos+message.msg_p[destRefPos]+1;
      const int orgRefPos = orgAddrPos+message.msg_p[orgAddrPos]+1;
      const int specificInfoLenPos = orgRefPos+(message.msg_p[orgRefPos]+1)/2+1+(message.msg_p[orgRefPos]?1:0);
      ET96MAP_USERDATA_T specificInfo;
      specificInfo.specificInfoLen = ((USHORT_T)message.msg_p[specificInfoLenPos])|(((USHORT_T)message.msg_p[specificInfoLenPos+1])<<8);
      if( specificInfo.specificInfoLen > 0 )
      {
        memcpy(specificInfo.specificData, message.msg_p+specificInfoLenPos+2, specificInfo.specificInfoLen );
      }
      ET96MAP_APP_CNTX_T ctx;
      ctx.acType=(ET96MAP_APP_CONTEXT_T)message.msg_p[4];
      reinterpret_cast<int&>(ctx.version)=message.msg_p[5];
      map_result = Et96MapOpenInd(
                                  (ET96MAP_LOCAL_SSN_T)message.msg_p[1] INSTARG(message.remoteInstance), // SSN
                                  ((ET96MAP_DIALOGUE_ID_T)message.msg_p[2])|(((ET96MAP_DIALOGUE_ID_T)message.msg_p[3])<<8), // Dialogue ID
                                  &ctx, // AC version
                                  (message.msg_p[destAddrPos]>0)?(ET96MAP_SS7_ADDR_T*)(message.msg_p+destAddrPos):0, // dest ss7 addr
                                  (message.msg_p[orgAddrPos]>0)?(ET96MAP_SS7_ADDR_T*)(message.msg_p+orgAddrPos):0, // org ss7 addr
#ifdef MAP_R12
                                  (message.msg_p[destRefPos]>0)?(ET96MAP_IMSI_OR_MSISDN_T*)(message.msg_p+destRefPos):0, // dest ref
#else
                                  (message.msg_p[destRefPos]>0)?(ET96MAP_IMSI_T*)(message.msg_p+destRefPos):0, // dest ref
#endif
                                  (message.msg_p[orgRefPos]>0)?(ET96MAP_ADDRESS_T*)(message.msg_p+orgRefPos):0, // dest ref
                                  (specificInfo.specificInfoLen>0)?&specificInfo:0
                                  );
    } else
    {
      map_result = Et96MapHandleIndication(&message);
    }
  } catch(exception& e) {
    __map_warn2__("Exception occured during processing MAP primitive: %s", e.what());
    return;
  } catch (...) {
    __map_warn__("Unknown exception occured during processing MAP primitive");
    return;
  }

  if ( map_result != ET96MAP_E_OK && smsc::logger::_map_cat->isWarnEnabled() )
  {
    smsc::core::buffers::TmpBuf<char,1024> text(message.size*4+1);
    //char *text = new char[message.size*4+1];
    int k = 0;
    for ( int i=0; i<message.size; i++)
    {
      k+=sprintf(text.get()+k,"%02x ",(unsigned)message.msg_p[i]);
    }
    text[k]=0;
    __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN, "error at Et96MapHandleIndication with code x%hx msg: %s",map_result,text.get());
  }

  /*
  if ( time_logger->isDebugEnabled() )
  {
    long usecs;
    gettimeofday( &curtime, 0 );
    usecs = curtime.tv_usec < utime.tv_usec?(1000000+curtime.tv_usec)-utime.tv_usec:curtime.tv_usec-utime.tv_usec;
    smsc_log_debug(time_logger, "prim=%d s=%ld us=%ld", message.primitive, curtime.tv_sec-utime.tv_sec, usecs );
  }
  */
}


#else
void MapIoTask::connect(unsigned)
{
  __map_warn__("MapIoTask::connect: no map stack on this platform");
}

void MapIoTask::init(unsigned)
{
  __map_warn__("MapIoTask::init: no map stack on this platform");
}

void MapIoTask::disconnect()
{
  __map_warn__("MapIoTask::disconnect: no map stack on this platform");
}

void MapIoTask::deinit(bool)
     {
  __map_warn__("MapIoTask::deinit: no map stack on this platform");
}

void MapIoTask::dispatcher()
{
  Event e;
  __map_trace__("MapIoTask::dispatcher: no map stack on this platform");
  e.Wait();
}


#endif

MapDialogContainer* MapDialogContainer::container = 0;
//Mutex MapDialogContainer::sync_object;
smsc::logger::Logger* MapDialogContainer::loggerStatDlg = 0;
smsc::logger::Logger* MapDialogContainer::loggerMapPdu = 0;

void MapIoTask::Start()
{
  try {
    try {
      init();
    } catch (exception& e) {
      __map_warn2__("exception in mapio, restarting: %s",e.what());
      kill(getpid(),9);
    }
    is_started = true;
    __trace2__("signal mapiotask start:%p",startevent);
    startevent->SignalAll();
    for(int i=0;i<mapIoTaskCount;i++)
    {
      tp.startTask(new DispatcherExecutor(this));
    }
  } catch (exception& e) {
    __map_warn2__("exception in mapio: %s",e.what());
  }
}

string MapDialogContainer::SC_ADRESS_VALUE = "79029869999";
string MapDialogContainer::USSD_ADRESS_VALUE = "79029869998";
ET96MAP_LOCAL_SSN_T MapDialogContainer::ussdSSN = 6;
int MapDialogContainer::nodeNumber = 1;
int MapDialogContainer::nodesCount = 1;
int MapDialogContainer::busyMTDelay = 10;
int MapDialogContainer::lockedByMoDelay = 10;
int MapDialogContainer::MOLockTimeout = 45;
bool MapDialogContainer::allowCallBarred = false;
bool MapDialogContainer::ussdV1Enabled = false;
bool MapDialogContainer::ussdV1UseOrigEntityNumber = false;
ET96MAP_LOCAL_SSN_T *MapDialogContainer::localSSNs = 0;
int                  MapDialogContainer::numLocalSSNs = 0;
int                 *MapDialogContainer::boundLocalSSNs = 0;
int                 *MapDialogContainer::patternBoundLocalSSNs = 0;

EINSS7INSTANCE_T MapDialogContainer::allInst[10];
int MapDialogContainer::allInstCount=0;
EINSS7INSTANCE_T MapDialogContainer::localInst[10];
int MapDialogContainer::localInstCount=0;
EINSS7INSTANCE_T MapDialogContainer::remInst[10];
int MapDialogContainer::remInstCount=0;
std::string MapDialogContainer::remoteMgmtAddress;

MapProxy* MapDialogContainer::proxy = 0;

void MapDialogContainer::abort()
{
#ifdef USE_MAP
/*  MAP_aborting = true;
  MsgClose(MY_USER_ID);
  MsgExit();
  EINSS7CpMsgClean();*/
#endif
}

int MapTracker::Execute(){
  Event e;
#ifdef USE_MAP
  for (;;) {
    time_t t = time(0);
    while (time(0)<(t+15)&&!isStopping) {
      time_t xx = time(0);
      if ( xx > t+15 ) break;
      e.Wait((int)(1000*(t-xx+15)));
    }
    __map_trace2__("MAP tracker:: alive %d dispatching %d",MAP_isAlive,MAP_dispatching);
    if ( isStopping ) return 0;
    if ( MAP_dispatching && !MAP_isAlive ) {
      __map_warn__("\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                    "MAP is DEAD"
                    "\n\n\n\n");
      abort();
    } else MAP_isAlive = false;
  }
#else
  e.Wait();
#endif
  return 0;
}

void MapDialogContainer::registerSelf(SmeManager* smeman)
{
  proxy->init();
  __map_trace__("register MAP_PROXY");
  smeman->registerInternallSmeProxy("MAP_PROXY",proxy);
  proxy->assignSmeRegistrar(smeman);
  __map_trace__("register MAP_PROXY OK");
}

void MapDialogContainer::unregisterSelf(SmeManager* smeman)
{
  __map_trace__("unregister MAP_PROXY");
  smeman->unregisterSmeProxy(proxy);
  __map_trace__("unregister MAP_PROXY OK");
}

void setMapProxyLimits(int timeout, int limit)
{
  MutexGuard mapMutexGuard(mapMutex);
  MapDialogContainer::getInstance()->setPerformanceLimits( timeout, limit );
}


/*
  Mutex& MAPSTATS_GetMutex(){
    static Mutex mutex;
    return mutex;
 }
*/

smsc::logger::Logger* MAPSTATS_GetLoggerSec() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.sec");
  return logger;
}
smsc::logger::Logger* MAPSTATS_GetLoggerMin() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.min");
  return logger;
}
smsc::logger::Logger* MAPSTATS_GetLoggerHour() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.hour");
  return logger;
}
smsc::logger::Logger* MAPSTATS_GetLoggerUpdate() {
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.update");
  return logger;
}

static time_t MAPSTATS_last_time_sec = 0;
static time_t MAPSTATS_last_time_min = 0;
static time_t MAPSTATS_last_time_hour = 0;

int MAPSTATS_open[MAPSTATS_COUNT][3] = {0,};
int MAPSTATS_close[MAPSTATS_COUNT][3] = {0,};
int MAPSTATS_recv[3] = {0,};

int MAPSTATS_dialogs[MAPSTATS_COUNT]={0,};
char *MAPSTATS_types[] = {
  "GSMRECV",
  "NEWDIALOG_IN",
  "DISPOSEDIALOG_IN",
  "NEWDIALOG_INSRI",
  "DISPOSEDIALOG_INSRI",
  "NEWDIALOG_USSD",
  "DISPOSEDIALOG_USSD",
  "NEWDIALOG_OUT",
  "DISPOSEDIALOG_OUT",
  "NEWDIALOG_NIUSSD",
  "DISPOSEDIALOG_NIUSSD"
};

enum {
  MAPSTATS__SEC,
  MAPSTATS__MIN,
  MAPSTATS__HOUR
};

void MAPSTATS_DumpDialogLC(MapDialog* dialog)
{
  if( MapDialogContainer::loggerStatDlg->isDebugEnabled() ) {
    struct timeval tv;
    gettimeofday( &tv, 0 );
    long long maked_mks = dialog->maked_at_mks;
    long long mks = ((long long)tv.tv_sec)*1000*1000 + (long long)tv.tv_usec;
    long long cl = mks-maked_mks;
    smsc_log_info(MapDialogContainer::loggerStatDlg, "dlg=%p (%x/%x) ussd=%s sec=%ld usec=%ld src=%s dst=%s",
                                  dialog,dialog->dialogid_map,dialog->dialogid_smsc,
                                  dialog->isUSSD?"true":"false",
                                  long(cl/(1000*1000)),long(cl%(1000*1000)),
                                  dialog->sms.get()?dialog->sms->getOriginatingAddress().value:"???",
                                  dialog->sms.get()?dialog->sms->getDestinationAddress().value:"???");
  }
}

static void MAPSTATS_Flush2(int idx)
{
  smsc::logger::Logger* log = MAPSTATS_GetLoggerSec();
  smsc_log_info(log, "open %d/%d/%d/%d/%d, closed %d/%d/%d/%d/%d, dlg %d/%d/%d/%d/%d, rcv %d",
            MAPSTATS_open[0][idx],
            MAPSTATS_open[1][idx],
            MAPSTATS_open[2][idx],
            MAPSTATS_open[3][idx],
            MAPSTATS_open[4][idx],

            MAPSTATS_close[0][idx],
            MAPSTATS_close[1][idx],
            MAPSTATS_close[2][idx],
            MAPSTATS_close[3][idx],
            MAPSTATS_close[4][idx],
            MAPSTATS_dialogs[0],
            MAPSTATS_dialogs[1],
            MAPSTATS_dialogs[2],
            MAPSTATS_dialogs[3],
            MAPSTATS_dialogs[4],
            MAPSTATS_recv[idx]
           );
}

void MAPSTATS_Flush(unsigned x,bool dump)
{
  if ( dump ) {
    switch ( x ) {
    case MAPSTATS__SEC:
      {
        MAPSTATS_Flush2(0);
      }
      break;
    case MAPSTATS__MIN:
      {
        MAPSTATS_Flush2(1);
      }
      break;
    case MAPSTATS__HOUR:
      {
        MAPSTATS_Flush2(2);
      }
      break;
    }
  }
  int from = 0, to = 0;
  switch ( x ) {
  case MAPSTATS__SEC: from = 0; to = 1; break;
  case MAPSTATS__MIN: from = 1; to = 2; break;
  case MAPSTATS__HOUR: from = 2; to = -1; break;
  }
  if ( to != -1 )
  {
    for(int i=0;i<MAPSTATS_COUNT;i++)
    {
      MAPSTATS_open[i][to] += MAPSTATS_open[i][from];
      MAPSTATS_close[i][to] += MAPSTATS_close[i][from];
    }
    MAPSTATS_recv[to] += MAPSTATS_recv[from];
  }
  for(int i=0;i<MAPSTATS_COUNT;i++)
  {
    MAPSTATS_open[i][from] = 0;
    MAPSTATS_close[i][from] = 0;
  }
  MAPSTATS_recv[from] = 0;
}

void MAPSTATS_Update_(MAPSTATS stats)
{
  if(stats==MAPSTATS_GSMRECV)
  {
    ++MAPSTATS_recv[0];
  }else if(stats<=MAPSTATS_DISPOSEDIALOG_NIUSSD)
  {
    int idx=stats;
    idx--;
    if(idx&1)
    {
      idx=(idx-1)/2;
      MAPSTATS_close[idx][0]++;
      MAPSTATS_dialogs[idx]--;
    }else
    {
      idx/=2;
      MAPSTATS_open[idx][0]++;
      MAPSTATS_dialogs[idx]++;
    }
  }
  if( stats != MAPSTATS_GSMRECV )
  {
    smsc_log_debug(MAPSTATS_GetLoggerUpdate(),
                  "updated %s dlg %d/%d/%d/%d/%d",
                   MAPSTATS_types[stats],
                   MAPSTATS_dialogs[0],
                   MAPSTATS_dialogs[1],
                   MAPSTATS_dialogs[2],
                   MAPSTATS_dialogs[3],
                   MAPSTATS_dialogs[4]
                  );
  }
//  if( MAPSTATS_dialogs_in + MAPSTATS_dialogs_out - MapDialogContainer::getInstance()->getDialogCount() > 1 ) {
//    smsc_log_warn(MAPSTATS_GetLoggerUpdate(),"Number of dialogs unbalanced %d/%d/%d",
//                  MAPSTATS_dialogs_in,
//                  MAPSTATS_dialogs_out,
//                  MapDialogContainer::getInstance()->getDialogCount()
//                  );
//  }
}

void MAPSTATS_Restart()
{
  //MutexGuard _mg(MAPSTATS_GetMutex());
  MAPSTATS_Flush(MAPSTATS__SEC,false);
  MAPSTATS_Flush(MAPSTATS__MIN,false);
  MAPSTATS_Flush(MAPSTATS__HOUR,false);
  time_t cur_time = time(0);
  MAPSTATS_last_time_sec = cur_time;
  MAPSTATS_last_time_min = cur_time;
  MAPSTATS_last_time_hour = cur_time;
  memset(MAPSTATS_dialogs,0,sizeof(MAPSTATS_dialogs));
}

void MAPSTATS_Update(MAPSTATS stats)
{
  //MutexGuard _mg(MAPSTATS_GetMutex());
  time_t cur_time = time(0);
  if ( cur_time > MAPSTATS_last_time_hour+60*60 )
  {
    // dump one hour stats
    MAPSTATS_last_time_hour = cur_time;
    MAPSTATS_Flush(MAPSTATS__HOUR,true);
  }
  if ( cur_time > MAPSTATS_last_time_min+60 )
  {
    // dump one minute stats
    MAPSTATS_last_time_min = cur_time;
    MAPSTATS_Flush(MAPSTATS__MIN,true);
  }
  if ( cur_time >= MAPSTATS_last_time_sec+1 )
  {
    // dump one second stats
    MAPSTATS_last_time_sec = cur_time;
    MAPSTATS_Flush(MAPSTATS__SEC,true);
  }
  MAPSTATS_Update_(stats);
}

void MAPSTATS_DumpDialog(MapDialog* dlg, time_t now, bool expired)
{
  static smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("map.stat.dlgdump");
  if( expired ) {
    smsc_log_warn(logger, "Expired dlg/map/smsc 0x%x/0x%x/0x%x(%s) state: %d, %ld sec, {%s->%s}",
                 dlg,
                 dlg->dialogid_map,
                 dlg->dialogid_smsc,
                 dlg->isUSSD?"USSD":"SMS",
                 (int)dlg->state,
                 (long)(now-(dlg->maked_at_mks/1000000)),
                 dlg->sms.get()?dlg->sms->getOriginatingAddress().value:"???",
                 dlg->sms.get()?dlg->sms->getDestinationAddress().value:"???");
  } else {
    smsc_log_info(logger, "dlg/map/smsc 0x%x/0x%x/0x%x(%s) state: %d, %ld sec, {%s->%s}",
                 dlg,
                 dlg->dialogid_map,
                 dlg->dialogid_smsc,
                 dlg->isUSSD?"USSD":"SMS",
                 (int)dlg->state,
                 (long)(now-(dlg->maked_at_mks/1000000)),
                 dlg->sms.get()?dlg->sms->getOriginatingAddress().value:"???",
                 dlg->sms.get()?dlg->sms->getDestinationAddress().value:"???");
  }
}

void MapProxy::checkLogging() {
#ifdef USE_MAP
/*  if ( smsc::logger::Logger::getInstance("map.trace.user1")->isDebugEnabled() ) {
    __map_trace__("Enable trace for system USER01");
    MsgTraceOn( USER01_ID );
  } else {
    __map_trace__("Disable trace for system USER01");
    MsgTraceOff( USER01_ID );
  }
  if ( smsc::logger::Logger::getInstance("map.trace.etsimap")->isDebugEnabled() ) {
    __map_trace__("Enable trace for system ETSIMAP_ID");
    MsgTraceOn( ETSIMAP_ID );
  } else {
    __map_trace__("Disable trace for system ETSIMAP_ID");
    MsgTraceOff( ETSIMAP_ID );
  }
  if ( smsc::logger::Logger::getInstance("map.trace.tcap")->isDebugEnabled() ) {
    __map_trace__("Enable trace for system TCAP_ID");
    MsgTraceOn( TCAP_ID );
  } else {
    __map_trace__("Disable trace for system TCAP_ID");
    MsgTraceOff( TCAP_ID );
  }
*/
#endif
}

bool isMapBound() {
#ifdef USE_MAP
  return memcmp( MapDialogContainer::boundLocalSSNs, MapDialogContainer::patternBoundLocalSSNs, MapDialogContainer::numLocalSSNs*sizeof(int)) == 0 ;
#else
  return false;
#endif
}
