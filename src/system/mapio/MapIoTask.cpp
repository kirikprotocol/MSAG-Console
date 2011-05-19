#include "MapIoTask.h"
#include "logger/Logger.h"
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "core/buffers/TmpBuf.hpp"
#include "system/smsc.hpp"
#ifdef SNMP
#include "snmp/SnmpAgent.hpp"
#include "system/snmp/SnmpCounter.hpp"
#endif
#include "system/smsc.hpp"


#define MAXENTRIES 40000

USHORT_T MY_USER_ID=USER01_ID;//!!

namespace smsc{
namespace system{
namespace mapio{

volatile bool MAP_connectedInst[10]={
false,
};

volatile int MAP_connectedInstCount=0;

}
}
}

void warnMapReq(USHORT_T result, const char* func);


#ifdef USE_MAP

#if !(EINSS7_THREADSAFE == 1)
#error "EINSS7_THREADSAFE REQUIRED!"
#endif


#ifdef EIN_HD
#define CONNINSTARG(arg) ,arg
#else
#define CONNINSTARG(arg)
#endif

//#define SMSC_FORWARD_RESPONSE 0x001

//static bool MAP_dispatching = false;
//static bool MAP_isAlive = false;
static bool MAP_aborting = false;



#define MAX_BIND_TIMEOUT 15


using namespace smsc::system::mapio;

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
    __map_warn2__("%s: confirmation received ssn=%d rinst=%d status=%d",__func__,lssn,INSTARG0(rinst),status);
    bool isBound=false;
    if( status == 0  /*|| status==1*/)//already bound is error
    {
      for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
      {
        if( MapDialogContainer::localSSNs[i] == lssn )
        {
          MapDialogContainer::boundLocalSSNs[INSTARG0(rinst)] |= (uint64_t)1<<i;
          isBound=true;
          __map_warn2__("%s: local ssn=%d rinst=%d bound",__func__,lssn,INSTARG0(rinst));
          break;
        }
      }
    }
    if(!isBound)
    {
      MapIoTask::ReconnectThread::reportDisconnect(INSTARG0(rinst),true);
    }
#ifdef SNMP
    const char* sid="MAP_PROXY";
    smsc::system::SnmpCounter::SmeTrapSeverities trpSvrt=smsc::system::SnmpCounter::getInstance().getSmeSeverities(sid);
    char snmpMsg[1024];
    if(isBound)
    {
      if(trpSvrt.onRegister!=-1)
      {
        if(isMapBound())
        {
          sprintf(snmpMsg,"CLEARED ESME %s registration successful (AlaramID=%s; severity=%d)",sid,sid,trpSvrt.onRegister);
          smsc::snmp::SnmpAgent::trap(sid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegister,snmpMsg);
        }
      }
    }else
    {
      if(trpSvrt.onRegisterFailed!=-1)
      {
        sprintf(snmpMsg,"ACTIVE ESME %s registration failed (AlaramID=%s; severity=%d)",sid,sid,trpSvrt.onRegisterFailed);
        smsc::snmp::SnmpAgent::trap(sid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegisterFailed,snmpMsg);
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
    __map_warn2__("%s: received ssn=%d rinst=%d user state=%d affected SSN=%d affected SPC=%d local SPC=%d",__func__,lssn,INSTARG0(rinst),userState,affectedSSN,affectedSPC,localSPC);
    if( affectedSPC == localSPC )
    {
      if( userState == 1 )
      {
        for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
        {
          if( MapDialogContainer::localSSNs[i] == affectedSSN )
          {
            if( MapDialogContainer::boundLocalSSNs[INSTARG0(rinst)]&((uint64_t)1<<i) )
            {
              MapDialogContainer::boundLocalSSNs[INSTARG0(rinst)] &= ~((uint64_t)1<<i);
              MapIoTask::ReconnectThread::reportDisconnect(INSTARG0(rinst),true);
              /*
              __map_warn2__("%s: SSN %d rinst %d is unavailable trying to rebind",__func__,affectedSSN,INSTARG0(rinst));
              USHORT_T result = Et96MapBindReq(MY_USER_ID, lssn INSTARG(rinst));
              if (result!=ET96MAP_E_OK)
              {
                __map_warn2__("%s: SSN %d Bind error 0x%hx",__func__,affectedSSN,result);
                MapIoTask::ReconnectThread::reportDisconnect(INSTARG0(rinst));
              }*/
            }
            break;
          }
        }
      } /*else {
        for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
        {
          if( MapDialogContainer::localSSNs[i] == affectedSSN )
          {
            MapDialogContainer::boundLocalSSNs[INSTARG0(rinst)] |= (uint64_t)1<<i;
            break;
          }
        }
      }*/
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

extern "C" uint16_t onBrokenConn(uint16_t fromID,
                        uint16_t toID,
                        uint8_t inst)
{
  __map_warn2__("broken conn:%d->%d,%d",fromID,toID,inst);
  MapIoTask::ReconnectThread::reportDisconnect(inst,false);
  /*MutexGuard mg(reconnectMon);
  MAP_connectedInstCount--;
  if(inst<10)
  {
    MAP_connectedInst[inst]=false;
  }
  __map_trace2__("MAP_connectedInstCount=%d",MAP_connectedInstCount);
  reconnectMon.notify();*/
  return RETURN_OK;
}

namespace smsc{
namespace system{
namespace mapio{

MapDialogContainer::MapDialogContainer()
{
  memset(dlgPool,0,sizeof(dlgPool));
  last_dump_time=0;
  dialogsCount=0;
  rinstIdx=0;
}


bool MapIoTask::ReconnectThread::connect()
{
  USHORT_T result;
  __map_warn__("Connecting to MAP stack");
//  result = MsgOpen(MY_USER_ID);
  MAP_connectedInstCount=0;
  int attempt=0;
  while(!isStopping && MAP_connectedInstCount==0)
  {
    for(int n=0;n<MapDialogContainer::remInstCount;n++)
    {
      __map_trace2__("connecting instance %d",MapDialogContainer::remInst[n]);
      result = EINSS7CpMsgConnNotify(MY_USER_ID, ETSIMAP_ID, MapDialogContainer::remInst[n],onBrokenConn);
      if ( result != RETURN_OK )
      {
        __map_warn2__("Error at MsgConn, code %hd, sleep 1 sec and retry connect",result);
      }else
      {
        MAP_connectedInstCount++;
        MAP_connectedInst[MapDialogContainer::remInst[n]]=true;
      }
    }
    if(MAP_connectedInstCount==0)
    {
      __map_warn__("No instancies are connected. Sleep 1 sec before reconnect attempt");
      sleep(1);
      attempt++;
      if(attempt==5)return false;
    }
  }
  if(isStopping)
  {
    return false;
  }
  sleep(1);
  bool bindOk=false;
  __map_warn2__("Binding %d subsystems", MapDialogContainer::numLocalSSNs);
  for(int n=0;n<MapDialogContainer::remInstCount;n++)
  {
    if(MAP_connectedInst[MapDialogContainer::remInst[n]])
    {
      for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
      {
        MapDialogContainer::boundLocalSSNs[MapDialogContainer::remInst[n]] &= ~((uint64_t)1<<i);
        result = Et96MapBindReq(MY_USER_ID, MapDialogContainer::localSSNs[i] CONNINSTARG(MapDialogContainer::remInst[n]));
        if (result!=ET96MAP_E_OK)
        {
          __map_warn2__("SSN %d Inst %d Bind error 0x%hx",MapDialogContainer::localSSNs[i],MapDialogContainer::remInst[n],result);
          //throw runtime_error("bind error");
        }else
        {
          bindOk=true;
        }
      }
    }
  }
  MapDialogContainer::getInstance()->restartStatistics();
  return bindOk;
}

EventMonitor MapIoTask::ReconnectThread::reconnectMon;

void MapIoTask::ReconnectThread::reportDisconnect(int rinst,bool needRel)
{
  MutexGuard mg(reconnectMon);
  if(needRel)
  {
    EINSS7CpMsgRelInst( MY_USER_ID, ETSIMAP_ID,rinst);
  }
  if(MAP_connectedInst[rinst])
  {
    MAP_connectedInstCount--;
    MAP_connectedInst[rinst]=false;
  }
  __map_trace2__("Disconnected rinst=%d MAP_connectedInstCount=%d",rinst,MAP_connectedInstCount);
  reconnectMon.notify();
}

class MutexTempUnlock{
public:
  MutexTempUnlock(Mutex& argMtx):mtx(argMtx)
  {
    mtx.Unlock();
  }
  ~MutexTempUnlock()
  {
    mtx.Lock();
  }

protected:
  Mutex& mtx;
};


void MapIoTask::ReconnectThread::init(bool firstTime)
{
  const int SLEEPTIME=5;
#ifdef EIN_HD
  reinit:
  if( isStopping || smsc::system::Smsc::getInstance().getStopFlag()) return;
  USHORT_T err;
  EINSS7CpMain_CpInit();
  err = EINSS7CpRegisterMPOwner(MY_USER_ID);
  if (err != RETURN_OK)
  {
    __map_warn2__("Error at EINSS7CpRegisterMPOwner, code 0x%hx",err);
    sleep(SLEEPTIME);
    MsgExit();
    goto reinit;
  }
  err=EINSS7CpRegisterRemoteCPMgmt(CP_MANAGER_ID, 0, (char*)MapDialogContainer::remoteMgmtAddress.c_str());
  if ( err != RETURN_OK)
  {
    __map_warn2__("Error at EINSS7CpRegisterRemoteCPMgmt, host='%s', code 0x%hx",MapDialogContainer::remoteMgmtAddress.c_str(),err);
    sleep(SLEEPTIME);
    MsgExit();
    goto reinit;
    //throw runtime_error("MsgInit error");
  }

  for(int i=0;i<MapDialogContainer::localInstCount;i++)
  {
    err = EINSS7CpMsgInitiate( MAXENTRIES, MapDialogContainer::localInst[i], FALSE );
    if ( err != RETURN_OK )
    {
      __map_warn2__("Error at MsgInit, code 0x%hx",err);
      sleep(SLEEPTIME);
      MsgExit();
      goto reinit;
    }
  }
  EINSS7CpCreateMessagePool(5000,65535);
  err= EINSS7CpMsgPortOpen( MY_USER_ID, TRUE);
  if ( err != RETURN_OK )
  {
    __map_warn2__("Error at MsgOpen, code 0x%hx",err);
    sleep(SLEEPTIME);
    MsgExit();
    goto reinit;
  }

  {
    if(!connect())
    {
      sleep(SLEEPTIME);
      MsgExit();
      goto reinit;
    }
  }
#else
  int reinitCount=0;
  reinit:
  reinitCount++;
  if(reinitCount>90/SLEEPTIME)
  {
    __map_warn2__("Reconnect count=%d, exiting!",reinitCount);
    smsc::system::Smsc::getInstance().stop();
    sleep(2);
  }
  if( isStopping || smsc::system::Smsc::getInstance().getStopFlag()) return;
  USHORT_T err;

  if(firstTime)
  {
    err = EINSS7CpMsgInitiate( MAXENTRIES, 0, FALSE );
    if ( err != RETURN_OK )
    {
      __map_warn2__("Error at MsgInit, code 0x%hx",err);
      sleep(SLEEPTIME);
      goto reinit;
    }
  }
  EINSS7CpCreateMessagePool(5000,65535);
  err= EINSS7CpMsgPortOpen( MY_USER_ID, TRUE);
  if ( err != RETURN_OK )
  {
    __map_warn2__("Error at MsgOpen, code 0x%hx",err);
    sleep(SLEEPTIME);
    goto reinit;
  }

  {
    if(!connect())
    {
      MsgClose(MY_USER_ID);
      sleep(SLEEPTIME);
      goto reinit;
    }
  }
#endif
}

int MapIoTask::ReconnectThread::Execute()
{
  USHORT_T result;
  while(!isStopping)
  {
    MutexGuard mg(reconnectMon);
    if(MAP_connectedInstCount==MapDialogContainer::remInstCount)
    {
      reconnectMon.wait(5000);
    }
    if(MAP_connectedInstCount==MapDialogContainer::remInstCount)
    {
      continue;
    }
    if(isStopping)
    {
      break;
    }

    if(MAP_connectedInstCount==0)
    {
      MutexTempUnlock mung(reconnectMon);
      if(!firstConnect)
      {
        disconnect();
      }
      sleep(3);
      init(firstConnect);
      firstConnect=false;
      continue;
    }

    for(int n=0;n<MapDialogContainer::remInstCount;n++)
    {
      if(MAP_connectedInst[MapDialogContainer::remInst[n]])
      {
        continue;
      }
      MapDialogContainer::getInstance()->DropAllDialogs(MapDialogContainer::remInst[n]);
      __map_warn2__("Reconnecting instance %d",MapDialogContainer::remInst[n]);
      bool bindOk=true;
      {
        MutexTempUnlock mung(reconnectMon);
        MapDialogContainer::boundLocalSSNs[MapDialogContainer::remInst[n]]=0;
        result = EINSS7CpMsgConnNotify(MY_USER_ID, ETSIMAP_ID, MapDialogContainer::remInst[n],onBrokenConn);
        if(result != RETURN_OK)
        {
          __map_warn2__("EINSS7CpMsgConnNotify returned error %d",result);
          continue;
        }
        sleep(4);
        for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
        {
          result = Et96MapBindReq(MY_USER_ID, MapDialogContainer::localSSNs[i] CONNINSTARG(MapDialogContainer::remInst[n]));
          if (result!=ET96MAP_E_OK)
          {
            __map_warn2__("SSN %d Inst %d Bind error 0x%hx",MapDialogContainer::localSSNs[i],MapDialogContainer::remInst[n],result);
            EINSS7CpMsgRelInst( MY_USER_ID, ETSIMAP_ID, MapDialogContainer::remInst[n]);
            bindOk=false;
            break;
          }
        }
      }
      if(bindOk)
      {
        MAP_connectedInst[MapDialogContainer::remInst[n]]=true;
      }
    }
    MAP_connectedInstCount=0;
    for(int n=0;n<MapDialogContainer::remInstCount;n++)
    {
      if(MAP_connectedInst[MapDialogContainer::remInst[n]])
      {
        MAP_connectedInstCount++;
      }
    }
    __map_warn2__("Reconnect: MAP_connectedInstCount=%d",MAP_connectedInstCount);
    if(MAP_connectedInstCount!=MapDialogContainer::remInstCount)
    {
      sleep(3);
    }
  }
  return 0;
}

void MapIoTask::init(unsigned timeout)
{
  USHORT_T err;
  MapDialogContainer::patternBoundLocalSSNs=0;
  for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
  {
    MapDialogContainer::patternBoundLocalSSNs|=(uint64_t)1<<i;
    MapDialogContainer::getInstance()->InitLSSN(MapDialogContainer::localSSNs[i]);
  }
  for( int i = 0; i < 10; i++ )
  {
    MapDialogContainer::boundLocalSSNs[i] = 0;
  }
  for(int n=0;n<MapDialogContainer::remInstCount;n++)
  {
    for(int i=0;i<MapDialogContainer::numLocalSSNs;i++)
    {
      widxMap[MapDialogContainer::remInst[n]][MapDialogContainer::localSSNs[i]]=new unsigned char[65536];
      memset(widxMap[MapDialogContainer::remInst[n]][MapDialogContainer::localSSNs[i]],255,65536);
    }
  }
  __map_trace__("MAP proxy init complete");
}

void MapIoTask::ReconnectThread::disconnect()
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
  {
    if(!MAP_connectedInst[n])
    {
      continue;
    }
    for( int i = 0; i < MapDialogContainer::numLocalSSNs; i++ )
    {
      if(!(MapDialogContainer::boundLocalSSNs[MapDialogContainer::remInst[n]]&((uint64_t)1<<i)))
      {
        continue;
      }
      result = Et96MapUnbindReq(MapDialogContainer::localSSNs[i] INSTARG(MapDialogContainer::remInst[n]));
      if ( result != ET96MAP_E_OK)
      {
        __map_warn2__("error at Et96MapUnbindReq SSN=%d errcode 0x%hx",MapDialogContainer::localSSNs[i],result);
      }
    }
    MapDialogContainer::boundLocalSSNs[MapDialogContainer::remInst[n]]=0;
  }

  for(int i=0;i<MapDialogContainer::remInstCount;i++)
  {
    if(!MAP_connectedInst[i])
    {
      continue;
    }
    result = EINSS7CpMsgRelInst( MY_USER_ID, ETSIMAP_ID, MapDialogContainer::remInst[i]);
    if ( result != MSG_OK)
    {
      __map_warn2__("error at MsgRel errcode 0x%hx",result);
    }
  }

  for(int i=0;i<MapDialogContainer::remInstCount;i++)
  {
    MapDialogContainer::getInstance()->DropAllDialogs(MapDialogContainer::remInst[i]);
  }

#ifdef EIN_HD
  result = MsgClose(MY_USER_ID);
  if ( result != MSG_OK)
  {
    __map_warn2__("error at MsgClose errcode 0x%hx",result);
  }
  MsgExit();
#endif
}

void MapIoTask::deinit()
{
  USHORT_T result;
  __map_warn__("deinitialize MAP_PROXY");
  ReconnectThread::disconnect();
#ifndef EIN_HD
  result = MsgClose(MY_USER_ID);
  if ( result != MSG_OK)
  {
    __map_warn2__("error at MsgClose errcode 0x%hx",result);
  }
  MsgExit();
#endif
  MapDialogContainer::destroyInstance();
}

/*struct ReceiveGuard{
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
};*/


void MapIoTask::killOverflow()
{
  MSG_T message;
  EINSS7INSTANCE_T rinst=0;
  int idx=mapIoTaskCount;
  while(!isStopping)
  {
    {
      MutexGuard mg(monitors[idx]);
      MsgQueue& q=queues[idx];
      while(!isStopping && q.Count()==0)
      {
        monitors[idx].wait(1000);
      }
      if(isStopping || q.Count()==0)
      {
        continue;
      }
      q.Pop(message);
    }
    if(message.primitive==MAP_OPEN_IND)
    {
      ET96MAP_DIALOGUE_ID_T dlgId=((ET96MAP_DIALOGUE_ID_T)message.msg_p[2])|(((ET96MAP_DIALOGUE_ID_T)message.msg_p[3])<<8);
        //(message.msg_p[2]<<8)| message.msg_p[3];
      ET96MAP_LOCAL_SSN_T lssn=message.msg_p[1];
#ifdef EIN_HD
      rinst=message.remoteInstance;
#endif
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      warnMapReq( Et96MapOpenResp(lssn INSTARG(rinst),dlgId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0), __func__);
      warnMapReq( Et96MapCloseReq(lssn INSTARG(rinst),dlgId,ET96MAP_NORMAL_RELEASE,0,0,0), __func__);
      smsc::system::Smsc::getInstance().incRejected();
    }
    EINSS7CpReleaseMsgBuffer(&message);
  }
}

void MapIoTask::dispatcher(int idx)
{
  MSG_T message;
  USHORT_T result;
  EINSS7INSTANCE_T rinst=0;


  std::vector<MSG_T> tokill;
  tokill.reserve(120);

  for (;;)
  {
    if ( isStopping )
    {
      return;
    }

    DialogRefGuard dlg;
    {
      MutexGuard mg(monitors[idx]);
      MsgQueue& q=queues[idx];
      while(!isStopping && q.Count()==0)
      {
        monitors[idx].wait(1000);
      }
      if(isStopping || q.Count()==0)
      {
        continue;
      }
      q.Pop(message);
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
      __log2__(smsc::logger::_mapmsg_cat,smsc::logger::Logger::LEVEL_DEBUG, "MsgRecv[inst=%d] msg: %s",INSTARG0(message.remoteInstance), text);
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
    }else
    {
      handleMessage(message);
      EINSS7CpReleaseMsgBuffer(&message);
      continue;
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
    __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN, "error at Et96MapHandleIndication with code x%hx msg[inst=%d]: %s",map_result,INSTARG0(message.remoteInstance),text.get());
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
/*void MapIoTask::connect(unsigned)
{
  __map_warn__("MapIoTask::connect: no map stack on this platform");
}*/

bool MapIoTask::ReconnectThread::connect()
{
  return false;
}

EventMonitor MapIoTask::ReconnectThread::reconnectMon;

void MapIoTask::ReconnectThread::reportDisconnect(int rinst,bool needRel)
{

}

void MapIoTask::ReconnectThread::init(bool firstTime)
{

}

int MapIoTask::ReconnectThread::Execute()
{
  return 0;
}

MapDialogContainer::MapDialogContainer()
{

}

void MapIoTask::init(unsigned)
{
  __map_warn__("MapIoTask::init: no map stack on this platform");
}

/*void MapIoTask::disconnect()
{
  __map_warn__("MapIoTask::disconnect: no map stack on this platform");
}*/

void MapIoTask::deinit()
{
  __map_warn__("MapIoTask::deinit: no map stack on this platform");
}

void MapIoTask::killOverflow()
{

}

void MapIoTask::dispatcher(int idx)
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

void MapIoTask::StartMap()
{
  try
  {
    try
    {
      init();
    } catch (exception& e)
    {
      __map_warn2__("exception in mapio, restarting: %s",e.what());
      kill(getpid(),9);
    }
    tp.startTask(new ReconnectThread());
    do{
      sleep(1);
    }while(MAP_connectedInstCount==0 && !isStopping && !smsc::system::Smsc::getInstance().getStopFlag());

    is_started = true;
    __trace2__("signal mapiotask start:%p",startevent);
    startevent->SignalAll();
    if( isStopping || smsc::system::Smsc::getInstance().getStopFlag()) return;
    for(int i=0;i<mapIoTaskCount;i++)
    {
      tp.startTask(new DispatcherExecutor(this,i));
    }
    tp.startTask(new OverflowKiller(this));
    Thread::Start();
  } catch (exception& e) {
    __map_warn2__("exception in mapio: %s",e.what());
  }
}

int MapIoTask::Execute()
{
#ifdef USE_MAP
  MSG_T message;
  USHORT_T result;
  EINSS7INSTANCE_T rinst=0;
  int didx=0;
  int didxRoundRobin=0;
  message.receiver = MY_USER_ID;
  while(!isStopping)
  {
    while(!isStopping && MAP_connectedInstCount==0)
    {
      usleep(100);
    }
    result = EINSS7CpMsgRecv_r(&message,1000);
    if ( result != MSG_OK )
    {
      if( result != MSG_TIMEOUT )
      {
        __map_warn2__("Error at MsgRecv with code %d",result);
      }
      continue;
    }
    didx=255;
    if(message.primitive==MAP_BIND_CONF && message.primitive==MAP_STATE_IND)
    {
      didx=0;
    }else if(message.primitive!=MAP_GET_AC_VERSION_CONF)
    {
      ET96MAP_DIALOGUE_ID_T dlgId=((ET96MAP_DIALOGUE_ID_T)message.msg_p[2])|(((ET96MAP_DIALOGUE_ID_T)message.msg_p[3])<<8);
        //(message.msg_p[2]<<8)| message.msg_p[3];
      ET96MAP_LOCAL_SSN_T lssn=message.msg_p[1];
#ifdef EIN_HD
      rinst=message.remoteInstance;
#endif
      unsigned char* widxPtr=widxMap[rinst][lssn];
      didx=widxPtr?widxPtr[dlgId]:255;
      if(didx==255)
      {
        didx=didxRoundRobin++;
        if(didxRoundRobin>=mapIoTaskCount)
        {
          didxRoundRobin=0;
        }
      }
      if(widxPtr)
      {
        if(queues[didx].Count()>200 && message.primitive==MAP_OPEN_IND)
        {
          monitors[mapIoTaskCount].Lock();
          queues[mapIoTaskCount].Push(message);
          monitors[mapIoTaskCount].notify();
          monitors[mapIoTaskCount].Unlock();
          widxPtr[dlgId]=mapIoTaskCount;
          continue;
        }else
        {
          if(didx==mapIoTaskCount && message.primitive==MAP_DELIMIT_IND)
          {
            widxPtr[dlgId]=255;
          }else
          {
            widxPtr[dlgId]=didx;
          }
        }
      }
    }else
    {
      didx=didxRoundRobin++;
      if(didxRoundRobin>=mapIoTaskCount)
      {
        didxRoundRobin=0;
      }
    }
    monitors[didx].Lock();
    queues[didx].Push(message);
    monitors[didx].notify();
    monitors[didx].Unlock();
  }
#endif
  return 0;
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
uint64_t             MapDialogContainer::boundLocalSSNs[10];
uint64_t             MapDialogContainer::patternBoundLocalSSNs;

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

/*
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
 */

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
  MapDialogContainer::getInstance()->setPerformanceLimits( timeout, limit );
}


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
  "NEWDIALOG_INSRI",
  "DISPOSEDIALOG_INSRI",
  "NEWDIALOG_IN",
  "DISPOSEDIALOG_IN",
  "NEWDIALOG_OUTSRI",
  "DISPOSEDIALOG_OUTSRI",
  "NEWDIALOG_OUT",
  "DISPOSEDIALOG_OUT",
  "NEWDIALOG_USSD",
  "DISPOSEDIALOG_USSD",
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
  smsc_log_info(log, "open %d/%d:%d/%d:%d:%d, closed %d/%d:%d/%d:%d:%d, dlg %d/%d:%d/%d:%d:%d, rcv %d",
            MAPSTATS_open[0][idx],
            MAPSTATS_open[1][idx],
            MAPSTATS_open[2][idx],
            MAPSTATS_open[3][idx],
            MAPSTATS_open[4][idx],
            MAPSTATS_open[5][idx],

            MAPSTATS_close[0][idx],
            MAPSTATS_close[1][idx],
            MAPSTATS_close[2][idx],
            MAPSTATS_close[3][idx],
            MAPSTATS_close[4][idx],
            MAPSTATS_close[5][idx],

            MAPSTATS_dialogs[0],
            MAPSTATS_dialogs[1],
            MAPSTATS_dialogs[2],
            MAPSTATS_dialogs[3],
            MAPSTATS_dialogs[4],
            MAPSTATS_dialogs[5],

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
                  "updated %s dlg %d/%d:%d/%d:%d:%d",
                   MAPSTATS_types[stats],
                   MAPSTATS_dialogs[0],
                   MAPSTATS_dialogs[1],
                   MAPSTATS_dialogs[2],
                   MAPSTATS_dialogs[3],
                   MAPSTATS_dialogs[4],
                   MAPSTATS_dialogs[5]
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

bool isMapBound() {
#ifdef USE_MAP
  for(int n=0;n<MapDialogContainer::remInstCount;n++)
  {
    if(MapDialogContainer::boundLocalSSNs[MapDialogContainer::remInst[n]]==MapDialogContainer::patternBoundLocalSSNs)
    {
      return true;
    }
  }
  //return memcmp( MapDialogContainer::boundLocalSSNs, MapDialogContainer::patternBoundLocalSSNs, MapDialogContainer::numLocalSSNs*sizeof(int)) == 0 ;
#endif
  return false;
}

}
}
}
