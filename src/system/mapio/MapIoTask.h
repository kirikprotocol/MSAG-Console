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
  static inline unsigned CalcHash(ET96MAP_DIALOGUE_ID_T id){
    return (unsigned)id;
  }
};

enum MapDialogState {
  MAPST_START,
  MAPST_WAIT_SUBMIT_RESPONSE,
  MAPST_READY_FOR_SENDSMS,
  MAPST_WAIT_RINFO,
  MAPST_RINFOIND,
  MAPST_READY_FOR_CLOSE,
  MAPST_WAIT_SEGMINTATION,
  MAPST_BROKEN,
  MAPST_OPENCONF
};

extern void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);


/**
  \class MapDialog
*/
class MapDialog{
  Mutex mutex;
  unsigned ref_count;
  MapDialogState state;
  ET96MAP_DIALOGUE_ID_T dialogid;
  unsigned smscDialogId;
  ET96MAP_INVOKE_ID_T invokeId;
  ET96MAP_LOCAL_SSN_T ssn;
  auto_ptr<char> abonent;
  auto_ptr<SMS> sms;
  auto_ptr<ET96MAP_SM_RP_UI_T> auto_ui;
  ET96MAP_APP_CNTX_T appContext;
  ET96MAP_ADDRESS_T m_msAddr;	
  ET96MAP_ADDRESS_T m_scAddr;	
  ET96MAP_SS7_ADDR_T scAddr;
  ET96MAP_SS7_ADDR_T destMscAddr;
  ET96MAP_SS7_ADDR_T mshlrAddr;
 	ET96MAP_SM_RP_DA_T smRpDa;
  ET96MAP_SM_RP_OA_T smRpOa;
public:
  MapDialog(ET96MAP_DIALOGUE_ID_T dialogid,ET96MAP_LOCAL_SSN_T lssn) : 
    ref_count(1),
    state(MAPST_START), 
    dialogid(dialogid),
    smscDialogId(0),
    ssn(lssn) 
    {}
  virtual ~MapDialog(){
    __trace2__("MAP::Dialog::~MapDialog 0x%x(0x%x)",dialogid,smscDialogId);
    require ( ref_count == 0 );
    if ( smscDialogId != 0 ){
      freeDialogueId(dialogid);
    }
  }
  Mutex& getMutex(){
    return mutex;
  }
  void Release(){
    unsigned x = 0;
    mutex.Lock();
    x = --ref_count;
    mutex.Unlock();
    if ( ref_count == 0 ){
      delete this;
    }
  }
  void AddRef(){
    MutexGuard g(mutex);
    ++ref_count;
  }
  USHORT_T getDialogId() { return dialogid; }
  void setDialogId(USHORT_T ndid) { dialogid = ndid; }
  unsigned getSMSCDialogId() { return smscDialogId; }
  void setSMSCDialogId(unsigned did) {smscDialogId=did;}
  const char* getAbonent() { 
    return abonent.get();
  }
  void setAbonent(const char* a) { 
    abonent = auto_ptr<char>(new char[32]);
    memset(abonent.get(),0,32); 
    strncpy(abonent.get(),a,31); 
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
  virtual void  Et96MapOpenConf (
    ET96MAP_LOCAL_SSN_T localSsn,
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_OPEN_RESULT_T openResult,
    ET96MAP_REFUSE_REASON_T *refuseReason_p,
    ET96MAP_SS7_ADDR_T *respondingAddr_sp,
    ET96MAP_APP_CNTX_T *appContext_sp,
    ET96MAP_USERDATA_T *specificInfo_sp,
    ET96MAP_PROV_ERR_T *provErrCode_p);
  void setInvokeId(ET96MAP_INVOKE_ID_T invokeId) {this->invokeId = invokeId;}
  // возвращает истину если это последнее сообщение в диалоге и далее диалог
  // должн быть закрыт
  virtual bool ProcessCmd(const SmscCommand& cmd);
  virtual void Et96MapPAbortInd(
    ET96MAP_LOCAL_SSN_T lssn,
    ET96MAP_DIALOGUE_ID_T dialogid,
    ET96MAP_PROV_REASON_T reason,
    ET96MAP_SOURCE_T source,
    UCHAR_T priorityOrder);

  virtual void Et96MapUAbortInd(
    ET96MAP_LOCAL_SSN_T lssn,
    ET96MAP_DIALOGUE_ID_T dialogid,
    ET96MAP_USER_REASON_T *reason,
    ET96MAP_DIAGNOSTIC_INFO_T* diag,
    ET96MAP_USERDATA_T *ud,
    UCHAR_T priorityOrder);
  virtual void Et96MapV2ForwardSmMTConf (
    ET96MAP_LOCAL_SSN_T localSsn,
    ET96MAP_DIALOGUE_ID_T dialogid,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
    ET96MAP_PROV_ERR_T *provErrCode_p);
  virtual void Et96MapDelimiterInd(
    ET96MAP_LOCAL_SSN_T lssn,
    ET96MAP_DIALOGUE_ID_T dialogId,
    UCHAR_T priorityOrder);
};

class DialogRefGuard{
  MapDialog* dialog;
public:
  DialogRefGuard(MapDialog* d = 0):dialog(d){
    //d->AddRef();
  }
  ~DialogRefGuard(){
    if ( dialog ) dialog->Release();
  }
  void asssign(MapDialog* d){
    if ( dialog == d ) return;
    if ( dialog ) dialog->Release();
    dialog = d;
  }
  bool isnull(){
    return dialog==0;
  }
  void forget(){
    dialog = 0;
  }
  MapDialog* operator->() { return dialog; }
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
  Hash<unsigned> lock_map;
  list<unsigned> dialogId_pool;
  friend void freeDialogueId(ET96MAP_DIALOGUE_ID_T dialogueId);
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
  
  MapDialog* createDialog(ET96MAP_DIALOGUE_ID_T dialogueid,ET96MAP_LOCAL_SSN_T lssn/*,const char* abonent*/){
    MutexGuard g(sync);
    /*__trace2__("MAP::createSMSCDialog: try create dialog on abonent %s",abonent);
    if ( lock_map.Exists(abonent) ) {
      __trace2__("MAP::createSMSCDialog: locked");
      return 0;
    }*/
    MapDialog* dlg = new MapDialog(dialogueid,lssn);
    //dlg->setAbonent(abonent);
    hash.Insert(dialogueid,dlg);
    //lock_map.Insert(abonent,1);
    __trace2__("MAP:: new dialog 0x%p for dialogid 0x%x",dlg,dialogueid);
    dlg->AddRef();
    return dlg;
  }
  
  MapDialog* createSMSCDialog(unsigned smsc_did,ET96MAP_LOCAL_SSN_T lssn,const char* abonent){
    MutexGuard g(sync);
    __trace2__("MAP::createSMSCDialog: try create SMSC dialog on abonent %s",abonent);
    if ( lock_map.Exists(abonent) ) {
      __trace2__("MAP::createSMSCDialog: locked");
      return 0;
    }
    ET96MAP_DIALOGUE_ID_T map_dialog = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    MapDialog* dlg = new MapDialog(map_dialog,lssn);
    dlg->setSMSCDialogId (smsc_did);
    dlg->setAbonent(abonent);
    hash.Insert(map_dialog,dlg);
    lock_map.Insert(abonent,1);
    __trace2__("MAP:: new dialog 0x%p for dialogid 0x%x->0x%x",dlg,smsc_did,map_dialog);
    dialogId_pool.pop_front();
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
    ET96MAP_DIALOGUE_ID_T map_dialogid = (ET96MAP_DIALOGUE_ID_T)dialogId_pool.front();
    dialogId_pool.push_back(did);
    dlg->setDialogId (map_dialogid);
    hash.Delete(did);
    hash.Insert(map_dialogid,dlg);
    __trace2__("MAP:: reassign dialog 0x%x->0x%x",did,map_dialogid);
    dialogId_pool.pop_front();
    return map_dialogid;
  }
  
  void dropDialog(ET96MAP_DIALOGUE_ID_T dialogueid){
    MutexGuard g(sync);
    MapDialog* item = 0;
    if ( hash.Get(dialogueid,item) ){
      __trace2__("MAP:: drop dialog 0x%p for dialogid 0x%x",item,dialogueid);
      if ( item->getAbonent() ) lock_map.Delete(item->getAbonent());
      hash.Delete(dialogueid);
      //delete item;
      item->Release();
    }else{
      __trace2__("MAP::dropDialog: has no dialog for dialogid 0x%x",dialogueid);
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


