/*
$Id$
*/

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "sms/sms.h"
#include "smeman/smsccmd.h"
#include "smeman/smeman.h"
using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;

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

#define EINSS7_THREADSAFE 1

extern "C" {
#include <portss7.h>
#include <ss7tmc.h>
#include <ss7log.h>
#include <ss7msg.h>
#include <et96map_api_defines.h>
#include <et96map_api_types.h>
#include <et96map_dialogue.h>
#include <et96map_sms.h>
}

#define SSN 8

enum MapState{
  MAPST_UNKNOWN,
  MAPST_WaitHlrVersion,
  MAPST_SendingRInfo,
  MAPST_RInfoFallBack,
  MAPST_WaitRInfoConf,
  MAPST_WaitRInfoClose,
  MAPST_WaitMcsVersion,
//  MAPST_SendingSms,
  MAPST_WaitSpecOpenConf,
  MAPST_WaitOpenConf,
  MAPST_WaitSmsConf,
  MAPST_WaitSpecDelimeter,
  MAPST_WaitSmsClose,
  MAPST_WaitSms,
  MAPST_WaitSmsMOInd,
  MAPST_WaitSmsMODelimiter,
  MAPST_WaitSubmitCmdConf,
  MAPST_WaitSmsMODelimiter2,
  MAPST_START,
  MAPST_CLOSED,
  MAPST_ABORTED,
  MAPST_BROKEN
};

class hash_func_ET96MAP_DID{
public:
  static inline unsigned CalcHash(ET96MAP_DIALOGUE_ID_T id){
    return (unsigned)id;
  }
};

void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
//ET96MAP_DIALOGUE_ID_T allocateDialogueId();

/**
  \class MapDialog
*/
struct MapDialog{
  Mutex mutex;
  MapState state;
  ET96MAP_DIALOGUE_ID_T dialogid_map;
  unsigned dialogid_smsc;
  ET96MAP_INVOKE_ID_T invokeId;
  string abonent;
  auto_ptr<SMS> sms;
  auto_ptr<ET96MAP_SM_RP_UI_T> auto_ui;
//  ET96MAP_APP_CNTX_T appContext;
  ET96MAP_ADDRESS_T m_msAddr;	
  ET96MAP_ADDRESS_T m_scAddr;	
  ET96MAP_SS7_ADDR_T scAddr;
  ET96MAP_SS7_ADDR_T destMscAddr;
  ET96MAP_SS7_ADDR_T mshlrAddr;
 	ET96MAP_SM_RP_DA_T smRpDa;
  ET96MAP_SM_RP_OA_T smRpOa;
  unsigned version;
  MapDialog(ET96MAP_DIALOGUE_ID_T dialogid,ET96MAP_LOCAL_SSN_T lssn,unsigned version=2) : 
    ref_count(1),
    state(MAPST_START), 
    dialogid_map(dialogid),
    dialogid_smsc(0),
    version(version)
    {}
  virtual ~MapDialog(){
    __trace2__("MAP::Dialog::~MapDialog 0x%x(0x%x)",dialogid_map,dialogid_smsc);
    require ( ref_count == 0 );
    if ( dialogid_smsc != 0 && dialogid_map != 0){
      freeDialogueId(dialogid_map);
    }
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
  void AddRef(){MutexGuard g(mutex);++ref_count;}
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
  XHash<ET96MAP_DIALOGUE_ID_T,MapDialog*,hash_func_ET96MAP_DID> hash;
  XHash<const string,unsigned,StringHashFunc> lock_map;
  list<unsigned> dialogId_pool;
  friend void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
  //ET96MAP_DIALOGUE_ID_T allocateDialogueId();
public:
  static MapDialogContainer* getInstance(){
    MutexGuard g(sync_object);
    if ( !container ) container = new MapDialogContainer();
    for (unsigned n=1;n<8*200+1;++n){
      container->dialogId_pool.push_back(n);
    }
    __trace2__("MAP::access to container 0x%p",container);
    return container;
  }
  
  MapProxy* getProxy() {
    return &proxy;
  }
  
  MapDialog* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* dlg = 0;
    __trace2__("MAP:: find for dialogid 0x%x, result addr 0x%p",dialogueid,&dlg);
    if ( hash.Get(dialogueid,dlg) ) {
      __trace2__("MAP:: find dialog 0x%p for dialogid 0x%x",dlg,dialogueid);
      dlg->AddRef();
      return dlg;
    }
    else return 0;
  }
  
  MapDialog* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn/*,const char* abonent*/,unsigned version=2){
    MutexGuard g(sync);
    MapDialog* dlg = new MapDialog(dialogueid,lssn,version);
    hash.Insert(dialogueid,dlg);
    __trace2__("MAP:: new dialog 0x%p for dialogid 0x%x",dlg,dialogueid);
    dlg->AddRef();
    return dlg;
  }
  
  /*MapDialog* createSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent){
    if ( abonent.length() == 0 )
      throw runtime_error("can't create MT dialog without abonent");
    MutexGuard g(sync);
    __trace2__("MAP::createSMSCDialog: try create SMSC dialog on abonent %s",abonent.c_str());
    if ( lock_map.Exists(abonent) ) {
      __trace2__("MAP::createSMSCDialog: locked");
      return 0;
    }
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dialogId_pool.pop_front();
    dlg->dialogid_smsc = smsc_did;
    dlg->abonent = abonent;
    hash.Insert(map_dialog,dlg);
    lock_map.Insert(abonent,1);
    __trace2__("MAP:: new dialog 0x%p for dialogid 0x%x->0x%x",dlg,smsc_did,map_dialog);
    dlg->AddRef();
    return dlg;
  }*/

  MapDialog* createOrAttachSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const string& abonent){
    if ( abonent.length() == 0 )
      throw runtime_error("can't create MT dialog without abonent");
    MutexGuard g(sync);
    __trace2__("MAP::createSMSCDialog: try create SMSC dialog on abonent %s",abonent.c_str());
    if ( lock_map.Exists(abonent) ) {
      __trace2__("MAP::createSMSCDialog: locked");
      return 0;
    }
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dialogId_pool.pop_front();
    dlg->dialogid_smsc = smsc_did;
    dlg->abonent = abonent;
    hash.Insert(map_dialog,dlg);
    lock_map.Insert(abonent,1);
    __trace2__("MAP:: new dialog 0x%p for dialogid 0x%x->0x%x",dlg,smsc_did,map_dialog);
    dlg->AddRef();
    return dlg;
  }
  
  USHORT_T reAssignDialog(unsigned did){
    MutexGuard g(sync);
     __trace2__("MAP:: reassign dialog");
    MapDialog* dlg = 0;
    hash.Get(did,dlg);
    if ( dlg == 0 ){
      __trace2__("MAP:: reassign dialog: here is no did 0x%x",did);
      throw runtime_error("MAP:: reassign dialog: here is no did");
    }
    ET96MAP_DIALOGUE_ID_T dialogid_map = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    dialogId_pool.push_back(did);
    dlg->dialogid_map = dialogid_map;
    dialogId_pool.pop_front();
    hash.Delete(did);
    hash.Insert(dialogid_map,dlg);
    __trace2__("MAP:: reassign dialog 0x%x->0x%x",did,dialogid_map);
    return dialogid_map;
  }
  
  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* item = 0;
    if ( hash.Get(dialogueid,item) ){
      __trace2__("MAP:: drop dialog 0x%p for dialogid 0x%x",item,dialogueid);
      if ( item->abonent.length() != 0 ) lock_map.Delete(item->abonent);
      hash.Delete(dialogueid);
      item->Release();
    }else{
      __trace2__("MAP::dropDialog: has no dialog for dialogid 0x%x",dialogueid);
    }
  }
  
  void registerSelf(SmeManager* smeman);
};

inline
void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId)
{
  __trace2__("MAP::% dialogid 0x%x retuned to pool",__PRETTY_FUNCTION__,dialogueId);
  MapDialogContainer::getInstance()->dialogId_pool.push_back(dialogueId);
}
/*ET96MAP_DIALOGUE_ID_T allocateDialogueId()
{
}*/


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
  MapIoTask(Event* startevent) : startevent(startevent),is_started(false) {}
  ~MapIoTask() {deinit();}
private:
  Event* startevent;
  bool is_started;
  void dispatcher();
  void init(unsigned timeout=0);
  void deinit();
};


