#ifndef __header_MAPIO_h__
#define __header_MAPIO_h__

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include "util/debug.h"
#include "util/smstext.h"
#include "sms/sms.h"
#include "smeman/smsccmd.h"
#include "smeman/smeman.h"
#include "core/synchronization/RecursiveMutex.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "MapLimits.hpp"

using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;

namespace smsc{
namespace logger{
extern smsc::logger::Logger* _map_cat;
extern smsc::logger::Logger* _mapmsg_cat;
extern smsc::logger::Logger* _mapdlg_cat;
}
}

using namespace smsc::util;
#define __map_trace2__(format,...) __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_DEBUG, format,__VA_ARGS__)
#define __map_trace__(text) __debug__(smsc::logger::_map_cat,text)
#define __map_warn2__(format,...) __log2__(smsc::logger::_map_cat,smsc::logger::Logger::LEVEL_WARN,format,__VA_ARGS__)
#define __map_warn__(text) __warn__(smsc::logger::_map_cat,text)
#define __mapdlg_trace2__(format,...) __log2__(smsc::logger::_mapdlg_cat,smsc::logger::Logger::LEVEL_DEBUG,format,__VA_ARGS__)
#define __mapdlg_trace__(text) __debug__(smsc::logger::_mapdlg_cat,text)

#include "core/buffers/XHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/threads/ThreadedTask.hpp"

#include "MapProxy.h"

using smsc::system::mapio::MapProxy;

using smsc::core::buffers::XHash;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::Event;

#ifdef USE_MAP

extern "C" {
#include <ss7cp.h>
#include <portss7.h>
#include <ss7tmc.h>
#include <ss7log.h>
#include <ss7msg.h>
#include <et96map_api_defines.h>
#include <et96map_api_types.h>
#include <et96map_dialogue.h>
#include <et96map_sms.h>
#include <et96map_ussd.h>
}

#define SSN 8
#define USSD_SSN (MapDialogContainer::GetUSSDSSN())
#define HLR_SSN 6
#define MAX_MT_LOCK_TIME 600

#endif

extern bool MAP_disconnectDetected;
extern Mutex mapMutex;

enum MAPSTATS{
  MAPSTATS_GSMRECV = 0,
  MAPSTATS_NEWDIALOG_IN,
  MAPSTATS_DISPOSEDIALOG_IN,
  MAPSTATS_NEWDIALOG_INSRI,
  MAPSTATS_DISPOSEDIALOG_INSRI,
  MAPSTATS_NEWDIALOG_USSD,
  MAPSTATS_DISPOSEDIALOG_USSD,
  MAPSTATS_NEWDIALOG_OUT,
  MAPSTATS_DISPOSEDIALOG_OUT,
  MAPSTATS_NEWDIALOG_NIUSSD,
  MAPSTATS_DISPOSEDIALOG_NIUSSD
};

enum {
  MAPSTAT_DLGIN,
  MAPSTAT_DLGINSRI,
  MAPSTAT_DLGUSSD,
  MAPSTAT_DLGOUT,
  MAPSTAT_DLGNIUSSD,

  MAPSTATS_COUNT
};

extern int MAPSTATS_dialogs[];

class MapDialog;

extern void MAPSTATS_Restart();
extern void MAPSTATS_Update(MAPSTATS);
extern void MAPSTATS_DumpDialogLC(MapDialog*);
extern void MAPSTATS_DumpDialog(MapDialog*,time_t,bool);
extern bool isMapBound();
extern void AbortMapDialog(unsigned dialogid,unsigned ssn);

enum MapState{
  MAPST_UNKNOWN = 0,
  MAPST_WaitHlrVersion = 1,
//  MAPST_SendingRInfo,
  MAPST_RInfoFallBack = 2,
  MAPST_WaitRInfoConf = 3,
  MAPST_WaitRInfoClose = 4,
  MAPST_WaitMcsVersion = 5,
//  MAPST_SendingSms,
  MAPST_WaitSpecOpenConf = 6,
  MAPST_WaitOpenConf = 7,
  MAPST_WaitSmsConf = 8,
  MAPST_WaitSpecDelimeter = 9,
  MAPST_WaitSmsClose = 10,
  MAPST_WaitSms = 11,
  MAPST_WaitSmsMOInd = 12,
  MAPST_WaitSmsMODelimiter = 13,
  MAPST_WaitSubmitCmdConf = 14,
  MAPST_WaitSmsMODelimiter2 = 15,
  MAPST_START = 16,
  MAPST_CLOSED = 17,
  MAPST_ABORTED = 18,
  MAPST_BROKEN = 19,
  MAPST_END = 20,
  MAPST_ImsiWaitACVersion = 21,
  MAPST_ImsiWaitOpenConf = 22,
  MAPST_ImsiWaitRInfo = 23,
  MAPST_ImsiWaitCloseInd = 24,
  MAPST_WaitImsiReq = 25,
  MAPST_USSDWaitResponce = 26,
  MAPST_WaitDelRepConf = 27,
  MAPST_MTFINAL = 28,
  MAPST_WaitDelClose = 29,
  MAPST_WaitAlertDelimiter = 30,
  MAPST_WaitNextMMS = 31,
  MAPST_SendNextMMS = 32,
  MAPST_WaitFwdMOOpenConf = 33,
  MAPST_WaitFwdMOConf = 34,
  MAPST_WaitUSSDReqOpenConf = 35,
  MAPST_WaitUSSDReqConf = 36,
  MAPST_WaitUSSDReqDelim = 37,
  MAPST_WaitUSSDReqClose = 38,
  MAPST_WaitUSSDNotifyOpenConf = 39,
  MAPST_WaitUSSDNotifyConf = 40,
  MAPST_WaitUSSDNotifyClose = 41,
  MAPST_WaitUSSDNotifyCloseErr = 42,
  MAPST_WaitSubmitUSSDRequestConf = 43,
  MAPST_WaitSubmitUSSDRequestCloseConf = 44,
  MAPST_WaitSubmitUSSDNotifyConf = 45,
  MAPST_WaitSubmitUSSDNotifyCloseConf = 46,
  MAPST_ReadyNextUSSDCmd = 47,
  MAPST_WaitUssdDelimiter = 48,
  MAPST_WaitUssdImsiReq = 49,
  MAPST_MapNoticed = 50,
  MAPST_WaitUssdV1Delimiter = 51,
  MAPST_WaitUSSDErrorClose = 52
};

class hash_func_ET96MAP_DID{
public:
  static inline unsigned CalcHash(unsigned id){
    return (unsigned)id;
  }
};

void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
//ET96MAP_DIALOGUE_ID_T allocateDialogueId();

static const unsigned MAX_DIALOGID_POOLED  = 32*1024-1;


/*
struct DialogCommand{
  MSG_T msg;
  unsigned char buffer[1024];
  DialogCommand()
  {
    msg.size=0;
  }
  DialogCommand(const MSG_T& argMsg)
  {
    msg=argMsg;
    msg.msg_p=buffer;
    msg.remoteInstance=0;
    msg.cp_p=0;
    memcpy(msg.msg_p,argMsg.msg_p,msg.size);
  }
  DialogCommand(const DialogCommand& rhs)
  {
    msg=rhs.msg;
    msg.msg_p=buffer;
    memcpy(msg.msg_p,rhs.msg.msg_p,msg.size);
  }
};
*/


/**
  \class MapDialog
*/
struct MapDialog{
  Mutex mutex;
  bool isUSSD:1;
  bool mms:1;
  bool hasIndAddress:1;
  bool hasMwdStatus:1;
  bool wasDelivered:1;
  bool subscriberAbsent:1;
  bool memoryExceeded:1;
  bool hlrWasNotified:1;
  bool isQueryAbonentStatus:1;
  bool dropChain:1;
  bool id_opened:1;
  bool isLocked:1;
  bool isAllocated:1;
  bool isDropping:1;
  bool needReportMsc:1;
  bool ussdProcessing:1;
  MapState state;
  int dlgType;
  ET96MAP_DIALOGUE_ID_T dialogid_map;
  unsigned dialogid_smsc;
  ET96MAP_INVOKE_ID_T invokeId;
  ET96MAP_INVOKE_ID_T origInvokeId;
  string abonent;
  string subsystem;
  string origAddress;
  auto_ptr<SMS> sms;
  auto_ptr<ET96MAP_SM_RP_UI_T> auto_ui;
  ET96MAP_ADDRESS_T m_msAddr;
  ET96MAP_ADDRESS_T m_scAddr;
  ET96MAP_SS7_ADDR_T scAddr;
  ET96MAP_SS7_ADDR_T destMscAddr;
  ET96MAP_SS7_ADDR_T mshlrAddr;
  ET96MAP_SM_RP_DA_T smRpDa;
  ET96MAP_SM_RP_OA_T smRpOa;
  list<SmscCommand> chain;
  unsigned version;
  unsigned hlrVersion;
  string s_imsi;
  string s_msc;
  MapDialog* associate;
  ET96MAP_LOCAL_SSN_T ssn;
  long long ussdSequence;
  unsigned ussdMrRef;
  ET96MAP_MWD_STATUS_T mwdStatus;
  unsigned routeErr;
  SmscCommand QueryAbonentCommand;
  unsigned udhiRef;
  unsigned udhiMsgNum;
  unsigned udhiMsgCount;
  long long maked_at_mks;
  time_t   lockedAt;
  smsc::core::buffers::CyclicQueue<MSG_T> cmdQueue;
  pthread_cond_t condVar;

  MapDialog()
  {
    InitDialog(0,0,0);
  }
  MapDialog(ET96MAP_DIALOGUE_ID_T dialogid,ET96MAP_LOCAL_SSN_T lssn,unsigned argVersion=2)
  {
    InitDialog(dialogid,lssn,argVersion);
  }

  void InitDialog(ET96MAP_DIALOGUE_ID_T dialogid,ET96MAP_LOCAL_SSN_T lssn,unsigned argVersion=2)
  {
    dlgType=MAPSTAT_DLGOUT;
    isUSSD=false;
    mms=false;
    hasIndAddress=false;
    hasMwdStatus=false;
    wasDelivered=false;
    subscriberAbsent=false;
    memoryExceeded=false;
    hlrWasNotified=false;
    isQueryAbonentStatus=false;
    dropChain=false;
    id_opened=false;
    ussdProcessing=false;
    state=MAPST_START;
    dialogid_map=dialogid;
    dialogid_smsc=0;
    invokeId=0;
    origInvokeId=0;
    version=argVersion;
    hlrVersion=0;
    associate=0;
    ssn=lssn;
    ussdSequence=0;
    ussdMrRef=0;
    routeErr=0;
    udhiRef=0;
    udhiMsgNum=0;
    udhiMsgCount=0;
    lockedAt=0;
    ref_count=1;
    isLocked=false;
    isAllocated=false;
    isDropping=false;
    needReportMsc=false;
    memset(&m_msAddr, 0, sizeof(ET96MAP_ADDRESS_T));
    memset(&m_scAddr, 0, sizeof(ET96MAP_ADDRESS_T));
    memset(&scAddr, 0, sizeof(ET96MAP_SS7_ADDR_T));
    memset(&destMscAddr, 0, sizeof(ET96MAP_SS7_ADDR_T));
    memset(&mshlrAddr,0,sizeof(ET96MAP_SS7_ADDR_T));
    memset(&smRpDa,0,sizeof(ET96MAP_SM_RP_DA_T));
    memset(&smRpOa,0,sizeof(ET96MAP_SM_RP_OA_T));
    memset(&mwdStatus,0,sizeof(ET96MAP_MWD_STATUS_T));
    struct timeval tv;
    gettimeofday( &tv, 0 );
    maked_at_mks = ((long long)tv.tv_sec)*1000*1000 + (long long)tv.tv_usec;
    __require__(cmdQueue.Count()==0);
    chain.clear();
    sms = auto_ptr<SMS>(0);
    auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(0);
    pthread_cond_init(&condVar,0);
  }
/*
  void CopyFrom(MapDialog& dlg)
  {
    isUSSD=dlg.isUSSD;
    mms=dlg.mms;
    hasIndAddress=dlg.hasIndAddress;
    hasMwdStatus=dlg.hasMwdStatus;
    wasDelivered=dlg.wasDelivered;
    subscriberAbsent=dlg.subscriberAbsent;
    memoryExceeded=dlg.memoryExceeded;
    hlrWasNotified=dlg.hlrWasNotified;
    isQueryAbonentStatus=dlg.isQueryAbonentStatus;
    dropChain=dlg.dropChain;
    id_opened=dlg.id_opened;
    isLocked=dlg.isLocked;
    isAllocated=dlg.isAllocated;
    state=dlg.state;
    dialogid_map=dlg.dialogid_map;
    dialogid_smsc=dlg.dialogid_smsc;
    invokeId=dlg.invokeId;
    origInvokeId=dlg.origInvokeId;
    abonent=dlg.abonent;
    subsystem=dlg.subsystem;
    sms=dlg.sms;
    auto_ui=dlg.auto_ui;
    m_msAddr=dlg.m_msAddr;
    m_scAddr=dlg.m_scAddr;
    scAddr=dlg.scAddr;
    destMscAddr=dlg.destMscAddr;
    mshlrAddr=dlg.mshlrAddr;
    smRpDa=dlg.smRpDa;
    smRpOa=dlg.smRpOa;
    chain=dlg.chain;
    version=dlg.version;
    hlrVersion=dlg.hlrVersion;
    s_imsi=dlg.s_imsi;
    s_msc=dlg.s_msc;
    associate=dlg.associate;
    ssn=dlg.ssn;
    ussdSequence=dlg.ussdSequence;
    ussdMrRef=dlg.ussdMrRef;
    mwdStatus=dlg.mwdStatus;
    routeErr=dlg.routeErr;
    QueryAbonentCommand=dlg.QueryAbonentCommand;
    udhiRef=dlg.udhiRef;
    udhiMsgNum=dlg.udhiMsgNum;
    udhiMsgCount=dlg.udhiMsgCount;
    maked_at_mks=dlg.maked_at_mks;
    lockedAt=dlg.lockedAt;
    cmdQueue=dlg.cmdQueue;
  }
  */

  virtual ~MapDialog()
  {
    require ( ref_count == 0 );
    require ( chain.size() == 0 );
  }

  void Release()
  {
    unsigned x = 0;
    mutex.Lock();
    unsigned dlgid=0xffffffff;
    MapDialog* ass=0;
    ref_count--;
    //__map_trace2__("release: dlgId=0x%x, refcnt=%d",dialogid_map,ref_count);
    if ( ref_count == 0 )
    {
      ass=associate;
      dlgid=dialogid_map;
      associate = 0;
      MAPSTATS_DumpDialogLC(this);
      sms = auto_ptr<SMS>(0);
      auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(0);
      chain.clear();
      isLocked=false;
      while(cmdQueue.Count())
      {
        EINSS7CpReleaseMsgBuffer(&cmdQueue.Front());
        cmdQueue.Pop();
      }
      cmdQueue.Clear();
      isAllocated=false;
    }
    mutex.Unlock();
    if(ass)
    {
      ass->Release();
    }
    if ( dlgid < MAX_DIALOGID_POOLED )
    {
      freeDialogueId(dlgid);
    }
  }

  MapDialog* AddRef(){
    MutexGuard g(mutex);
    ++ref_count;
    //__map_trace2__("addref: dlgId=0x%x, refcnt=%d",dialogid_map,ref_count);
    return this;
  }

  MapDialog* AddRefUnlocked()
  {
    ++ref_count;
    //__map_trace2__("addref: dlgId=0x%x, refcnt=%d",dialogid_map,ref_count);
    return this;
  }

  MapDialog* AddRefIfAllocated()
  {
    MutexGuard g(mutex);
    if(!isAllocated)return 0;
    ++ref_count;
    //__map_trace2__("addref: dlgId=0x%x, refcnt=%d",dialogid_map,ref_count);
    return this;
  }

  void AssignSms(SMS* argSms)
  {
    MutexGuard g(mutex);
    sms.reset(argSms);
  }

  void Clean()
  {
    state = MAPST_START;
    //dialogid_map = 0;
    //dialogid_smsc;
    invokeId = 0;
    //string abonent;
    sms = auto_ptr<SMS>(0);
    auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(0);
    //ET96MAP_ADDRESS_T m_msAddr;
    //ET96MAP_ADDRESS_T m_scAddr;
    //ET96MAP_SS7_ADDR_T scAddr;
    //ET96MAP_SS7_ADDR_T destMscAddr;
    //ET96MAP_SS7_ADDR_T mshlrAddr;
    //ET96MAP_SM_RP_DA_T smRpDa;
    //ET96MAP_SM_RP_OA_T smRpOa;
    //list<SmscCommand> chain;
    version = 0;
    mms = false;
    isUSSD = false;
    //isAllocated=false;
  }

private:
  unsigned ref_count;
};

inline unsigned
MKDID(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn)
{
  return
    ((unsigned(dialogueid)&0xffff)|(unsigned(lssn)<<24))
    & 0xff00ffff;
}

struct ChainIsVeryLong : public runtime_error {
  ChainIsVeryLong(const char* s) : runtime_error(s) {}
};

struct NextMMSPartWaiting : public runtime_error {
  NextMMSPartWaiting(const char* s) : runtime_error(s) {}
};
/**
  \class MapDialogContainer
*/
class MapDialogContainer{
  struct StringHashFunc{
    static inline int CalcHash(const string& key){
      const unsigned char* curr = (const unsigned char*)key.c_str();
      unsigned count = *curr;
      while(*curr){count += 37 * count + *curr;curr++;}
      count=(unsigned)(( ( count * (unsigned)19L ) + (unsigned)12451L ) % (unsigned)8882693L);
      return count;
    }
  };
  static MapDialogContainer* container;
  RecursiveMutex sync;
  static MapProxy* proxy;
  time_t last_dump_time;

  MapDialog** dlgPool[256];
  uint32_t dialogsCount;

  XHash<const string,MapDialog*,StringHashFunc> lock_map;
  list<unsigned> dialogId_pool;
  friend void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
  static string SC_ADRESS_VALUE;
  static string USSD_ADRESS_VALUE;
  static ET96MAP_LOCAL_SSN_T ussdSSN;
  static int busyMTDelay;
  static int lockedByMoDelay;
  static int MOLockTimeout;
  static bool allowCallBarred;
  static bool ussdV1Enabled;
  static bool ussdV1UseOrigEntityNumber;
  static int nodeNumber;
  static int nodesCount;

  int    processTimeout;
  int    processLimit;

  MapDialogContainer()
  {
    memset(dlgPool,0,sizeof(dlgPool));
    last_dump_time=0;
    dialogsCount=0;
  }

  void Dump()
  {
    static smsc::logger::Logger* dlogger = smsc::logger::Logger::getInstance("map.stat.dlgdump");
    if ( time(0) < last_dump_time+60 ) return;
    time_t now = time(NULL);
    last_dump_time = now;
    MapDialog* dlg;
    for(int j=0;j<numLocalSSNs;j++)
    for(int i=0;i<65536;i++)
    {
      dlg=dlgPool[localSSNs[j]][i];
      if(!dlg->isAllocated)continue;
      if((now-(dlg->maked_at_mks/1000000))>processTimeout*2)
      {
        MAPSTATS_DumpDialog(dlg, now, true);
        AbortMapDialog(dlg->dialogid_map, dlg->ssn);
        dropDialogUnlocked(dlg);
        dlg->Release();
      } else {
        MAPSTATS_DumpDialog(dlg, now, false);
      }
    }
  }

public:
  static ET96MAP_LOCAL_SSN_T *localSSNs;
  static int                *boundLocalSSNs;
  static int                *patternBoundLocalSSNs; /* contains true values, used for memcmp to check if all SSN bound*/
  static int                 numLocalSSNs;
  static smsc::logger::Logger* loggerStatDlg;
  static smsc::logger::Logger* loggerMapPdu;

  void setPerformanceLimits(int newTimeOut,int newProcLimit)
  {
//    MutexGuard g(sync);
    __mapproxy_trace2__("Setting proccessing limits on map proxy timeout=%d, limit=%d", newTimeOut, newProcLimit);
    processTimeout=newTimeOut+newTimeOut/4;
    processLimit=newProcLimit;
  }

  void InitLSSN(uint8_t lssn)
  {
    if(dlgPool[lssn])return;
    dlgPool[lssn]=new MapDialog*[65536];
    for(int i=0;i<65536;i++)
    {
      dlgPool[lssn][i]=new MapDialog();
    }
  }

  /**
   * need for statistics
   **/
/*
   void mapPacketReceived()
   {
 //    MutexGuard g(sync);
     MAPSTATS_Update(MAPSTATS_GSMRECV);
   }
*/

  /**
   * need for statistics
   **/
  void restartStatistics()
  {
    MutexGuard g(sync);
    MAPSTATS_Restart();
  }

  static void destroyInstance()
  {
//    MutexGuard g(sync_object);
    if ( container ) {
      delete container;
      container = 0;
    }
  }

  void DropAllDialogs()
  {
    MutexGuard g(sync);
    __mapdlg_trace2__("%s: dropping all dialogs",__func__);
    for(int j=0;j<numLocalSSNs;j++)
    {
      for(int i=0;i<65536;i++)
      {
        if(!dlgPool[localSSNs[j]])continue;
        MapDialog* dlg=dlgPool[localSSNs[j]][i];
        if(!dlg->isAllocated)continue;
        dropDialogUnlocked(dlg);
        dlg->Release();
      }
    }
  }

  static MapDialogContainer* getInstance(){
//    MutexGuard g(sync_object);
    if ( !container )
    {
      loggerStatDlg = smsc::logger::Logger::getInstance("map.stat.dlg");
      loggerMapPdu = smsc::logger::Logger::getInstance("map.pdu");
      container = new MapDialogContainer();
      for (unsigned n=1+nodeNumber;n<MAX_DIALOGID_POOLED;n+=nodesCount){
        container->dialogId_pool.push_back(n);
      }
    }
    return container;
  }
  static int GetNodeNumber() { return nodeNumber; }
  static void SetNodeNumber(const int n) { nodeNumber = n; }
  static int GetNodesCount() { return nodesCount; }
  static void SetNodesCount(const int n) { nodesCount = n; }
  static string GetSCAdress() { return SC_ADRESS_VALUE; }
  static void SetSCAdress(const string& scAddr) { SC_ADRESS_VALUE = scAddr; }
  static string GetUSSDAdress() { return USSD_ADRESS_VALUE; }
  static void SetUSSDAdress(const string& scAddr) { USSD_ADRESS_VALUE = scAddr; }
  static ET96MAP_LOCAL_SSN_T GetUSSDSSN() { return ussdSSN; }
  static void SetUSSDSSN(int ssn, const string& addUssdSSN)
  {
    ussdSSN = (ET96MAP_LOCAL_SSN_T)ssn;
    int addssns = 0;
    if( addUssdSSN.length() > 0 ) {
      const char *str = addUssdSSN.c_str();
      for( int i = 0; str[i]; i++ ) if( str[i] == ',' ) addssns++;
      addssns++;
    }
    __log2__(smsc::logger::_mapdlg_cat,smsc::logger::Logger::LEVEL_INFO,"%s: initializing local SSNs additional %d",__func__, addssns);
    localSSNs = new ET96MAP_LOCAL_SSN_T[addssns+2];
    boundLocalSSNs = new int[addssns+2];
    patternBoundLocalSSNs = new int[addssns+2];
    numLocalSSNs = addssns+2;
    const char *str = addUssdSSN.c_str();
    localSSNs[0] = SSN;
    localSSNs[1] = ussdSSN;
    int i = 0;
    int idx = 2;
    int lssn = 0;
    while(*str && sscanf(str, "%d%n,",&lssn, &i) == 1 )
    {
      localSSNs[idx++] = lssn;
      __log2__(smsc::logger::_mapdlg_cat,smsc::logger::Logger::LEVEL_INFO,"%s:initializing additional SSN %d",__func__, lssn);
      str+=i+(str[i]==0?0:1);
    }
  }
  static void setProxy( MapProxy* _proxy ) { proxy = _proxy; }
  MapProxy* getProxy() { return proxy; }
  static void setBusyMTDelay(int val){busyMTDelay=val;}
  static int  getBusyMTDelay(){return busyMTDelay;}
  static void setLockedByMoDelay(int val){lockedByMoDelay=val;}
  static int  getLockedByMoDelay(){return lockedByMoDelay;}
  static void setMOLockTimeout(int val){MOLockTimeout=val;}
  static int  getMOLockTimeout(){return MOLockTimeout;}
  static void setAllowCallBarred(bool val){allowCallBarred=val;}
  static bool  getAllowCallBarred(){return allowCallBarred;}
  static void setUssdV1Enabled(bool val){ussdV1Enabled=val;}
  static bool  getUssdV1Enabled(){return ussdV1Enabled;}
  static void setUssdV1UseOrigEntityNumber(bool val){ussdV1UseOrigEntityNumber=val;}
  static bool  getUssdV1UseOrigEntityNumber(){return ussdV1UseOrigEntityNumber;}

  static void dropInstance()
  {
//    MutexGuard g(sync_object);
    if(container) delete container;
    container=NULL;
  }

  unsigned getDialogCount() {
//    MutexGuard g(sync);
    return dialogsCount;
  }

  unsigned getNumberOfDialogs()
  {
    MutexGuard g(sync);
    return lock_map.Count();
  }

  MapDialog* newDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn,unsigned version=2)
  {
    if(!dlgPool[lssn])
    {
      throw Exception("DialogContainer: unsupported lssn:%d",lssn);
    }
    MapDialog* dlg=dlgPool[lssn][dialogueid];
    MutexGuard mg(dlg->mutex);
    if(dlg->isAllocated)
    {
      throw Exception("DialogContainer: attempt to allocate already allocated dialog:%d/%d",dialogueid,lssn);
    }
    dlg->InitDialog(dialogueid,lssn,version);
    dlg->isAllocated=true;
    return dlg->AddRefUnlocked();
  }

  MapDialog* newLockedDialogAndRef(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn,unsigned version,int dlgType)
  {
    if(!dlgPool[lssn])
    {
      throw Exception("DialogContainer: unsupported lssn:%d",lssn);
    }
    MapDialog* dlg=dlgPool[lssn][dialogueid];
    MutexGuard mg(dlg->mutex);
    if(dlg->isAllocated)
    {
      throw Exception("DialogContainer: attempt to allocate already allocated dialog:%d/%d",dialogueid,lssn);
    }
    dlg->InitDialog(dialogueid,lssn,version);
    dlg->isAllocated=true;
    dlg->isLocked=true;
    dlg->dlgType=dlgType;
    return dlg->AddRefUnlocked();
  }

  MapDialog* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn)
  {
    if(dlgPool[lssn]==0)
    {
      throw Exception("Unsupported ssn:%d",lssn);
    }
    return dlgPool[lssn][dialogueid]->AddRefIfAllocated();
  }

  MapDialog* getLockedDialogOrEnqueue(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn,MSG_T& msg)
  {
    {
      MutexGuard g(sync);
      MAPSTATS_Update(MAPSTATS_GSMRECV);
    }
    if(dlgPool[lssn]==0)
    {
      EINSS7CpReleaseMsgBuffer(&msg);
      throw Exception("Unsupported ssn:%d",lssn);
    }
    MapDialog* dlg=dlgPool[lssn][dialogueid];
    MutexGuard mg(dlg->mutex);
    if(!dlg->isAllocated)
    {
      __map_warn2__("Failed to get dialog for prim=0x%x,dlgId=0x%x,lssn=%u",(unsigned int)msg.primitive,(unsigned int)dialogueid,(unsigned int)lssn);
      EINSS7CpReleaseMsgBuffer(&msg);
      return 0;
    }
    if(dlg->isDropping)
    {
      EINSS7CpReleaseMsgBuffer(&msg);
      return 0;
    }
    if(dlg->isLocked)
    {
      dlg->cmdQueue.Push(msg);
      return 0;
    }
    dlg->isLocked=true;
    return dlg->AddRefUnlocked();
  }

  MapDialog* createLockedDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn,unsigned version,bool isUSSD)
  {
    using smsc::system::mapio::MapLimits;
    if(isUSSD)
    {
      if( MAPSTATS_dialogs[MAPSTAT_DLGUSSD] >= MapLimits::getInstance().getLimitUSSD() )
      {
        MutexGuard g(sync);
        Dump();
        throw ProxyQueueLimitException(MAPSTATS_dialogs[MAPSTAT_DLGUSSD] , MapLimits::getInstance().getLimitUSSD());
      }
    }else
    {
      if( MAPSTATS_dialogs[MAPSTAT_DLGIN] >= MapLimits::getInstance().getLimitIn() )
      {
        MutexGuard g(sync);
        Dump();
        throw ProxyQueueLimitException(MAPSTATS_dialogs[MAPSTAT_DLGIN] , MapLimits::getInstance().getLimitIn());
      }
    }
    if( dialogueid <= MAX_DIALOGID_POOLED )
    {
      __warn2__(smsc::logger::_mapdlg_cat,"Dialog form SS7 network has too low ID 0x%x.",dialogueid);
      throw ProxyQueueLimitException(dialogueid , MAX_DIALOGID_POOLED);
    }
    {
      MutexGuard g(sync);
      MAPSTATS_Update(isUSSD?MAPSTATS_NEWDIALOG_USSD:MAPSTATS_NEWDIALOG_IN);
      MAPSTATS_Update(MAPSTATS_GSMRECV);
    }
    __mapdlg_trace2__("created new dialog for dialogid 0x%x",dialogueid);
    try
    {
      return newLockedDialogAndRef(dialogueid,lssn,version,isUSSD?MAPSTAT_DLGUSSD:MAPSTAT_DLGIN);
    } catch(std::exception& e)
    {
      MutexGuard g(sync);
      MAPSTATS_Update(isUSSD?MAPSTATS_DISPOSEDIALOG_USSD:MAPSTATS_DISPOSEDIALOG_IN);
      throw;
    }
  }

  MapDialog* createDialogImsiReq(ET96MAP_LOCAL_SSN_T lssn,MapDialog* associate)
  {
    using smsc::system::mapio::MapLimits;
    if(MAPSTATS_dialogs[MAPSTAT_DLGINSRI]>=MapLimits::getInstance().getLimitInSRI())
    {
      MutexGuard g(sync);
      Dump();
      throw ProxyQueueLimitException(MAPSTATS_dialogs[MAPSTAT_DLGINSRI],MapLimits::getInstance().getLimitInSRI());
    }
    MutexGuard g(sync);
    if ( dialogId_pool.empty() ) {
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    try{
      ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
      MAPSTATS_Update(MAPSTATS_NEWDIALOG_INSRI);
      MapDialog* dlg=newDialog(map_dialog,lssn,2);
      dlg->dlgType=MAPSTAT_DLGINSRI;
      dialogId_pool.pop_front();
      __mapdlg_trace2__("create new 'IMSI' dialog 0x%p for dialogid 0x%x",dlg,map_dialog);
      require ( dlg != associate );
      dlg->associate = associate->AddRef();
      return dlg;
    }catch(...)
    {
      MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_INSRI);
      throw;
    }
  }


  MapDialog* createOrAttachSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent, const SmscCommand& cmd)
  {
    MapDialog* item=0;
    MutexGuard g(sync);
    {
      if ( dialogId_pool.empty() )
      {
        smsc_log_warn(smsc::logger::_mapdlg_cat, "Dialog id POOL is empty" );
        Dump();
        throw runtime_error("MAP:: POOL is empty");
      }
      __mapdlg_trace2__("try to create SMSC dialog on abonent %s",abonent.c_str());
      if (abonent.length() != 0)
      {
        MapDialog** itemptr=lock_map.GetPtr(abonent);
        if(itemptr)
        {
          item = *itemptr;
        }
      }
    }
    if(item)
    {
      // check if dialog is opened too long
      time_t curtime = time(NULL);
      if( curtime - item->lockedAt >= MAX_MT_LOCK_TIME )
      {
        // drop locked dialog and all msg in chain, and create dialog as new.
        __warn2__(smsc::logger::_mapdlg_cat,"Dialog locked too long id=%x.",item->dialogid_map);
        dropDialogUnlocked(item);
        item->Release();
      } else
      {
        MutexGuard mg(item->mutex);
        if(!item->isDropping)//this dialog is currently processed by dropmapdialog function in map state machine
        {
          if( item->sms.get() && item->sms.get()->hasBinProperty(Tag::SMSC_CONCATINFO) )
          {
            // check if it's really next part of concatenated message
            if( !cmd->get_sms()->hasBinProperty(Tag::SMSC_CONCATINFO) )
            {
              throw NextMMSPartWaiting("Waiting next part of concat message");
            }
            if( item->sms.get()->getConcatMsgRef() != cmd->get_sms()->getConcatMsgRef() )
            {
              __mapdlg_trace2__("Waiting next part of other concat message: %d != %d",item->sms.get()->getConcatMsgRef(),cmd->get_sms()->getConcatMsgRef());
              throw NextMMSPartWaiting("Waiting next part of other concat message");
            }
            if( item->state == MAPST_WaitNextMMS )
            {
              item->state = MAPST_SendNextMMS;
              item->dialogid_smsc = smsc_did;
              item->abonent = abonent;
              item->lockedAt = time(NULL);
              return item->AddRefUnlocked();
            } else
            {
              __mapdlg_trace2__("add command to chain of dlg=0x%x, st=%d, size %d",item->dialogid_map,item->state,item->chain.size());
              item->chain.push_back(cmd);
              return 0;
            }
          } else
          {
            if ( item->chain.size() > 25 )
            {
              __mapdlg_trace2__("chain is vely long in dlg 0x%x (%d)",item->dialogid_map,item->chain.size());
              throw ChainIsVeryLong("chain is very long");
            }
            __mapdlg_trace2__("add command to chain of dlg 0x%x, size %d",item->dialogid_map,item->chain.size());
            item->chain.push_back(cmd);
            return 0;
          }
        }
      }
    }
    using smsc::system::mapio::MapLimits;
    if( MAPSTATS_dialogs[MAPSTAT_DLGOUT] >= MapLimits::getInstance().getLimitOut() )
    {
//      MutexGuard g(sync);
      Dump();
      throw ProxyQueueLimitException(MAPSTATS_dialogs[MAPSTAT_DLGOUT] , MapLimits::getInstance().getLimitOut());
    }
//    MutexGuard g(sync);
    try{
      ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
      MAPSTATS_Update(MAPSTATS_NEWDIALOG_OUT);
      MapDialog* dlg=newDialog(map_dialog,lssn);
      dlg->dlgType=MAPSTAT_DLGOUT;
      dialogId_pool.pop_front();
      dlg->dialogid_smsc = smsc_did;
      dlg->abonent = abonent;
      dlg->lockedAt = time(NULL);
      if ( abonent.length() != 0 )
      {
        lock_map.Insert(abonent,dlg);
      }
      __mapdlg_trace2__("new dialog 0x%p for dialogid 0x%x/0x%x",dlg,smsc_did,map_dialog);
      return dlg;
    }
    catch(...)
    {
      MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_OUT);
      throw;
    }
  }

  MapDialog* createOrAttachSMSCUSSDDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent, const SmscCommand& cmd)
  {
    ET96MAP_DIALOGUE_ID_T map_dialog;
    {
      MutexGuard g(sync);
      if ( dialogId_pool.empty() )
      {
        smsc_log_warn(smsc::logger::_mapdlg_cat, "Dialog id POOL is empty" );
        Dump();
        throw runtime_error("MAP:: POOL is empty");
      }
      using smsc::system::mapio::MapLimits;
      if( MAPSTATS_dialogs[MAPSTAT_DLGNIUSSD] >= MapLimits::getInstance().getLimitNIUSSD() )
      {
        Dump();
        throw ProxyQueueLimitException(MAPSTATS_dialogs[MAPSTAT_DLGNIUSSD] , MapLimits::getInstance().getLimitNIUSSD());
      }
      map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
      dialogId_pool.pop_front();
      MAPSTATS_Update(MAPSTATS_NEWDIALOG_NIUSSD);
    }
    MapDialog* dlg;
    try{
      dlg = newDialog(map_dialog,lssn);
      dlg->dlgType=MAPSTAT_DLGNIUSSD;
      dlg->dialogid_smsc = smsc_did;
      dlg->abonent = abonent;
      dlg->lockedAt = time(NULL);
      __mapdlg_trace2__("new USSD dialog 0x%p for dialogid 0x%x/0x%x",dlg,smsc_did,map_dialog);
    }catch(...)
    {
      MutexGuard g(sync);
      dialogId_pool.push_back(map_dialog);
      MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_NIUSSD);
      throw;
    }
    return dlg;
  }

  USHORT_T reAssignDialog(unsigned did,unsigned oldssn,unsigned ssn)
  {
    MapDialog* dlg = 0;
    if(dlgPool[oldssn]!=0)
    {
      dlg = dlgPool[oldssn][did];
      bool isAllocated;
      dlg->mutex.Lock();
      isAllocated=dlg->isAllocated;
      dlg->mutex.Unlock();
      if ( !isAllocated )
      {
        dlg = 0;
      }
    }
    if ( dlg == 0 )
    {
      throw Exception("MAP:: reassign dialog: there is no dialog for did=0x%x/%x",did,oldssn);
    }
    if(!dlgPool[ssn])
    {
      throw Exception("Unsupported ssn:%d",ssn);
    }
    ET96MAP_DIALOGUE_ID_T dialogid_map;
    {
      MutexGuard g(sync);
      MAPSTATS_DumpDialogLC(dlg);
      if ( dialogId_pool.empty() )
      {
        smsc_log_warn(smsc::logger::_mapdlg_cat, "Dialog id POOL is empty" );
        Dump();
        throw runtime_error("MAP:: POOL is empty");
      }
      dialogid_map = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
      dialogId_pool.pop_front();
      if ( did < MAX_DIALOGID_POOLED )
      {
        if(dlg->dlgType==MAPSTAT_DLGOUT)
        {
          MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_OUT);
        }else if(dlg->dlgType==MAPSTAT_DLGNIUSSD)
        {
          MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_NIUSSD);
        }else
        if(dlg->dlgType==MAPSTAT_DLGINSRI)
        {
          MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_INSRI);
        }else
        {
          smsc_log_warn(smsc::logger::_mapdlg_cat,"unexpected dlgType=%d for out dlg",dlg->dlgType);
        }
        dialogId_pool.push_back(did);
      } else
      {
        if(dlg->dlgType==MAPSTAT_DLGIN)
        {
          MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_IN);
        }else
        if(dlg->dlgType==MAPSTAT_DLGUSSD)
        {
          MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_USSD);
        }else
        {
          __map_warn2__("unexpected dlgType=%d for in dlg",dlg->dlgType);
        }
      }

      MAPSTATS_Update(MAPSTATS_NEWDIALOG_OUT);
      {
        MapDialog* swpdlg=dlgPool[ssn][dialogid_map];
        MutexGuard swpmg(swpdlg->mutex);
        MutexGuard dlgmg(dlg->mutex);
        if(swpdlg->isAllocated)
        {
//          MutexGuard g(sync);
          dialogId_pool.push_back(dialogid_map);
          MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_OUT);
          throw Exception("Attempt to reassign to allocated dlg:%d->%d",did,dialogid_map);
        }
        dlgPool[oldssn][did]=swpdlg;
        dlgPool[ssn][dialogid_map]=dlg;
        dlg->dlgType = MAPSTAT_DLGOUT;
        dlg->dialogid_map = dialogid_map;
        dlg->ssn = ssn;
        dlg->lockedAt = time(NULL);
        struct timeval tv;
        gettimeofday( &tv, 0 );
        dlg->maked_at_mks = ((long long)tv.tv_sec)*1000*1000 + (long long)tv.tv_usec;
      }
    }
    __mapdlg_trace2__("dialog reassigned 0x%x->0x%x",did,dialogid_map);
    return dialogid_map;
  }

  void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId)
  {
    MutexGuard g(sync);
    dialogId_pool.push_back(dialogueId);
  }

  void dropDialog(MapDialog* item)
  {
    {
      MutexGuard g(sync);
      dropDialogUnlocked(item);
    }
    item->Release();
  }

  void dropDialogUnlocked(MapDialog* item)
  {
    if ( item->abonent.length() != 0 && !item->isUSSD)
    {
      MapDialog** dlgptr=lock_map.GetPtr(item->abonent);
      if(dlgptr && *dlgptr==item)
      {
        lock_map.Delete(item->abonent);
      }
    }
    item->state = MAPST_END;
    if(item->dialogid_map < MAX_DIALOGID_POOLED )
    {
      if(item->dlgType==MAPSTAT_DLGOUT)
      {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_OUT);
      }else if(item->dlgType==MAPSTAT_DLGNIUSSD)
      {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_NIUSSD);
      }else
      if(item->dlgType==MAPSTAT_DLGINSRI)
      {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_INSRI);
      }else
      {
        __map_warn2__("unexpected dlgType=%d for out dlg",item->dlgType);
      }
    } else
    {
      if(item->dlgType==MAPSTAT_DLGIN)
      {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_IN);
      }else
      if(item->dlgType==MAPSTAT_DLGUSSD)
      {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_USSD);
      }else
      {
        __map_warn2__("unexpected dlgType=%d for in dlg",item->dlgType);
      }
    }
    __mapdlg_trace2__("drop dialog(%d) 0x%x",item->dlgType,item->dialogid_map);
  }

  void registerSelf(SmeManager* smeman);
  void unregisterSelf(SmeManager* smeman);
  void abort();
};

class DialogRefGuard{
  MapDialog* dialog;
  DialogRefGuard(const DialogRefGuard&);
  void operator==(const DialogRefGuard&);
public:
  DialogRefGuard(MapDialog* d = 0):dialog(d){/*d->AddRef();*/}
  ~DialogRefGuard()
  {
    if(dialog)
    {
      dialog->Release();
    }
  }
  void assign(MapDialog* d){
    if ( dialog == d )
    {
      return;
    }
    if ( dialog )
    {
      dialog->Release();
    }
    dialog = d;
    dialog->lockedAt = time(NULL);
  }
  bool isnull()
  {
    return dialog==0;
  }
  void forget()
  {
    dialog = 0;
  }
  MapDialog* operator->()
  {
    return dialog;
  }
  MapDialog* get()
  {
    return dialog;
  }
};


inline
void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId)
{
//  __mapdlg_trace2__("Dialogid 0x%x retuned to pool",dialogueId);
  MapDialogContainer::getInstance()->freeDialogueId(dialogueId);
}

class MapTracker : public ThreadedTask{
public:
  virtual int Execute();
  virtual const char* taskName() { return "MapTracker";}
};

/*
*/
class MapIoTask{
public:

  bool isStarted() {return is_started;}
  MapProxy proxy;
  MapIoTask(Event* startevent,const string& scAddr, const string& ussdCenterAddr, 
            int ussdSSN, const string& addUssdSSN, 
            int busyMTDelay, int lockedByMODelay, int MOLockTimeout, 
            bool allowCallBarred, bool ussdV1Enabled, bool ussdV1UseOrigEntityNumber,
            int  nodeNumber, int nodesCount) : startevent(startevent)
  {
    MapDialogContainer::SetNodeNumber( nodeNumber );
    MapDialogContainer::SetNodesCount( nodesCount );
    MapDialogContainer::SetSCAdress(scAddr);
    MapDialogContainer::SetUSSDAdress(ussdCenterAddr);
    MapDialogContainer::SetUSSDSSN(ussdSSN, addUssdSSN);
    MapDialogContainer::setProxy( &proxy );
    MapDialogContainer::setBusyMTDelay(busyMTDelay);
    MapDialogContainer::setLockedByMoDelay(lockedByMODelay);
    MapDialogContainer::setMOLockTimeout(MOLockTimeout);
    MapDialogContainer::setAllowCallBarred(allowCallBarred);
    MapDialogContainer::setUssdV1Enabled(ussdV1Enabled);
    MapDialogContainer::setUssdV1UseOrigEntityNumber(ussdV1UseOrigEntityNumber);
    mapIoTaskCount=1;
    is_started=false;
    isStopping=false;
  }
  void Start();
  void setMapIoTaskCount(int newcnt)
  {
    mapIoTaskCount=newcnt;
  }
  void Stop()
  {
    isStopping=true;
    tp.Wait();
    deinit(true);
  }
  ~MapIoTask()
  {
    __mapdlg_trace__("Destroying MapIoTask");
    MapDialogContainer::setProxy( 0 );
  }
protected:
  class DispatcherExecutor;
  friend class MapIoTask::DispatcherExecutor;
  class DispatcherExecutor:public smsc::core::threads::ThreadedTask{
  public:
    DispatcherExecutor(MapIoTask* mit):mapIoTask(mit)
    {
    }
    virtual int Execute()
    {
      while(!mapIoTask->isStopping)
      {
        try{
          mapIoTask->dispatcher();
        }catch(std::exception& e)
        {
          __map_warn2__("Exception in dispatcher:'%s'",e.what());
        }
      }
      return 0;
    }
    virtual void stop()
    {
      mapIoTask->isStopping=true;
    }
    virtual const char* taskName() { return "MapIoTask::dispatcher";}
  protected:
    MapIoTask* mapIoTask;
  };

private:
  int mapIoTaskCount;
  smsc::core::threads::ThreadPool tp;
  Event* startevent;
  EventMonitor receiveMon;
  bool isStopping;
  bool is_started;
  void dispatcher();
  void init(unsigned timeout=0);
  void connect(unsigned timeout=0);
  void deinit(bool connected);
  void disconnect();

  void handleMessage(MSG_T& message);
};

#endif // __header_MAPIO_h__
