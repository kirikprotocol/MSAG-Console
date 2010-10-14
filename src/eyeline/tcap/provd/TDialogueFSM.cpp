#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

//#include "util/Exception.hpp"
#include "eyeline/tcap/provd/TDialogueFSM.hpp"
#include "eyeline/tcap/provd/TCompIndComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//using eyeline::ros::RejectProblem;
using eyeline::ros::ROSRejectPdu;
using eyeline::ros::ROSPduPrimitiveAC;
using smsc::core::synchronization::MutexGuard;
//using smsc::core::synchronization::MutexTryGuard;

using eyeline::corex::timers::TimeWatcherIface;
/* ************************************************************************** *
 * class TDialogueFSM implementaton
 * ************************************************************************** */
const char* const
  TDialogueFSM::stateToString(TDialogueFSM::TransactionState_e trn_state)
{
  switch (trn_state) {
  case fsmIDLE:           return "fsmIDLE";
  case fsmINIT_SENT:      return "fsmINIT_SENT";
  case fsmINIT_RECEIVED:  return "fsmINIT_RECEIVED";
  case fsmACTIVE:         return "fsmACTIVE";
  case fsmDONE:           return "fsmDONE";
  default:;
  }
  return "fsmINVALID";
}


void TDialogueFSM::init(TDialogueId dlg_id, const TDlgLocalConfig & dlg_cfg,
            TDlgCoordinatorIface * dlg_mgr)
{
  MutexGuard  grd(_sync);
  _cfg = dlg_cfg;
  _dlgMgr = dlg_mgr;
  _dlgUser = dlg_cfg._indHdl;
  _trnUId.initLocal(dlg_id, 0);
  snprintf(_logId, sizeof(_logId), "TDlg[%u]", dlg_id);
}

void TDialogueFSM::init(TDialogueId dlg_id, const TDlgRemoteConfig & dlg_cfg,
              TDlgCoordinatorIface * dlg_mgr)
{
  MutexGuard  grd(_sync);
  _cfg = dlg_cfg;
  _dlgMgr = dlg_mgr;
  _dlgUser = dlg_cfg._indHdl;
  _trnUId.initRemote(0, dlg_id);
  snprintf(_logId, sizeof(_logId), "TDlg[%u]", dlg_id);
}

void TDialogueFSM::log_error_state(const char * tr_prim) const
{
  smsc_log_error(_logger, "%s: invalid %s, mismatched state=[%s(%u)]", _logId,
                 tr_prim, stateToString(), _trnState);
}

// ---------------------------------------
// -- TimerListenerIface implementaton
// ---------------------------------------
TimeWatcherIface::SignalResult_e
  TDialogueFSM::onTimerEvent(const TimerIdent & tm_uid, const OpaqueArg * opaque_arg/* = NULL*/)
{
/*
  MutexTryGuard grd(*this);
  if (!grd.tgtLocked())
    return TimeWatcherIface::evtResignal;
*/
  MutexGuard  grd(_sync);

  smsc_log_debug(_logger, "%s: timer[%s] is signalled", _logId, tm_uid.c_str());

  if (!opaque_arg) {  //dialogue timer
    if ((tm_uid != _dlgTmo._uid)
        || (_dlgTmo._state == TDialogueFSM::tmoIdle)) { //assertion!!!
      smsc_log_fatal(_logger, "%s: invalid dialogue timer[%s] is signalled", _logId,
                     tm_uid.c_str());
    } else {
      if (_dlgTmo._state == TDialogueFSM::tmoActive) {
        smsc_log_warn(_logger, "%s: dialogue timer[%s] signaled",  _logId, tm_uid.c_str());
        //indication task will call updateDlgByIndication(TPAbortIndComposer & t_pAbort_ind)
        _dlgMgr->notifyLclTCUser(_trnUId, PAbort::p_resourceLimitation, _cfg._scspLink);
        _refGrd.UnRef();
      }
      _dlgTmo._state = TDialogueFSM::tmoDone;
    }
  } else {            //invoke timer
    if (!opaque_arg->isUInt()) { //assertion!!!
      smsc_log_fatal(_logger, "%s: invalid invoke timer[%s] is signalled", _logId,
                     tm_uid.c_str());
    } else {
      uint8_t     invId = (uint8_t)opaque_arg->getUInt();
      TimerInfo * pTmo = (invId < _lclInv.size()) ? &_lclInv[invId] : NULL;

      if (!pTmo || (pTmo->_state == TDialogueFSM::tmoIdle)) { //assertion!!!
        smsc_log_fatal(_logger, "%s: invalid invoke[%u] timer[%s] is signalled",  _logId,
                       (unsigned)invId, tm_uid.c_str());
      } else {
        if (pTmo->_state == TDialogueFSM::tmoActive) {
          smsc_log_warn(_logger, "%s:  invoke[%u] timer[%s] signaled",  _logId,
                        (unsigned)invId, tm_uid.c_str());
          _dlgMgr->notifyLclTCUser(_trnUId, invId);
          _refGrd.UnRef();
        }
        pTmo->_state = TDialogueFSM::tmoDone;
      }
    }
  }
  _sync.notifyAll();
  return TimeWatcherIface::evtOk;
}

// ----------------------------------------------------------------------------
// -- Protected/Private methods
// ----------------------------------------------------------------------------
// -- NOTE: FSM must be locked prior to call of these methods!
// ----------------------------------------------------------------------------
TCRCode_e
  TDialogueFSM::verifyTCReqComponents(const TComponentsPtrList & comp_list,
                                      const char * req_id,
                                      const TComponentType_e comp_type) const
{
  if (!comp_list.empty()) {
    for (TComponentsPtrList::const_iterator it = comp_list.begin();
                                            it != comp_list.end() && (*it); ++it) {
      const ros::ROSPduPrimitiveAC * rosComp = (*it)->get();
      uint8_t invId = rosComp->getInvokeId();

      if (rosComp->isInvoke()) { //verify local invocation id
        if (invId < _lclInv.size()) {
          if (!_lclInv[invId].isIdle()) { //invId isn't safe to reuse
            smsc_log_error(_logger, "%s: invalid %s, duplicate invokeId=%u of %s", _logId,
                           req_id, (unsigned)invId, rosComp->nmKind());
            return TCError::errInvokeId;
          }
        } else {
          if (invId == _lclInv.npos()) {
            smsc_log_error(_logger, "%s: invalid %s, illegal invokeId=%u of %s", _logId,
                           req_id, (unsigned)invId, rosComp->nmKind());
            return TCError::errTCAPLimit;
          }
          _lclInv.at(invId); //construct TimerInfo
        }

        if (comp_type == TComponentType::compNotReturnable) {
          const ros::ROSInvokePdu * pInv = static_cast<const ros::ROSInvokePdu *>(rosComp);
          if (pInv->getParam().isAlwaysReturn()) {
            smsc_log_error(_logger, "%s: invalid %s, invoke[%u] expects result", _logId,
                           req_id, (unsigned)invId);
            return TCError::errComponentPortion;
          }
        }
      } else { //verify remote invocation id
        if (comp_type == TComponentType::compInvokeOnly) {
          smsc_log_error(_logger, "%s: invalid %s, illegal %s", _logId,
                         req_id, rosComp->nmKind());
          return TCError::errComponentPortion;
        }
        if (_rmtInv.find(invId) == _rmtInv.npos()) {
          smsc_log_error(_logger, "%s: invalid %s, invalid invokeId=%u of %s", _logId,
                         req_id, (unsigned)invId, rosComp->nmKind());
          return TCError::errInvokeId;
        }
      }
    }
  }
  return TCError::dlgOk;
}

bool TDialogueFSM::verifyTCIndComponents(TComponentsList * comps,
                                         const char * tind_id,
                                         const TComponentType_e comp_type) /*throw()*/
{
  if (!comps || comps->empty())
    return true;

  _rejComps.clear();

  for (TComponentsList::iterator it = comps->begin(); it != comps->end(); ++it) {
    const ROSPduPrimitiveAC * rosComp = it->get();
    uint8_t invId = rosComp->getInvokeId();
    TimerInfo * pTmo = NULL;

    switch (rosComp->getKind()) {
    case ROSPduPrimitiveAC::rosInvoke: {
      //verify remote invocation id
      if (_rmtInv.find(invId) == _rmtInv.npos()) {
        _rmtInv.insert(invId);
      } else {
        smsc_log_error(_logger, "%s: invalid %s, duplicate invokeId=%u of %s", _logId,
                       tind_id, (unsigned)invId, rosComp->nmKind());
        //reject this component
        TC_LRejectIndComposer indReject;
        indReject.setDialogueId(_trnUId.getIdLocal());
        indReject.setProblem(RejectProblem::rejInvoke,
                             RejectProblem::rji_duplicateInvocation);
        indReject.setInvokeId(rosComp->getInvokeId());
        /**/
        _rejComps.push_back(indReject);
        it->clear();
      }
    } break;

    case ROSPduPrimitiveAC::rosError: {
      //verify local invocation id
      if (invId < _lclInv.size()) {
        pTmo = &_lclInv[invId];
      } else {
        smsc_log_error(_logger, "%s: invalid %s, unknown invokeId=%u of %s", _logId,
                       tind_id, (unsigned)invId, rosComp->nmKind());
        //reject this component
        TC_LRejectIndComposer indReject;
        indReject.setDialogueId(_trnUId.getIdLocal());
        indReject.setInvokeId(rosComp->getInvokeId());
        indReject.setProblem(RejectProblem::rejError,
                             RejectProblem::rje_unrecognizedInvocation);
        /**/
        _rejComps.push_back(indReject);
        it->clear();
      }
    } break;

    case ROSPduPrimitiveAC::rosResult:
    case ROSPduPrimitiveAC::rosResultNL: {
      //verify local invocation id
      if (invId < _lclInv.size()) {
        pTmo = (rosComp->getKind() == ROSPduPrimitiveAC::rosResult) ? &_lclInv[invId] : NULL;
      } else {
        smsc_log_error(_logger, "%s: invalid %s, unknown invokeId=%u of %s", _logId,
                       tind_id, (unsigned)invId, rosComp->nmKind());
        //reject this component
        TC_LRejectIndComposer indReject;
        indReject.setDialogueId(_trnUId.getIdLocal());
        indReject.setInvokeId(rosComp->getInvokeId());
        indReject.setProblem(ros::RejectProblem::rejResult,
                             ros::RejectProblem::rjr_unrecognizedInvocation);
        /**/
        _rejComps.push_back(indReject);
        it->clear();
      }
    } break;

    case ROSPduPrimitiveAC::rosReject: {
      ROSRejectPdu * pdu = it->reject().get();
      //check if local invocation was rejected by remote peer
      if (pdu->getParam().getProblemKind() == RejectProblem::rejInvoke)
        pTmo = (invId < _lclInv.size()) ? &_lclInv[invId] : NULL;
    } break;

    case ROSPduPrimitiveAC::rosCancel: {
      //component wasn't properly identified/decoded by underlying layer
      //reject this component
      TC_LRejectIndComposer indReject;
      indReject.setDialogueId(_trnUId.getIdLocal());
      indReject.setProblem(ros::RejectProblem::rejGeneral,
                           ros::RejectProblem::rjg_unrecognizedPDU);
      /**/
      _rejComps.push_back(indReject);
      it->clear();
    } break;
    //default:;
    } /*eosw*/

    if (pTmo && !stopTimer(*pTmo)) {
      smsc_log_fatal(_logger, "%s: failed to stop invoke[%u] timer[%s]",  _logId,
                     (unsigned)invId, pTmo->_uid.c_str());
    }
  } /*eofr*/
  return !_rejComps.empty();
}


// ----------------------------------------------------------------------------
// -- Timers handling methods
// ----------------------------------------------------------------------------
bool TDialogueFSM::stopTimer(TimerInfo & tmo_info)
{
  if (tmo_info._state == tmoIdle)
    return true;

  if (tmo_info._state == TDialogueFSM::tmoActive) {
    if (tmo_info._hdl.stop() == TimeWatcherIface::errTimerState) {
      tmo_info._state = TDialogueFSM::tmoCancelled;
    } else {
      tmo_info._state = TDialogueFSM::tmoDone;
      _refGrd.UnRef();
    }
  }
  if (tmo_info._state == TDialogueFSM::tmoCancelled)
    _sync.wait(); //wait for onTimerEvent() completion

  return (tmo_info._state == tmoDone);
}


bool TDialogueFSM::activateDialogueTimer(void) /*throw()*/
{
  _dlgTmo._hdl = _cfg._tmoReg->CreateTimer(this, 
                            TimeSlice(_cfg._dlgTmo, TimeSlice::tuSecs), NULL);

  if (!_dlgTmo._hdl.empty() && _dlgTmo._hdl.start()) {
    _dlgTmo._state = TDialogueFSM::tmoActive;
    _dlgTmo._uid = _dlgTmo._hdl.getIdent();
    _refGrd.AddRef();
    smsc_log_debug(_logger, "%s: started dialogue timer[%s]", _logId,
                   _dlgTmo._uid.c_str());
    return true;
  }
  smsc_log_fatal(_logger, "%s: failed to start dialogue timer", _logId);
  return false;
}

//NOTE: _lclInv must have initialized element associated with inv_id
bool TDialogueFSM::startInvokeTimer(uint8_t inv_id, bool is_restart/* = false*/) /*throw()*/
{
  TimerInfo & pTmo = _lclInv[inv_id];

  if (!pTmo._hdl.empty() && pTmo._hdl.start()) {
    pTmo._state = TDialogueFSM::tmoActive;
    pTmo._uid = pTmo._hdl.getIdent();
    _refGrd.AddRef();
    smsc_log_debug(_logger, "%s: %sstarted invoke[%u] timer[%s]", _logId,
                   is_restart ? "re":"", (unsigned)inv_id, pTmo._uid.c_str());
    return true;
  }
  smsc_log_fatal(_logger, "%s: failed to %sstart invoke[%u] timer", _logId,
                 is_restart ? "re":"", (unsigned)inv_id);
  return false;
}

//NOTE: _lclInv must have initialized element associated with inv_id
bool TDialogueFSM::activateInvokeTimer(uint8_t inv_id, uint16_t tmo_secs/* = 0*/) /*throw()*/
{
  TimerInfo & pTmo = _lclInv[inv_id];
  OpaqueArg   tmoArg((unsigned)inv_id);
  TimeSlice   tmoVal(tmo_secs ? tmo_secs : _cfg._dlgTmo, TimeSlice::tuSecs);

  pTmo._hdl = _cfg._tmoReg->CreateTimer(this, tmoVal, &tmoArg);
  return startInvokeTimer(inv_id);
}

//Stops timer associated with specified Invocation.
//Returns true if timer succesfully stopped, false - otherwise.
//NOTE: inv_id must be a valid position in _lclInv array.
bool TDialogueFSM::cancelInvokeTimer(uint8_t inv_id)  /*throw()*/
{
  TimerInfo & pTmo = _lclInv[inv_id];
  if (!stopTimer(pTmo)) {
    smsc_log_fatal(_logger, "%s: failed to stop invoke[%u] timer[%s]",  _logId,
                   (unsigned)inv_id, pTmo._uid.c_str());
    return false;
  }
  return true;
}

//Stops all dialogue initiated timers (dialogue timer + invocation timers).
void TDialogueFSM::cancelDialogueTimers(void) /*throw()*/
{
  if (!_dlgTmo.empty() && !stopTimer(_dlgTmo))
    smsc_log_fatal(_logger, "%s: failed to stop dialogue timer", _logId);

  for (uint8_t i = 0; i < _lclInv.size(); ++i)
    cancelInvokeTimer(i);
}
//
void TDialogueFSM::cancelTCompTimers(const TComponentsPtrList & comp_list,
                                   TComponentsPtrList::const_iterator end_it) /*throw()*/
{
  for (TComponentsPtrList::const_iterator it = comp_list.begin();
                                                it != comp_list.end(); ++it) {
    const ros::ROSPduPrimitiveAC * rosComp = (*it)->get();
    if (rosComp && rosComp->isInvoke()) {
      const ros::ROSInvokePdu * pInv = static_cast<const ros::ROSInvokePdu *>(rosComp);
      if (pInv->getParam().isAlwaysReturn())
        cancelInvokeTimer(pInv->getInvokeId());
    }
  }
}

bool TDialogueFSM::activateTCompTimers(const TComponentsPtrList & comp_list) /*throw()*/
{
  if (!comp_list.empty()) {
    for (TComponentsPtrList::const_iterator it = comp_list.begin();
                                                  it != comp_list.end(); ++it) {
      const ros::ROSPduPrimitiveAC * rosComp = (*it)->get();
      if (rosComp && rosComp->isInvoke()) {
        const ros::ROSInvokePdu * pInv = static_cast<const ros::ROSInvokePdu *>(rosComp);
        if (pInv->getParam().isAlwaysReturn()
            && !activateInvokeTimer(pInv->getInvokeId(), pInv->getParam()._timeOut)) {
          cancelTCompTimers(comp_list, ++it);
          return false;
        }
      }
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
// -- Public methods
// ----------------------------------------------------------------------------

void TDialogueFSM::stopDialogue(void) /*throw()*/
{
  //remove dialogue from active dialogue registry
  _dlgMgr->rlseDialogue(getDialogueId());
  //reset TCUser in order to block indication reporting
  _dlgUser = NULL;
  //stop all timers, reset all refs
  cancelDialogueTimers();
}


// ----------------------------------------------------------------------------
// -- Dialogue request primitives verification and finalization
// ----------------------------------------------------------------------------
TCRCode_e
  TDialogueFSM::prepareDlgRequest(TBeginReqComposer & treq_begin,
                                  SCSPUnitdataReq * req_udt) const /*throw()*/
{
  MutexGuard  grd(_sync);

  if (_trnState != fsmIDLE) {
    log_error_state(treq_begin.TReq().getIdent());
    return TCError::errDialogueState;
  }
  if (treq_begin.TReq().getDestAddress().empty()) {
    smsc_log_error(_logger, "%s: invalid %s, empty dst address", _logId,
                   treq_begin.TReq().getIdent());
    return TCError::errDstAddress;
  }

  TCRCode_e rval = 
    verifyTCReqComponents(treq_begin.TReq().getCompList(), treq_begin.TReq().getIdent(),
                          TComponentType::compInvokeOnly);
  if (rval != TCError::dlgOk)
    return rval;

  treq_begin.setTransactionId(_trnUId);
  if (!treq_begin.TReq().getAppCtx())
   treq_begin.setAppCtx(_cfg._acOid);

  if (req_udt) {
    try {
      rval = treq_begin.serialize2UDT(*req_udt, _cfg._ownAddr, treq_begin.TReq().getDestAddress());
    } catch (const std::exception & exc) {
      smsc_log_error(_logger, "%s: %s::serialize2UDT() exception=\'%s\'",
                     _logId, treq_begin.TReq().getIdent(), exc.what());
      rval = TCError::errTCAPUnknown;
    }
  }
  return rval;
}

TCRCode_e
  TDialogueFSM::prepareDlgRequest(TContReqComposer & treq_cont,
                                  SCSPUnitdataReq * req_udt) const /*throw()*/
{
  MutexGuard  grd(_sync);
  if ((_trnState != fsmINIT_RECEIVED) && (_trnState != fsmACTIVE)) {
    log_error_state(treq_cont.TReq().getIdent());
    return TCError::errDialogueState;
  }

  TCRCode_e rval = 
    verifyTCReqComponents(treq_cont.TReq().getCompList(), treq_cont.TReq().getIdent(),
                          TComponentType::compAny);
  if (rval != TCError::dlgOk)
    return  rval;

  const SCCPAddress * ownAddr = &_cfg._ownAddr;

  if (_trnState == fsmINIT_RECEIVED) { //response to TBeginInd:
    //TCUser may substitute org_adr or appCtx
    treq_cont.setDialogueResponse();
    if (!treq_cont.TReq().getOrigAddress().empty())
      ownAddr = &(treq_cont.TReq().getOrigAddress());
  }
  treq_cont.setTransactionId(_trnUId);
  if (!treq_cont.TReq().getAppCtx())
   treq_cont.setAppCtx(_cfg._acOid);

  if (req_udt) {
    try {
      rval = treq_cont.serialize2UDT(*req_udt, *ownAddr, _rmtAddr);
    } catch (const std::exception & exc) {
      smsc_log_error(_logger, "%s: %s::serialize2UDT() exception=\'%s\'",
                     _logId, treq_cont.TReq().getIdent(), exc.what());
      rval = TCError::errTCAPUnknown;
    }
  }
  return rval;
}

TCRCode_e
  TDialogueFSM::prepareDlgRequest(TEndReqComposer & treq_end,
                                  SCSPUnitdataReq * req_udt) const /*throw()*/
{
  MutexGuard  grd(_sync);
  if ((_trnState != fsmINIT_RECEIVED) && (_trnState != fsmACTIVE)) {
    log_error_state(treq_end.TReq().getIdent());
    return TCError::errDialogueState;
  }
  //verify that CompList contains no Invoke with return result expected
  TCRCode_e rval =
    verifyTCReqComponents(treq_end.TReq().getCompList(), treq_end.TReq().getIdent(),
                          TComponentType::compNotReturnable);
  if (rval != TCError::dlgOk)
    return  rval;

  const SCCPAddress * ownAddr = &_cfg._ownAddr;

  if (_trnState == fsmINIT_RECEIVED) { //response to TBeginInd:
    //TCUser may substitute org_adr or appCtx
    treq_end.setDialogueResponse();
    if (!treq_end.TReq().getOrigAddress().empty())
      ownAddr = &(treq_end.TReq().getOrigAddress());
  }
  treq_end.setTransactionId(_trnUId);
  if (!treq_end.TReq().getAppCtx())
   treq_end.setAppCtx(_cfg._acOid);

  if (req_udt) {
    try {
      rval = treq_end.serialize2UDT(*req_udt, *ownAddr, _rmtAddr);
    } catch (const std::exception & exc) {
      smsc_log_error(_logger, "%s: %s::serialize2UDT() exception=\'%s\'",
                     _logId, treq_end.TReq().getIdent(), exc.what());
      rval = TCError::errTCAPUnknown;
    }
  }
  return rval;
}

TCRCode_e
  TDialogueFSM::prepareDlgRequest(TUAbortReqComposer & treq_uAbort,
                                  SCSPUnitdataReq * req_udt) const /*throw()*/
{
  MutexGuard  grd(_sync);

  if ((_trnState == fsmIDLE) || (_trnState == fsmDONE)) {
    log_error_state(treq_uAbort.TReq().getIdent());
    return TCError::errDialogueState;
  }
  treq_uAbort.setTransactionId(_trnUId);
  if (_trnState == fsmINIT_RECEIVED)
    treq_uAbort.setDialogueResponse();

  TCRCode_e rval = TCError::dlgOk;
  if (req_udt) {
    try {
      rval = treq_uAbort.serialize2UDT(*req_udt, _cfg._ownAddr, _rmtAddr);
    } catch (const std::exception & exc) {
      smsc_log_error(_logger, "%s: %s::serialize2UDT() exception=\'%s\'",
                     _logId, treq_uAbort.TReq().getIdent(), exc.what());
      rval = TCError::errTCAPUnknown;
    }
  }
  return rval;
}

// ----------------------------------------------------------------------------
// -- Dialogue(Component contained) request primitives handling
// ----------------------------------------------------------------------------

TCRCode_e TDialogueFSM::updateDlgByRequest(const TBeginReqComposer & treq_begin)
  /*throw(std::exception)*/
{
  MutexGuard  grd(_sync);

  if (!activateDialogueTimer())
    return TCError::errTCAPLimit;
  if (!activateTCompTimers(treq_begin.TReq().getCompList())) {
    cancelDialogueTimers();
    return TCError::errTCAPLimit;
  }
  _rmtAddr = treq_begin.TReq().getDestAddress();
  if (_cfg._acOid != *treq_begin.getAppCtx()) //TCUser downgrades appCtx
    _cfg._acOid = *treq_begin.getAppCtx();

  _trnState = fsmINIT_SENT;
  return TCError::dlgOk;
}

TCRCode_e TDialogueFSM::updateDlgByRequest(const TContReqComposer & treq_cont)
  /*throw(std::exception)*/
{
  MutexGuard  grd(_sync);

  if (!activateTCompTimers(treq_cont.TReq().getCompList()))
    return TCError::errTCAPLimit;

  if (_trnState == fsmINIT_RECEIVED) { //response to TBeginInd: 
    //TCUser may substitute org_adr or appCtx
    if (!treq_cont.TReq().getOrigAddress().empty()) 
      _cfg._ownAddr = treq_cont.TReq().getOrigAddress();
    if (treq_cont.TReq().getAppCtx())
      _cfg._acOid = *treq_cont.getAppCtx();
  }
  _trnState = fsmACTIVE;
  return TCError::dlgOk;
}

TCRCode_e TDialogueFSM::updateDlgByRequest(const TEndReqComposer & treq_end)
  /*throw(std::exception)*/
{
  MutexGuard  grd(_sync);

  if (_trnState == fsmINIT_RECEIVED) { //response to TBeginInd: 
    //TCUser may substitute org_adr or appCtx
    if (!treq_end.TReq().getOrigAddress().empty()) 
      _cfg._ownAddr = treq_end.TReq().getOrigAddress();
    if (treq_end.TReq().getAppCtx())
      _cfg._acOid = *treq_end.getAppCtx();
  }

  _trnState = fsmDONE;
  cancelDialogueTimers();
  _dlgMgr->rlseDialogue(getDialogueId());
  return TCError::dlgOk;
}

TCRCode_e TDialogueFSM::updateDlgByRequest(const TUAbortReqComposer & treq_uAbort)
  /*throw(std::exception)*/
{
  MutexGuard  grd(_sync);
  _trnState = fsmDONE;
  cancelDialogueTimers();
  _dlgMgr->rlseDialogue(getDialogueId());
  return TCError::dlgOk;
}

// ----------------------------------------------------------------------------
// -- Component only request primitives handling
// ----------------------------------------------------------------------------
TCRCode_e
  TDialogueFSM::updateDlgByRequest(const TC_U_Cancel_Req & tc_req) /*throw()*/
{
  MutexGuard  grd(_sync);
  if (tc_req.getInvokeId() < _lclInv.size()) {
    cancelInvokeTimer(tc_req.getInvokeId());
    return TCError::dlgOk;
  }
  return TCError::errInvokeId;
}

TCRCode_e
  TDialogueFSM::updateDlgByRequest(const TC_Timer_Reset_Req & tc_req) /*throw()*/
{
  MutexGuard  grd(_sync);
  if (tc_req.getInvokeId() < _lclInv.size()) {
    if (cancelInvokeTimer(tc_req.getInvokeId()) && startInvokeTimer(tc_req.getInvokeId(), true))
      return TCError::dlgOk;
    return TCError::errTCAPUnknown;
  }
  return TCError::errInvokeId;
}


// ----------------------------------------------------------------------------
// -- Dialogue Indications handling methods
// ----------------------------------------------------------------------------
void TDialogueFSM::updateDlgByIndication(TBeginIndComposer & t_begin_ind)
  /*throw(std::exception)*/
{
   //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  if (_trnState != fsmIDLE) { //assertion!!!
    log_error_state(t_begin_ind.getIdent());
    return;
  }
  
  _rmtAddr = t_begin_ind.getOrigAddress();
  _trnUId.bindRemote(t_begin_ind.getOrigTrId());
  _trnState = fsmINIT_RECEIVED;
  _cfg._scspLink = t_begin_ind.getSCSPLink();
  /**/
  if (!activateDialogueTimer()) {
    _dlgMgr->notifyRmtTCUser(_trnUId, PAbort::p_resourceLimitation,
                             _cfg._scspLink, _cfg._ownAddr, _rmtAddr);
    stopDialogue();
    return;
  }

  //check components and create rejected component indication primitives if necessary
  verifyTCIndComponents(t_begin_ind.getCompList(), t_begin_ind.getIdent(),
                        TComponentType::compInvokeOnly);

  if (!notifyLocalTCUser(grdUser, t_begin_ind)) {
    _dlgMgr->notifyRmtTCUser(_trnUId, TDialogueAssociate::dsu_no_reason_given,
                             _cfg._scspLink, _cfg._ownAddr, _rmtAddr);
    stopDialogue();
    return;
  }
  //TODO: report all rejected components, for now only 1st one
  if (!_rejComps.empty() && !notifyLocalTCUser(grdUser, _rejComps.front())) {
    _dlgMgr->notifyRmtTCUser(_trnUId, TDialogueAssociate::dsu_no_reason_given,
                             _cfg._scspLink, _cfg._ownAddr, _rmtAddr);
    stopDialogue();
  }
}

void TDialogueFSM::updateDlgByIndication(TContIndComposer & t_cont_ind)
  /*throw(std::exception)*/
{
   //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  if ((_trnState != fsmINIT_SENT) && (_trnState != fsmACTIVE)) {
    log_error_state(t_cont_ind.getIdent());
    return;
  }

  if (_trnState == fsmINIT_SENT) {
    _rmtAddr = t_cont_ind.getOrigAddress();
    _trnUId.bindRemote(t_cont_ind.getOrigTrId());

    if (t_cont_ind.getAppCtx()) //Remote TCUser sets another appCtx
      _cfg._acOid = *t_cont_ind.getAppCtx();
    _trnState = fsmACTIVE;
  }
  _cfg._scspLink = t_cont_ind.getSCSPLink();
  //check components and create rejected component indication primitives if necessary
  verifyTCIndComponents(t_cont_ind.getCompList(), t_cont_ind.getIdent(), TComponentType::compAny);

  if (!notifyLocalTCUser(grdUser, t_cont_ind)) {
    _dlgMgr->notifyRmtTCUser(_trnUId, TDialogueAssociate::dsu_no_reason_given,
                             _cfg._scspLink, _cfg._ownAddr, _rmtAddr);
    stopDialogue();
    return;
  }
  //TODO: report all rejected components, for now only 1st one
  if (!_rejComps.empty() && !notifyLocalTCUser(grdUser, _rejComps.front())) {
    _dlgMgr->notifyRmtTCUser(_trnUId, TDialogueAssociate::dsu_no_reason_given,
                             _cfg._scspLink, _cfg._ownAddr, _rmtAddr);
    stopDialogue();
  }
}

void TDialogueFSM::updateDlgByIndication(TEndIndComposer & t_end_ind)
  /*throw(std::exception)*/
{
   //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  if ((_trnState != fsmINIT_SENT) && (_trnState != fsmACTIVE)) {
    log_error_state(t_end_ind.getIdent());
    return;
  }

  if (_trnState == fsmINIT_SENT) {
    _rmtAddr = t_end_ind.getOrigAddress();
    if (t_end_ind.getAppCtx()) //Remote TCUser sets another appCtx
      _cfg._acOid = *t_end_ind.getAppCtx();
  }
  _trnState = fsmDONE;
  _cfg._scspLink = t_end_ind.getSCSPLink();
  //check components and create rejected component indication primitives if necessary
  verifyTCIndComponents(t_end_ind.getCompList(), t_end_ind.getIdent(), TComponentType::compNotReturnable);

  if (!notifyLocalTCUser(grdUser, t_end_ind)) {
    stopDialogue();
    return;
  }
  //TODO: report all rejected components, for now only 1st one
  if (!_rejComps.empty())
    notifyLocalTCUser(grdUser, _rejComps.front());
  stopDialogue();
}

void TDialogueFSM::updateDlgByIndication(TUAbortIndComposer & t_uAbort_ind)
  /*throw(std::exception)*/
{
  //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  if ((_trnState == fsmIDLE) && (_trnState == fsmDONE)) {
    log_error_state(t_uAbort_ind.getIdent());
    return;
  }

  if (_trnState == fsmINIT_SENT) {
    _rmtAddr = t_uAbort_ind.getOrigAddress();
    if (t_uAbort_ind.getAppCtx()) //Remote TCUser sets another appCtx
      _cfg._acOid = *t_uAbort_ind.getAppCtx();
  }
  _trnState = fsmDONE;
  _cfg._scspLink = t_uAbort_ind.getSCSPLink();
  notifyLocalTCUser(grdUser, t_uAbort_ind);
  stopDialogue();
}

//Initiated either by local DialogCorrdinator or by SCCP Service Providec
void TDialogueFSM::updateDlgByIndication(TPAbortIndComposer & t_pAbort_ind)
  /*throw(std::exception)*/
{
  //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  _trnState = fsmDONE;
  _cfg._scspLink = t_pAbort_ind.getSCSPLink();
  notifyLocalTCUser(grdUser, t_pAbort_ind);
  stopDialogue();
}

//Initiated either by local DialogCorrdinator or by SCCP Service Providec
void TDialogueFSM::updateDlgByIndication(TNoticeIndComposer & t_notice_ind)
  /*throw(std::exception)*/
{
  //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  _cfg._scspLink = t_notice_ind.getSCSPLink();
  if (!notifyLocalTCUser(grdUser, t_notice_ind))
    stopDialogue();
}

//Initiated only by local DialogCorrdinator
void TDialogueFSM::updateDlgByIndication(TC_LCancelIndComposer & tc_ind)
  /*throw(std::exception)*/
{
  //ensure first that TC User has finished processing of previous indications
  TDlgIndHandlerGuard   grdUser(_dlgUser);
  MutexGuard            grd(_sync);

  if (!notifyLocalTCUser(grdUser, tc_ind) && (_trnState != fsmDONE)) {
    _dlgMgr->notifyRmtTCUser(_trnUId, TDialogueAssociate::dsu_no_reason_given,
                             _cfg._scspLink, _cfg._ownAddr, _rmtAddr);
    stopDialogue();
  }
}

}}}

