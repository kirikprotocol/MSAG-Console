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

class hash_func_ET96MAP_DID{
public:
  static inline int CalcHash(ET96MAP_DIALOGUE_ID_T id){
    return (int)id;
  }
};

enum MapDialogState {
  MAPST_START,
  MAPST_WAIT_SUBMIT_RESPONSE
};


/**
  \class MapDialog
*/
class MapDialog{
  MapDialogState state;
  ET96MAP_DIALOGUE_ID_T dialogid;
  ET96MAP_INVOKE_ID_T invokeId;
  ET96MAP_LOCAL_SSN_T ssn;
public:
  MapDialog(ET96MAP_DIALOGUE_ID_T dialogid,
            ET96MAP_LOCAL_SSN_T lssn) : state(MAPST_START), dialogid(dialogid),ssn(lssn) {}
  virtual USHORT_T  Et96MapV2ForwardSmMOInd( 
    ET96MAP_LOCAL_SSN_T lssn, 
    ET96MAP_DIALOGUE_ID_T dialogId,
    ET96MAP_INVOKE_ID_T invokeId, 
    ET96MAP_SM_RP_DA_T* dstAddr, 
    ET96MAP_SM_RP_OA_T* srcAddr,  
    ET96MAP_SM_RP_UI_T* ud );
  void setInvokeId(ET96MAP_INVOKE_ID_T invokeId) {this->invokeId = invokeId;}
  // возвращает истину если это последнее сообщение в диалоге и далее диалог
  // должн быть закрыт
  bool ProcessCmd(SmscCommand& cmd);
};

/**
  \class MapDialogCntItem
*/
//struct MapDialogCntItem{
  //ET96MAP_DIALOGUE_ID_T dialogueId;
  //ET96MAP_LOCAL_SSN_T localSsn;
  //ET96MAP_INVOKE_ID_T invokeId;
  //auto_ptr<MapDialog> dialogue;
//};

extern ET96MAP_DIALOGUE_ID_T allocateDialogueId();
extern void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);

/**
  \class MapDialogContainer
*/
class MapDialogContainer{
  static MapDialogContainer* container;
  static Mutex sync_object;
  MapProxy proxy;
  XHash<ET96MAP_DIALOGUE_ID_T,MapDialog*,hash_func_ET96MAP_DID> hash;
  
public:
  static MapDialogContainer* getInstance(){
    MutexGuard g(sync_object);
    if ( !container ) container = new MapDialogContainer();
    return container;
  }
  
  MapProxy* getProxy() {
    return &proxy;
  }
  
  MapDialog* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MapDialog* dlg = 0;
    if ( hash.Get(dialogueid,dlg) ) return dlg;
    else return 0;
  }
  
  MapDialog* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn){
    MapDialog* dlg = new MapDialog(dialogueid,lssn);
    hash.Insert(dialogueid,dlg);
    return dlg;
  }
  
  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MapDialog* item = 0;
    if ( hash.Get(dialogueid,item) ){
      hash.Delete(dialogueid);
      delete item;
    }
  }
  
  void registerSelf(SmeManager* smeman){
    proxy.init();
    __trace2__("MAP::register MAP_PROXY");
    smeman->registerSmeProxy("MAP_PROXY",&proxy);
    __trace2__("MAP::register MAP_PROXY OK");
  }
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
  void init();
  void deinit();
};


