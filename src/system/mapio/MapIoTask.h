/*
$Id$
*/

#ifndef __header_MAPIO_h__
#define __header_MAPIO_h__

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "../../util/debug.h"
#include "../../sms/sms.h"
#include "../../smeman/smsccmd.h"
#include "../../smeman/smeman.h"
using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;
namespace smsc{
namespace util{
extern log4cpp::Category* _map_cat;
extern log4cpp::Category* _mapdlg_cat;
};
};
#define __map_trace2__(format,args...) __debug2__(smsc::util::_map_cat,format,##args)
#define __map_trace__(text) __debug__(smsc::util::_map_cat,text)
#define __map_warn2__(format,args...) __warn2__(smsc::util::_map_cat,format,##args)
#define __map_warn__(text) __warn__(smsc::util::_map_cat,text)
#define __mapdlg_trace2__(format,args...) __debug2__(smsc::util::_mapdlg_cat,format,##args)
#define __mapdlg_trace__(text) __debug__(smsc::util::_mapdlg_cat,text)

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

#define EINSS7_THREADSAFE 1

extern "C" {
#include "inc/portss7.h"
#include "inc/ss7tmc.h"
#include "inc/ss7log.h"
#include "inc/ss7msg.h"
#include "inc/et96map_api_defines.h"
#include "inc/et96map_api_types.h"
#include "inc/et96map_dialogue.h"
#include "inc/et96map_sms.h"
#include "inc/et96map_ussd.h"
}

#define SSN 8
#define USSD_SSN 6
#define HLR_SSN 6

enum MAPSTATS{
  MAPSTATS_GSMDIALOG_CLOSE,
  MAPSTATS_GSMDIALOG_ABORT,
  MAPSTATS_GSMDIALOG_OPENIN,
  MAPSTATS_GSMDIALOG_OPENOUT,
  MAPSTATS_GSMDIALOG_CLOSEIN,
  MAPSTATS_GSMDIALOG_CLOSEOUT,
  MAPSTATS_GSMDIALOG_ABORTSELF,
  MAPSTATS_GSMDIALOG_APORTIN,
  MAPSTATS_GSMRECV,
  MAPSTATS_NEWDIALOG,
  MAPSTATS_DISPOSEDIALOG,
  MAPSTATS_REMAPDIALOG,
  MAPSTATS_REASSIGNDIALOG,
};
class MapDialog;

extern void MAPSTATS_Restart();
extern void MAPSTATS_Update(MAPSTATS);
extern void MAPSTATS_DumpDialogLC(MapDialog*);
extern void MAPSTATS_DumpDialog(MapDialog*);


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
  MAPST_WaitAlertDelimiter = 30
};

class hash_func_ET96MAP_DID{
public:
  static inline unsigned CalcHash(ET96MAP_DIALOGUE_ID_T id){
    return (unsigned)id;
  }
};

void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
//ET96MAP_DIALOGUE_ID_T allocateDialogueId();

static const unsigned MAX_DIALOGID_POOLED  = 8*200+1;

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
    ref_count(1),
    state(MAPST_START),
    dialogid_map(dialogid),
    dialogid_smsc(0),
    version(version),
    associate(0),
    ssn(lssn),
    ussdSequence(0),
    ussdMrRef(0)
//    isMOreq(false),
//    dialogid_req(0)
  {
    MAPSTATS_Update(MAPSTATS_NEWDIALOG);
    struct timeval tv;
    gettimeofday( &tv, 0 );
    maked_at_mks = ((long long)tv.tv_sec)*1000*1000 + (long long)tv.tv_usec;
  }
  virtual ~MapDialog(){
    __mapdlg_trace2__("~MapDialog 0x%x(0x%x)",dialogid_map,dialogid_smsc);
    require ( ref_count == 0 );
    require ( chain.size() == 0 );
//    if ( dialogid_smsc != 0 && dialogid_map != 0){
//      require(dialogid_map < MAX_DIALOGID_POOLED);
    if ( dialogid_map < MAX_DIALOGID_POOLED )
      freeDialogueId(dialogid_map);
    //}
    if ( associate ) associate->Release();
    associate = 0;
    MAPSTATS_Update(MAPSTATS_DISPOSEDIALOG);
    MAPSTATS_DumpDialogLC(this);
  }
  Mutex& getMutex(){return mutex;}
  void Release(){
    unsigned x = 0;
    mutex.Lock();
    x = --ref_count;
    mutex.Unlock();
    if ( ref_count == 0 ){
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

class DialogRefGuard{
  MapDialog* dialog;
public:
  DialogRefGuard(MapDialog* d = 0):dialog(d){/*d->AddRef();*/}
  ~DialogRefGuard(){if ( dialog ) dialog->Release();}
  void assign(MapDialog* d){
    if ( dialog == d ) return;
    if ( dialog ) dialog->Release();
    dialog = d;
  }
  bool isnull(){return dialog==0;}
  void forget(){dialog = 0;}
  MapDialog* operator->() { return dialog; }
  MapDialog* get() { return dialog; }
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
  MapProxy proxy;
  time_t last_dump_time;
  XHash<ET96MAP_DIALOGUE_ID_T,MapDialog*,hash_func_ET96MAP_DID> hash;
  XHash<const string,MapDialog*,StringHashFunc> lock_map;
  list<unsigned> dialogId_pool;
  friend void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
  static string SC_ADRESS_VALUE;
  //ET96MAP_DIALOGUE_ID_T allocateDialogueId();
  void Dump() {
    if ( time(0) < last_dump_time+60 ) return;
    last_dump_time = time(0);
    ET96MAP_DIALOGUE_ID_T key;
    MapDialog* dlg;
    hash.First();
    while(hash.Next(key,dlg)) {
      MAPSTATS_DumpDialog(dlg);
    }
  }
public:
  static MapDialogContainer* getInstance(){
    MutexGuard g(sync_object);
    if ( !container ) {
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
  static void dropInstance()
  {
    MutexGuard g(sync_object);
    if(container) delete container;
    container=NULL;
  }

  unsigned getDialogCount() {
    return hash.Count();
  }

  unsigned getNumberOfDialogs()
  {
    return lock_map.Count();
  }

  MapProxy* getProxy() {
    return &proxy;
  }

  MapDialog* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* dlg = 0;
    if ( hash.Get(dialogueid,dlg) ) {
      __mapdlg_trace2__("found dialog 0x%p for dialogid 0x%x",dlg,dialogueid);
      dlg->AddRef();
      return dlg;
    }
    else
    {
      __mapdlg_trace2__("dialog not found for dialogid 0x%x",dialogueid);
      return 0;
    }
  }

  MapDialog* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn/*,const char* abonent*/,unsigned version=2){
    MutexGuard g(sync);
    MapDialog* dlg = new MapDialog(dialogueid,lssn,version);
    hash.Insert(dialogueid,dlg);
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
    MapDialog* dlg = new MapDialog(map_dialog,lssn,2);
    dialogId_pool.pop_front();
    hash.Insert(map_dialog,dlg);
    __mapdlg_trace2__("create new 'IMSI' dialog 0x%p for dialogid 0x%x",dlg,map_dialog);
    dlg->AddRef();
    dlg->associate = associate->AddRef();
    require ( dlg != associate );
    return dlg;
  }

  MapDialog* createOrAttachSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent, const SmscCommand& cmd){
    //if ( abonent.length() == 0 )
    //  throw runtime_error("MAP::createOrAttachSMSCDialog: can't create MT dialog without abonent");
    MutexGuard g(sync);
    if ( dialogId_pool.size() == 0 ) {
      smsc::util::_mapdlg_cat->warn( "Dialog id POOL is empty" );
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    __mapdlg_trace2__("try to create SMSC dialog on abonent %s",abonent.c_str());
    if ( ( abonent.length() != 0 ) && lock_map.Exists(abonent) ) {
      MapDialog* item = lock_map[abonent];
      if ( item == 0 ) {
        __mapdlg_trace2__("dialog for abonent %s is not present!",abonent.c_str());
        throw runtime_error("MAP::createOrAttachSMSCDialog: has no dialog for abonent ");
      }else if ( item->chain.size() > 25 ) {
        __mapdlg_trace2__("chain is verly long (%d)",item->chain.size());
        throw runtime_error("chain is very long");
      }
      __mapdlg_trace2__("add command to chain, size %d",item->chain.size());
      item->chain.push_back(cmd);
      return 0;
    }
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dialogId_pool.pop_front();
    dlg->dialogid_smsc = smsc_did;
    dlg->abonent = abonent;
    hash.Insert(map_dialog,dlg);
    if ( abonent.length() != 0 ) lock_map.Insert(abonent,dlg);
    __mapdlg_trace2__("new dialog 0x%p for dialogid 0x%x/0x%x",dlg,smsc_did,map_dialog);
    dlg->AddRef();
    return dlg;
  }

  USHORT_T reAssignDialog(unsigned did,unsigned ssn){
    MAPSTATS_Update(MAPSTATS_REASSIGNDIALOG);
    MutexGuard g(sync);
    MapDialog* dlg = 0;
    hash.Get(did,dlg);
    if ( dlg == 0 ){
      __mapdlg_trace2__("couldn't reassign dialog, here is no did 0x%x",did);
      throw runtime_error("MAP:: reassign dialog: here is no did");
    }
    if ( dialogId_pool.size() == 0 ) {
      smsc::util::_mapdlg_cat->warn( "Dialog id POOL is empty" );
      Dump();
      throw runtime_error("MAP:: POOL is empty");
    }
    ET96MAP_DIALOGUE_ID_T dialogid_map = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    if ( did < MAX_DIALOGID_POOLED ) dialogId_pool.push_back(did);
    dlg->dialogid_map = dialogid_map;
    dlg->ssn = ssn;
    dialogId_pool.pop_front();
    hash.Delete(did);
    hash.Insert(dialogid_map,dlg);
    __mapdlg_trace2__("dialog reassigned 0x%x->0x%x",did,dialogid_map);
    return dialogid_map;
  }


  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* item = 0;
    if ( hash.Get(dialogueid,item) ){
      if ( item->abonent.length() != 0 ) {
        lock_map.Delete(item->abonent);
      }
      hash.Delete(dialogueid);
      item->state = MAPST_END;
      item->Release();
      __mapdlg_trace2__("dropped dialog 0x%p for dialogid 0x%x",item,dialogueid);
    }else{
      __mapdlg_trace2__("has no dialog for dialogid 0x%x",dialogueid);
    }
  }

  void registerSelf(SmeManager* smeman);
  void unregisterSelf(SmeManager* smeman);
  void abort();
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
  MapIoTask(Event* startevent,const string& scAddr) : startevent(startevent),is_started(false)
  {
    MapDialogContainer::SetSCAdress(scAddr);
  }
  ~MapIoTask() {deinit();}
private:
  Event* startevent;
  bool is_started;
  void dispatcher();
  void init(unsigned timeout=0);
  void deinit();
};

#endif // __header_MAPIO_h__
