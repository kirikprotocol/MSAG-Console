/* ************************************************************************** *
 *
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_REQUEST_ROCESSOR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_REQUEST_ROCESSOR_HPP

#include "util/OptionalObjT.hpp"

#include "inman/interaction/asyncmgr/AsynWorkerDefs.hpp"

#include "ussman/inap/map_uss/DlgMapUSS.hpp"
#include "ussman/comp/map_uss/MapUSSComps.hpp"

#include "ussman/interaction/MsgUSSProcess.hpp"
#include "ussman/service/USSRequestCfg.hpp"

namespace smsc {
namespace ussman {

using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;

using smsc::util::TonNpiAddress;
using smsc::util::RCHash;

using smsc::inman::inap::ObjFinalizerIface;
using smsc::inman::inap::ObjAllcStatus_e;

using smsc::inman::interaction::WorkerGuard;

using smsc::ussman::comp::USSDataString;
using smsc::ussman::comp::USSOperationData;
using smsc::ussman::inap::uss::MapUSSDlg;

using smsc::ussman::interaction::SPckUSSRequest;
using smsc::ussman::interaction::SPckUSSResult;

class USSReqProcessor : public smsc::inman::interaction::WorkerIface
                      , protected smsc::ussman::inap::uss::USSDhandlerITF {
public:
  enum ProcState_e {
    procIdle = 0
    , procInited    //MAP dialog successfully opened and request is sent, waiting for result
    , procResulted  //MAP dialog yelds result
    , procAborted   //processing aborted
    , procDone      //processing finished, all resources released
  };

  USSReqProcessor() : smsc::inman::interaction::WorkerIface()
    , _state(procIdle), _cfg(0)
  { }
  virtual ~USSReqProcessor();

  //
  void configure(const USSRequestCfg & use_cfg, const char * id_pfx);
  //Note: input packet is completely deserialized !
  void wrkHandlePacket(const SPckUSSRequest & recv_pck) /*throw()*/;
  //
  const USSOperationData & getReqData(void) const { return _reqData; }

  // ------------------------------------------
  // -- WorkerIface interface methods:
  // ------------------------------------------
  //Initializes worker as it's got from pool.
  virtual void wrkInit(smsc::inman::interaction::WorkerID w_id,
                       smsc::inman::interaction::WorkerManagerIface * use_mgr,
                       Logger * use_log = NULL);
  //Prints some information about worker state/status
  virtual void wrkLogState(std::string & use_str) const;
  //
  virtual void wrkAbort(const char * abrt_reason = NULL);

private:
  //Ids of external entities, which may refer this object
  enum RefEntities_e {
    refIdItself = 0
    , refIdMapDialog
    , refIdMAX //just a max cap
  };

  mutable EventMonitor    _sync;
  volatile ProcState_e    _state;
  const USSRequestCfg *   _cfg;
  char                    _logId[sizeof("USSReq[%u:%u]") + 2*sizeof(unsigned int)*3 + 1];
  USSOperationData        _reqData;
  SPckUSSResult           _resPck;
  WorkerGuard             _wrkRefs[refIdMAX];
  smsc::util::OptionalObj_T<MapUSSDlg> _mapDlg;

  bool hasRef(RefEntities_e ref_id) const { return _wrkRefs[ref_id].get() != NULL; }
  void addRef(RefEntities_e ref_id) { _wrkRefs[ref_id] = _wrkMgr->getWorkerGuard(*this); }
  void unRef(RefEntities_e ref_id) { _wrkRefs[ref_id].release(); }

  bool initMapDialog(uint8_t rmt_ssn, const TonNpiAddress & rnpi) /*throw()*/;
  void rlseMapDialog(void)/*throw()*/;
  void sendResult(void) const /*throw()*/;
  void doCleanUp(void)/*throw()*/;

protected:
  // ------------------------------------------
  // -- USSDhandlerITF interface methods:
  // ------------------------------------------
  virtual void onMapResult(const USSDataString & res_arg);
  //if err_code != 0, no result has been got from MAP service,
  //NOTE: dialog user may destroy reporting MAP dialog from this callback in two ways:
  //   1) by calling MapDialogAC::releaseThis() if dialog is allocated on heap
  //   2) by calling ObjFinalizerIface::finalizeObj() and then MapDialogAC destructor
  virtual ObjAllcStatus_e onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code = 0);
  //
  virtual void Awake(void) { _sync.notify(); }
};

} //ussman
} //smsc

#endif /* __SMSC_USSMAN_REQUEST_ROCESSOR_HPP */

