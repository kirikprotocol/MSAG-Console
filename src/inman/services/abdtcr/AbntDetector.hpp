/* ************************************************************************** *
 * Abonent Detector request processor: implements abonent contract and gsmSCF 
 * parameters determination logic.
 * ************************************************************************** */
#ifndef __INMAN_ABNT_DETECTOR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABNT_DETECTOR_HPP

#include "inman/common/OptionalObjT.hpp"
#include "inman/abprov/IAProvider.hpp"
#include "inman/interaction/msgdtcr/MsgContract.hpp"
#include "inman/interaction/asyncmgr/AsynWorkerDefs.hpp"
#include "inman/services/abdtcr/AbntDtcrDefs.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::util::RCHash;
using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;

using smsc::core::timers::TimeWatcherITF;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::OPAQUE_OBJ;

using smsc::inman::iaprvd::AbonentId;
using smsc::inman::iaprvd::AbonentSubscription;

using smsc::inman::interaction::WorkerGuard;
using smsc::inman::interaction::SPckContractRequest;
using smsc::inman::interaction::AbntContractRequest;

using smsc::inman::iapmgr::IAPRule;
using smsc::inman::iapmgr::IAPPrio_e;
using smsc::inman::iapmgr::INScfCFG;

class AbonentDetector : public smsc::inman::interaction::WorkerIface
                      , smsc::inman::iaprvd::IAPQueryListenerITF
                      , smsc::core::timers::TimerListenerITF
                      , smsc::inman::interaction::AbntContractReqHandlerITF {
public:
  enum ADState_e {
    adIdle = 0,
    adIAPQuering,
    adTimedOut,
    adDetermined,
    adCompleted,    // AD -> SMSC : AbntContractResult
    adAborted
  };

  AbonentDetector() : smsc::inman::interaction::WorkerIface()
    , _state(adIdle), _curIAPrvd(AbonentPolicy::iapNone), _cfgScf(NULL), _wErr(0)
  { }
  virtual ~AbonentDetector();

  static const char * state2Str(ADState_e st);
  //
  void configure(const AbonentDetectorCFG & use_cfg, const char * id_pfx);
  //Note: input packet is completely deserialized !
  void wrkHandlePacket(const SPckContractRequest & recv_pck) /*throw()*/;

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

protected:
  // -------------------------------------------------------
  // -- IAPQueryListenerITF interface methods:
  // -------------------------------------------------------
  //Returns false if listener unable to handle query report right now, so
  //requests query to be rereported.
  virtual bool onIAPQueried(const AbonentId & ab_number,
                            const AbonentSubscription & ab_info, RCHash qry_status);
  // -------------------------------------------------------
  // -- TimerListenerITF interface methods:
  // -------------------------------------------------------
  virtual TimeWatcherITF::SignalResult
      onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj);
  // -------------------------------------------------------
  // -- AbntContractReqHandlerITF interface methods:
  // -------------------------------------------------------
  virtual bool onContractReq(const AbntContractRequest * p_req, uint32_t req_id);

private:
  //Ids of external entities, which may refer this object
  enum RefEntities_e {
    refIdItself = 0x0
    , refIdIAProvider = 0x01
    , refIdTMWatcher = 0x02
    , refIdMAX = 0x03 //just a max cap
  };

  mutable EventMonitor  _sync;
  ADState_e             _state;
  AbonentDetectorCFG    _cfg;
  //prefix for logging info
  char                  _logId[sizeof("AbntDet[%u:%u]") + 2*sizeof(smsc::inman::interaction::WorkerID)*3 + 1];

  AbonentSubscription   _abCsi;     //abonent CAMEL subscriptions
  TonNpiAddress         _abNumber;  //calling abonent ISDN number
  IAPRule               _iapRule;   //abonent policy rule
  IAPPrio_e             _curIAPrvd; //UId of last IAProvider asked
  const INScfCFG *      _cfgScf;    //serving gsmSCF(IN-point) configuration
  uint32_t              _wErr;
  WorkerGuard           _wrkRefs[refIdMAX];
  smsc::util::OptionalObj_T<TimerHdl> _iapTimer;   //timer for InAbonentProvider quering

  const char * state2Str(void) { return state2Str(_state); }

  bool hasRef(RefEntities_e ref_id) const { return _wrkRefs[ref_id].get() != NULL; }
  void addRef(RefEntities_e ref_id) { _wrkRefs[ref_id] = _wrkMgr->getWorkerGuard(*this); }
  void unRef(RefEntities_e ref_id) { _wrkRefs[ref_id].release(); }

  bool verifyReq(const AbntContractRequest * p_req);
  //
  bool sendResult(void) const;
  //Returns true if qyery is started, so execution will continue in another thread.
  bool startIAPQuery(void);
  void doCleanUp(void);
  //Adjusts the MO-SM gsmSCF parameters combining cache/IAProvider CSIs
  //and gsmSCF parameters from config.xml
  void configureMOSM(void);
  void configureSCF(void);
  void reportAndClean(void);
  //
  bool startTimer(void);
  void stopTimer(void);
};

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ABNT_DETECTOR_HPP */

