/*
$Id$
*/

#include <stdio.h>
#include <malloc.h>

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

/**
  \class MapDialog
*/
class MapDialog{
public:
  virtual USHORT_T Et96MapV2ForwardSmReq(
    ET96MAP_LOCAL_SSN_T localSsn,
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_SM_RP_DA_T *smRpDa_sp,
    ET96MAP_SM_RP_OA_T *smRpOa_sp,
    ET96MAP_SM_RP_UI_T *smRpUi_sp,
    ET96MAP_MMS_T moreMsgsToSend);
  
  virtual USHORT_T  Et96MapV2ForwardSmMOInd( 
    ET96MAP_LOCAL_SSN_T lssn, 
    ET96MAP_DIALOGUE_ID_T dialogId,
    ET96MAP_INVOKE_ID_T invokeId, 
    ET96MAP_SM_RP_DA_T* dstAddr, 
    ET96MAP_SM_RP_OA_T* srcAddr,  
    ET96MAP_SM_RP_UI_T* ud );
};

/**
  \class MapDialogCntItem
*/
struct MapDialogCntItem{
  ET96MAP_DIALOGUE_ID_T dialogueId;
  ET96MAP_LOCAL_SSN_T localSsn;
  ET96MAP_INVOKE_ID_T invokeId;
  MapDialog* dialogue;
};

extern ET96MAP_DIALOGUE_ID_T allocateDialogueId();
extern void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);

/**
  \class MapDialogContainer
*/
class MapDialogContainer{
  static MapDialogContainer* container;
  static Mutex sync_object;
  MapProxy proxy;
  XHash<ET96MAP_DIALOGUE_ID_T,MapDialogCntItem*,hash_func_ET96MAP_DID> hash;
  
public:
  static MapDialogContainer* getInstance(){
    MutexGuard g(sync_object);
    if ( !container ) container = new MapDialogContainer();
    return container;
  }
  MapProxy* MapGetProxy() {
    return &proxy;
  }
  MapDialogCntItem* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MapDialogCntItem* item = 0;
    if ( hash.Get(dialogueid,item) ) return item;
    else return 0;
  }
  MapDialogCntItem* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MapDialogCntItem* itm = new MapDialogCntItem();
    itm->dialogueId = dialogueid;
    hash.Insert(dialogueid,itm);
    return itm;
  }
  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MapDialogCntItem* item = 0;
    if ( hash.Get(dialogueid,item) ){
      hash.Delete(dialogueid);
      delete item;
    }
  }
};

/*
*/
class MapIoTask : public ThreadedTask{
public:
  virtual int Execute(){
    try{
      init();
      startevent->Signal();
      is_started = true;
      dispatcher();
      //deinit();
    }catch(std::exception& e){
      __trace2__("exception in mapio: %s",e.what());
    }
  }
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


