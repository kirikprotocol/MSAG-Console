/*
$Id$
*/

#ifndef __header_MAPIO_h__
#define __header_MAPIO_h__

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>
#include "util/debug.h"
#include "../../util/smstext.h"
#include "../../sms/sms.h"
#include "../../smeman/smsccmd.h"
#include "../../smeman/smeman.h"
using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;

namespace smsc{
namespace logger{
extern smsc::logger::Logger* _map_cat;
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

#include "../../core/buffers/XHash.hpp"
#include "../../core/synchronization/Mutex.hpp"
#include "../../core/synchronization/Event.hpp"
#include "../../core/threads/ThreadedTask.hpp"

#include "MapProxy.h"

using smsc::system::mapio::MapProxy;

using smsc::core::buffers::XHash;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::Event;

extern "C" {
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

enum MAPSTATS{
  MAPSTATS_GSMDIALOG_OPENIN,
  MAPSTATS_GSMDIALOG_OPENOUT,
  MAPSTATS_GSMDIALOG_CLOSEIN,
  MAPSTATS_GSMDIALOG_CLOSEOUT,
  MAPSTATS_GSMRECV,
  MAPSTATS_NEWDIALOG_IN,
  MAPSTATS_DISPOSEDIALOG_IN,
  MAPSTATS_NEWDIALOG_OUT,
  MAPSTATS_DISPOSEDIALOG_OUT
};
extern int MAPSTATS_dialogs_in;
extern int MAPSTATS_dialogs_out;

class MapDialog;

extern void MAPSTATS_Restart();
extern void MAPSTATS_Update(MAPSTATS);
extern void MAPSTATS_DumpDialogLC(MapDialog*);
extern void MAPSTATS_DumpDialog(MapDialog*);
extern bool isMapBound();

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
  MAPST_MapNoticed = 50
};

class hash_func_ET96MAP_DID{
public:
  static inline unsigned CalcHash(unsigned id){
    return (unsigned)id;
  }
};

void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
//ET96MAP_DIALOGUE_ID_T allocateDialogueId();

static const unsigned MAX_DIALOGID_POOLED  = 8*2000+1;

/**
  \class MapDialog
*/
struct MapDialog{
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
  Mutex mutex;
  MapState state;
  ET96MAP_DIALOGUE_ID_T dialogid_map;
  unsigned dialogid_smsc;
  ET96MAP_INVOKE_ID_T invokeId;
  ET96MAP_INVOKE_ID_T origInvokeId;
  string abonent;
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
//  bool isMOreq;
//  unsigned dialogid_req;
  MapDialog(ET96MAP_DIALOGUE_ID_T dialogid,ET96MAP_LOCAL_SSN_T lssn,unsigned version=2) :
    isUSSD(false),
    mms(false),
    hasIndAddress(false),
    hasMwdStatus(false),
    wasDelivered(false),
    subscriberAbsent(false),
    memoryExceeded(false),
    hlrWasNotified(false),
    isQueryAbonentStatus(false),
    dropChain(false),
    id_opened(false),
    state(MAPST_START),
    dialogid_map(dialogid),
    dialogid_smsc(0),
    invokeId(0),
    origInvokeId(0),
    version(version),
    hlrVersion(0),
    associate(0),
    ssn(lssn),
    ussdSequence(0),
    ussdMrRef(0),
    routeErr(0),
    udhiRef(0),
    udhiMsgNum(0),
    udhiMsgCount(0),
    lockedAt(0),
    ref_count(1)
  {
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
  }
  virtual ~MapDialog(){
    require ( ref_count == 0 );
    require ( chain.size() == 0 );
//    if ( dialogid_smsc != 0 && dialogid_map != 0){
//      require(dialogid_map < MAX_DIALOGID_POOLED);
    if ( dialogid_map < MAX_DIALOGID_POOLED )
      freeDialogueId(dialogid_map);
    //}
    if ( associate ) associate->Release();
    associate = 0;
    MAPSTATS_DumpDialogLC(this);
  }
  Mutex& getMutex(){return mutex;}
  void Release(){
    unsigned x = 0;
    mutex.Lock();
    x = --ref_count;
    mutex.Unlock();
    if ( x == 0 ){
      delete this;
    }
  }

  MapDialog* AddRef(){MutexGuard g(mutex);++ref_count;return this;}

  void Clean() {
    MutexGuard g(mutex);
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
  static Mutex sync_object;
  Mutex sync;
  static MapProxy* proxy;
  time_t last_dump_time;
  XHash<unsigned,MapDialog*,hash_func_ET96MAP_DID> hash_;
  XHash<const string,MapDialog*,StringHashFunc> lock_map;
  list<unsigned> dialogId_pool;
  friend void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
  static string SC_ADRESS_VALUE;
  static string USSD_ADRESS_VALUE;
  static ET96MAP_LOCAL_SSN_T ussdSSN;
  int    processTimeout;
  int    processLimit;

  void Dump() {
    if ( time(0) < last_dump_time+60 ) return;
    last_dump_time = time(0);
    unsigned key;
    MapDialog* dlg;
    hash_.First();
    while(hash_.Next(key,dlg)) {
      MAPSTATS_DumpDialog(dlg);
    }
  }
public:
  static smsc::logger::Logger* loggerStatDlg;

  void setPerformanceLimits(int newTimeOut,int newProcLimit)
  {
    MutexGuard g(sync);
    __mapproxy_trace2__("Setting proccessing limits on map proxy timeout=%d, limit=%d", newTimeOut, newProcLimit);
    processTimeout=newTimeOut+newTimeOut/4;
    processLimit=newProcLimit;
  }

  static void destroyInstance(){
    MutexGuard g(sync_object);
    if ( container ) {
      delete container;
      container = 0;
    }
  }

  static MapDialogContainer* getInstance(){
    MutexGuard g(sync_object);
    if ( !container ) {
      loggerStatDlg = smsc::logger::Logger::getInstance("map.stat.dlg");
      container = new MapDialogContainer();
      container->last_dump_time = 0;
      for (unsigned n=1;n<MAX_DIALOGID_POOLED;++n){
        container->dialogId_pool.push_back(n);
      }
    }
    return container;
  }
  static string GetSCAdress() { return SC_ADRESS_VALUE; }
  static void SetSCAdress(const string& scAddr) { SC_ADRESS_VALUE = scAddr; }
  static string GetUSSDAdress() { return USSD_ADRESS_VALUE; }
  static void SetUSSDAdress(const string& scAddr) { USSD_ADRESS_VALUE = scAddr; }
  static ET96MAP_LOCAL_SSN_T GetUSSDSSN() { return ussdSSN; }
  static void SetUSSDSSN(int ssn) { ussdSSN = (ET96MAP_LOCAL_SSN_T)ssn; }
  static void setProxy( MapProxy* _proxy ) { proxy = _proxy; }
  MapProxy* getProxy() { return proxy; }

  static void dropInstance()
  {
    MutexGuard g(sync_object);
    if(container) delete container;
    container=NULL;
  }

  unsigned getDialogCount() {
    return hash_.Count();
  }

  unsigned getNumberOfDialogs()
  {
    return lock_map.Count();
  }


  MapDialog* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn){
    MutexGuard g(sync);
    MapDialog* dlg = 0;
    if ( hash_.Get(MKDID(dialogueid,lssn),dlg) ) {
      dlg->AddRef();
      return dlg;
    }
    else
    {
      return 0;
    }
  }

  MapDialog* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn/*,const char* abonent*/,unsigned version=2){
    MutexGuard g(sync);
    if( MAPSTATS_dialogs_in >= processLimit ) throw ProxyQueueLimitException(MAPSTATS_dialogs_in,processLimit);
    MAPSTATS_Update(MAPSTATS_NEWDIALOG_IN);
    MapDialog* dlg = new MapDialog(dialogueid,lssn,version);
    hash_.Insert(MKDID(dialogueid,lssn),dlg);
    __mapdlg_trace2__("created new dialog 0x%p for dialogid 0x%x",dlg,dialogueid);
    dlg->AddRef();
    return dlg;
  }

  MapDialog* createDialogImsiReq(ET96MAP_LOCAL_SSN_T lssn,MapDialog* associate){
    MutexGuard g(sync);
    if ( dialogId_pool.size() == 0 ) {
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MAPSTATS_Update(MAPSTATS_NEWDIALOG_OUT);
    MapDialog* dlg = new MapDialog(map_dialog,lssn,2);
    dialogId_pool.pop_front();
    hash_.Insert(MKDID(map_dialog,lssn),dlg);
    __mapdlg_trace2__("create new 'IMSI' dialog 0x%p for dialogid 0x%x",dlg,map_dialog);
    dlg->AddRef();
    dlg->associate = associate->AddRef();
    require ( dlg != associate );
    return dlg;
  }


  MapDialog* createOrAttachSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent, const SmscCommand& cmd){
    MutexGuard g(sync);
    if ( dialogId_pool.size() == 0 ) {
      smsc_log_warn(smsc::logger::_mapdlg_cat, "Dialog id POOL is empty" );
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    __mapdlg_trace2__("try to create SMSC dialog on abonent %s",abonent.c_str());
    if ( ( abonent.length() != 0 ) && lock_map.Exists(abonent) ) {
      MapDialog* item = lock_map[abonent];
      if ( item == 0 ) {
        __mapdlg_trace2__("dialog for abonent %s is not present!",abonent.c_str());
        throw runtime_error("MAP::createOrAttachSMSCDialog: has no dialog for abonent ");
      }
      // check if dialog is opened too long
      time_t curtime = time(NULL);
      if( curtime - item->lockedAt >= MAX_MT_LOCK_TIME ) {
        // drop locked dialog and all msg in chain, and create dialog as new.
        __warn2__(smsc::logger::_mapdlg_cat,"Dialog locked too long id=%x.",item->dialogid_map);
        for (;!item->chain.empty();item->chain.pop_front())
        {
    //drop chain elements
        }
        _dropDialog( item->dialogid_map, item->ssn );
      } else {
        if( item->sms.get() && item->sms.get()->hasBinProperty(Tag::SMSC_CONCATINFO) ) {
          // check if it's really next part of concatenated message
          if( !cmd->get_sms()->hasBinProperty(Tag::SMSC_CONCATINFO) ) {
            throw NextMMSPartWaiting("Waiting next part of concat message");
          }
          if( item->sms.get()->getConcatMsgRef() != cmd->get_sms()->getConcatMsgRef() ) {
            __mapdlg_trace2__("Waiting next part of other concat message: %d != %d",item->sms.get()->getConcatMsgRef(),cmd->get_sms()->getConcatMsgRef());
            throw NextMMSPartWaiting("Waiting next part of other concat message");
          }
          if( item->state == MAPST_WaitNextMMS ) {
            item->state = MAPST_SendNextMMS;
            item->dialogid_smsc = smsc_did;
            item->abonent = abonent;
            item->AddRef();
            item->lockedAt = time(NULL);
            return item;
          } else {
            __mapdlg_trace2__("add command to chain, size %d",item->chain.size());
            item->chain.push_back(cmd);
            return 0;
          }
        } else {
          if ( item->chain.size() > 25 ) {
            __mapdlg_trace2__("chain is vely long (%d)",item->chain.size());
            throw ChainIsVeryLong("chain is very long");
          }
          __mapdlg_trace2__("add command to chain, size %d",item->chain.size());
          item->chain.push_back(cmd);
          return 0;
        }
      }
    }
    if( MAPSTATS_dialogs_out >= processLimit ) throw ProxyQueueLimitException(MAPSTATS_dialogs_out,processLimit);
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MAPSTATS_Update(MAPSTATS_NEWDIALOG_OUT);
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dialogId_pool.pop_front();
    dlg->dialogid_smsc = smsc_did;
    dlg->abonent = abonent;
    hash_.Insert(MKDID(map_dialog,lssn),dlg);
    dlg->lockedAt = time(NULL);
    if ( abonent.length() != 0 ) lock_map.Insert(abonent,dlg);
    __mapdlg_trace2__("new dialog 0x%p for dialogid 0x%x/0x%x",dlg,smsc_did,map_dialog);
    dlg->AddRef();
    return dlg;
  }

  MapDialog* createOrAttachSMSCUSSDDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent, const SmscCommand& cmd){
    //if ( abonent.length() == 0 )
    //  throw runtime_error("MAP::createOrAttachSMSCDialog: can't create MT dialog without abonent");
    MutexGuard g(sync);
    if ( dialogId_pool.size() == 0 ) {
      smsc_log_warn(smsc::logger::_mapdlg_cat, "Dialog id POOL is empty" );
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    if( MAPSTATS_dialogs_out >= processLimit ) throw ProxyQueueLimitException(MAPSTATS_dialogs_out,processLimit);
    __mapdlg_trace__("try to create SMSC USSD dialog");
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MAPSTATS_Update(MAPSTATS_NEWDIALOG_OUT);
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dialogId_pool.pop_front();
    dlg->dialogid_smsc = smsc_did;
    dlg->abonent = abonent;
    hash_.Insert(MKDID(map_dialog,lssn),dlg);
    dlg->lockedAt = time(NULL);
    __mapdlg_trace2__("new USSD dialog 0x%p for dialogid 0x%x/0x%x",dlg,smsc_did,map_dialog);
    dlg->AddRef();
    return dlg;
  }

  USHORT_T reAssignDialog(unsigned did,unsigned oldssn,unsigned ssn){
    MutexGuard g(sync);
    MapDialog* dlg = 0;
    hash_.Get(MKDID(did,oldssn),dlg);
    if ( dlg == 0 ){
      __mapdlg_trace2__("couldn't reassign dialog, here is no did 0x%x",did);
      throw runtime_error("MAP:: reassign dialog: here is no did");
    }
    MAPSTATS_DumpDialogLC(dlg);
    if ( dialogId_pool.size() == 0 ) {
      smsc_log_warn(smsc::logger::_mapdlg_cat, "Dialog id POOL is empty" );
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    ET96MAP_DIALOGUE_ID_T dialogid_map = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    if ( did < MAX_DIALOGID_POOLED ) {
      MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_OUT);
      dialogId_pool.push_back(did);
    } else {
      MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_IN);
    }
    MAPSTATS_Update(MAPSTATS_NEWDIALOG_OUT);
    dlg->dialogid_map = dialogid_map;
    dlg->ssn = ssn;
    dialogId_pool.pop_front();
    hash_.Delete(MKDID(did,oldssn));
    hash_.Insert(MKDID(dialogid_map,ssn),dlg);
    __mapdlg_trace2__("dialog reassigned 0x%x->0x%x",did,dialogid_map);
    dlg->lockedAt = time(NULL);
    struct timeval tv;
    gettimeofday( &tv, 0 );
    dlg->maked_at_mks = ((long long)tv.tv_sec)*1000*1000 + (long long)tv.tv_usec;
    return dialogid_map;
  }

  void releaseDialog(MapDialog *dialog){
    MutexGuard g(sync);
    dialog->Release();
  }

  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid,unsigned ssn){
    MutexGuard g(sync);
    _dropDialog(dialogueid,ssn);
  }

  void _dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid,unsigned ssn){
    MapDialog* item = 0;
    if ( hash_.Get(MKDID(dialogueid,ssn),item) ){
      if ( item->abonent.length() != 0 ) {
        lock_map.Delete(item->abonent);
      }
      hash_.Delete(MKDID(dialogueid,ssn));
      item->state = MAPST_END;
      item->Release();
      if(dialogueid < MAX_DIALOGID_POOLED ) {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_OUT);
      } else {
        MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG_IN);
      }
    }else{
      __mapdlg_trace2__("has no dialog for dialogid 0x%x",dialogueid);
    }
  }
  void registerSelf(SmeManager* smeman);
  void unregisterSelf(SmeManager* smeman);
  void abort();
};

class DialogRefGuard{
  MapDialog* dialog;
  DialogRefGuard(const DialogRefGuard&);
public:
  DialogRefGuard(MapDialog* d = 0):dialog(d){/*d->AddRef();*/}
  ~DialogRefGuard(){
    if ( dialog ) {
      MapDialogContainer::getInstance()->releaseDialog(dialog);
    }
  }
  void assign(MapDialog* d){
    if ( dialog == d ) return;
    if ( dialog ) {
      MapDialogContainer::getInstance()->releaseDialog(dialog);
    }
    dialog = d;
    dialog->lockedAt = time(NULL);
  }
  bool isnull(){return dialog==0;}
  void forget(){dialog = 0;}
  MapDialog* operator->() { return dialog; }
  MapDialog* get() { return dialog; }
};


inline
void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId)
{
//  __mapdlg_trace2__("Dialogid 0x%x retuned to pool",dialogueId);
  MapDialogContainer::getInstance()->dialogId_pool.push_back(dialogueId);
}

class MapTracker : public ThreadedTask{
public:
  virtual int Execute();
  virtual const char* taskName() { return "MapTracker";}
};

/*
*/
class MapIoTask : public ThreadedTask{
public:
  virtual int Execute();
  virtual const char* taskName() { return "MapIoTask";}
  bool isStarted() {return is_started;}
  MapProxy proxy;
  MapIoTask(Event* startevent,const string& scAddr, const string& ussdCenterAddr, int ussdSSN) : startevent(startevent),is_started(false)
  {
    MapDialogContainer::SetSCAdress(scAddr);
    MapDialogContainer::SetUSSDAdress(ussdCenterAddr);
    MapDialogContainer::SetUSSDSSN(ussdSSN);
    MapDialogContainer::setProxy( &proxy );
  }
  ~MapIoTask() {
    MapDialogContainer::setProxy( 0 );
  }
private:
  Event* startevent;
  bool is_started;
  void dispatcher();
  void init(unsigned timeout=0);
  void deinit(bool connected);
};

#endif // __header_MAPIO_h__
