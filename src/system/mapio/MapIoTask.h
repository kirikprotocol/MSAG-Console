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

class hash_func_ET96MAP_DID{
public:
  static inline int CalcHash(ET96MAP_DIALOGUE_ID_T id){
    return (int)id;
  }
};

enum MapDialogState {
  MAPST_START,
  MAPST_WAIT_SUBMIT_RESPONSE,
  MAPST_READY_FOR_SENDSMS,
  MAPST_WAIT_RINFO,
  MAPST_RINFOIND
};

extern void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);


/**
  \class MapDialog
*/
class MapDialog{
  MapDialogState state;
  ET96MAP_DIALOGUE_ID_T dialogid;
  unsigned smscDialogId;
  ET96MAP_INVOKE_ID_T invokeId;
  ET96MAP_LOCAL_SSN_T ssn;
  auto_ptr<SMS> sms;
  ET96MAP_APP_CNTX_T appContext;
  ET96MAP_ADDRESS_T m_msaddr;	
  ET96MAP_ADDRESS_T m_scaddr;	
  ET96MAP_SS7_ADDR_T scAddr;
  ET96MAP_SS7_ADDR_T destMscAddr;
  ET96MAP_SS7_ADDR_T mshlrAddr;
 	ET96MAP_SM_RP_DA_T smRpDa;
public:
  MapDialog(ET96MAP_DIALOGUE_ID_T dialogid,
    ET96MAP_LOCAL_SSN_T lssn) : state(MAPST_START), dialogid(dialogid),ssn(lssn), smscDialogId(0) 
    {}
  virtual ~MapDialog(){
    if ( smscDialogId != 0 ){
      freeDialogueId(dialogid);
    }
  }
  virtual USHORT_T  Et96MapV2SendRInfoForSmConf ( 
    ET96MAP_LOCAL_SSN_T localSsn,
		ET96MAP_DIALOGUE_ID_T dialogueId,
		ET96MAP_INVOKE_ID_T invokeId,
		ET96MAP_IMSI_T *imsi_sp,
		ET96MAP_ADDRESS_T *mscNumber_sp,
		ET96MAP_LMSI_T *lmsi_sp,
		ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
		ET96MAP_PROV_ERR_T *provErrCode_p );
  virtual USHORT_T  Et96MapV2ForwardSmMOInd( 
    ET96MAP_LOCAL_SSN_T lssn, 
    ET96MAP_DIALOGUE_ID_T dialogId,
    ET96MAP_INVOKE_ID_T invokeId, 
    ET96MAP_SM_RP_DA_T* dstAddr, 
    ET96MAP_SM_RP_OA_T* srcAddr,  
    ET96MAP_SM_RP_UI_T* ud );
  virtual bool  Et96MapCloseInd(
    ET96MAP_LOCAL_SSN_T ssn,
    ET96MAP_DIALOGUE_ID_T did,
    ET96MAP_USERDATA_T *ud,
    UCHAR_T priorityOrder);
  void setInvokeId(ET96MAP_INVOKE_ID_T invokeId) {this->invokeId = invokeId;}
  // возвращает истину если это последнее сообщение в диалоге и далее диалог
  // должн быть закрыт
  virtual bool ProcessCmd(const SmscCommand& cmd);
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

/**
  \class MapDialogContainer
*/
class MapDialogContainer{
  static MapDialogContainer* container;
  static Mutex sync_object;
  Mutex sync;
  MapProxy proxy;
  XHash<ET96MAP_DIALOGUE_ID_T,MapDialog*,hash_func_ET96MAP_DID> hash;
  list<unsigned> dialogId_pool;
  friend void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
public:
  static MapDialogContainer* getInstance(){
    MutexGuard g(sync_object);
    if ( !container ) container = new MapDialogContainer();
    for (unsigned n=1;n<8*200+1;++n){
      container->dialogId_pool.push_back(n);
    }
    __trace2__("MAP::access to container 0x%x",container);
    return container;
  }
  
  MapProxy* getProxy() {
    return &proxy;
  }
  
  MapDialog* getDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* dlg = 0;
    __trace2__("MAP:: find for dialogid 0x%x, result addr 0x%x",dialogueid,&dlg);
    if ( hash.Get(dialogueid,dlg) ) {
      __trace2__("MAP:: find dialog 0x%x for dialogid 0x%x",dlg,dialogueid);
      return dlg;
    }
    else return 0;
  }
  
  MapDialog* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn){
    MutexGuard g(sync);
    MapDialog* dlg = new MapDialog(dialogueid,lssn);
    hash.Insert(dialogueid,dlg);
    __trace2__("MAP:: new dialog 0x%x for dialogid 0x%x",dlg,dialogueid);
    return dlg;
  }
  
  MapDialog* createSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn){
    MutexGuard g(sync);
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dlg->smscDialogId = smsc_did;
    hash.Insert(map_did,dlg);
    __trace2__("MAP:: new dialog 0x%x for dialogid 0x%x",dlg,dialogueid);
    dialogId_pool.pop_front();
    return dlg;
  }
  
  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* item = 0;
    if ( hash.Get(dialogueid,item) ){
      __trace2__("MAP:: drop dialog 0x%x for dialogid 0x%x",item,dialogueid);
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


