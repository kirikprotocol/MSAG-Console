#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/map/DlgMapAC.hpp"
using smsc::core::synchronization::MutexGuard;

#include "inman/comp/MapOpErrors.hpp"
using smsc::inman::comp::_RCS_MAPOpErrors;
using smsc::inman::comp::MAPServiceRC;
using smsc::inman::comp::_RCS_MAPService;

#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::TC_PAbortCause;
using smsc::inman::inap::_RCS_TC_PAbort;
using smsc::inman::inap::TC_UAbortCause;
using smsc::inman::inap::_RCS_TC_UAbort;
using smsc::inman::inap::_RCS_TC_Report;

using namespace smsc::util;

namespace smsc {
namespace inman {
namespace inap {

/* ************************************************************************** *
 * class MapDialogAC implementation:
 * ************************************************************************** */
MapDialogAC::MapDialogAC(const char * use_ident, Logger * use_log/* = NULL*/)
  : _delThis(false), _logPfx(use_ident), _logger(use_log), _tcDlg(NULL)
{
  _ctrState.value = 0;
  if (!_logger)
    _logger = Logger::getInstance("smsc.inman.inap");
  _thisRefs.addRef(MapDialogAC::refIdItself);
  strcpy(_logId, _logPfx);
}

MapDialogAC::~MapDialogAC()
{
  MutexGuard  grd(_sync);
  _thisRefs.unRef(MapDialogAC::refIdItself);
  _delThis = false;
  unbindTCDialog();

  if (_resHdl.getNext()) {
    smsc_log_warn(_logger, "%s: result handler still binded!", _logId);
//    _resHdl.Reset(NULL);
  }
  if (_thisRefs.hasRefs()) {
    smsc_log_error(_logger, "%s: ~MapDialogAC() refs present: {%s}", _logId, _thisRefs.toString().c_str());
  }
  smsc_log_debug(_logger, "%s: destroyed", _logId);
}

ObjAllcStatus_e MapDialogAC::releaseThis(void)
{
  bool doDel = false;
  {
    MutexGuard  grd(_sync);
    _thisRefs.unRef(MapDialogAC::refIdItself);
    _delThis = true;
    doDel = !_thisRefs.hasRefs();
  }
  if (doDel) {
    delete this;
    return ObjFinalizerIface::objDestroyed;
  }
  return ObjFinalizerIface::objActive;
}

void MapDialogAC::endDialog(bool do_release/* = true*/)
{
  MutexGuard  grd(_sync);
  if (do_release)
    unbindTCDialog();
  else
    endTCDialog();
}

bool MapDialogAC::unbindUser(void)
{
  MutexGuard  grd(_sync);
  if (_resHdl.Unref()) {
    _thisRefs.unRef(MapDialogAC::refIdHandler);
    return true;
  }
  return false;
}

//Unlocks reference to MAP User, notifies it aboud dialog end if required
//and destroys this object if it's released.
//Note: _sync MUST be unlocked upon entry!
void MapDialogAC::unRefUserNotify(bool do_end, RCHash end_rc)
{
  if (!do_end) {
    MutexGuard  grd(_sync);
    doUnrefUser();
  } else {
    unRefUserAndDie(end_rc);
  }
}

//Unrefs and unlocks result handler and destroys this object if it's released
//Note: _sync MUST be unlocked upon entry!
void MapDialogAC::unRefUserAndDie(RCHash end_rc)
{
  //may call releaseThis()/finalizeObj()
  if (_resHdl->onDialogEnd(*this, end_rc) != ObjFinalizerIface::objDestroyed) {
    if ((finalizeObj() == ObjFinalizerIface::objFinalized)
        && _delThis && !_thisRefs.hasRefs()) //die if requested(
      delete this;
  }
}

/* ------------------------------------------------------------------------ *
 * ObjFinalizerIface interface methods
 * ------------------------------------------------------------------------ */
//Unrefs and unlocks result handler
ObjAllcStatus_e MapDialogAC::finalizeObj(void)
{
  MutexGuard tmp(_sync);
  doUnrefUser();
  return _thisRefs.hasRefsExcept(MapDialogAC::refIdItself) ?
            ObjFinalizerIface::objActive : ObjFinalizerIface::objFinalized;
}

/* ------------------------------------------------------------------------ *
 * NOTE: _sync must be locked upon entry to following methods
 * ------------------------------------------------------------------------ */
void MapDialogAC::bindUser(MapDlgUserIface & res_hdl)
{
  _resHdl.Reset(&res_hdl);
  _thisRefs.addRef(MapDialogAC::refIdHandler);
}

//Binds TC Dialog
void MapDialogAC::bindTCDialog(uint16_t timeout_sec/* = 0*/)
{
  //xompose logId string, that depends on TC Dialog id.
  snprintf(_logId, sizeof(_logId)-1, "%s[%u:%Xh]", _logPfx,
           (unsigned)_tcDlg->getId().tcInstId, (unsigned)_tcDlg->getId().dlgId);

  if (timeout_sec)
    _tcDlg->setInvokeTimeout(timeout_sec);

  _tcDlg->bindUser(this);
  _thisRefs.addRef(MapDialogAC::refIdTCDialog);
}

//Locks reference to MAP user.
//Returns false if result handler is not set.
bool MapDialogAC::doRefUser(void)
{
  if (_resHdl.get() && _resHdl.Lock()) {
    _thisRefs.addRef(MapDialogAC::refIdHandler);
    return true;
  }
  return false;
}

//Unlocks reference to MAP user.
void MapDialogAC::doUnrefUser(void)
{
  if (_resHdl.getNext())
    _resHdl->Awake();
  _resHdl.UnLock();
  _thisRefs.unRef(MapDialogAC::refIdHandler);
}

//ends TC dialog
void MapDialogAC::endTCDialog(void)
{
  if (_tcDlg) {
    if (!_tcDlg->checkState(TCDlgState::dlgClosed)) {
      smsc_log_debug(_logger, "%s: T_END_REQ, state: 0x%x", _logId, _ctrState.value);
      Dialog::Ending doEnd = (_ctrState.s.ctrInited == MapDialogAC::operInited) ?
                              Dialog::endUAbort : Dialog::endPrearranged;
      try {
        _tcDlg->endDialog(doEnd);
      } catch (const std::exception & exc) {
        smsc_log_error(_logger, "%s: TC_END_REQ %s", _logId, exc.what());
      }
    }
    _tcDlg->releaseAllInvokes();
  }
}

//ends TC dialog and releases TC dialog object
void MapDialogAC::unbindTCDialog(void)
{
  while (_tcDlg && !_tcDlg->unbindUser()) { //TCDlg refers this object
    //prevent object from being deleted by other thread
    _thisRefs.addRef(MapDialogAC::refIdItself);
    _sync.wait();
    _thisRefs.unRef(MapDialogAC::refIdItself);
  }
  endTCDialog();

  if (_tcDlg) {
    rlseTCDialog();
    _tcDlg = NULL;
    _thisRefs.unRef(MapDialogAC::refIdTCDialog);
  }
}


/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
void MapDialogAC::onDialogInvoke(Invoke * op, bool lastComp)
{
  MutexGuard  grd(_sync);
  smsc_log_warn(_logger, "%s: unexoected TC_INVOKE_IND as %s", _logId, op->idStr().c_str());
}

void MapDialogAC::onDialogContinue(bool compPresent)
{
  MutexGuard  grd(_sync);
  if (!compPresent) {
    smsc_log_warn(_logger, "%s: missing component in TC_CONT_IND", _logId);
  } //else wait for ongoing Invoke result/error
}

//TCAP indicates DialogPAbort: either due to TC layer error on one of sides or
//because of local TC dialog timeout is expired (no T_END_IND from remote peer).
void MapDialogAC::onDialogPAbort(uint8_t abortCause)
{
  {
    MutexGuard  grd(_sync);
    _ctrState.s.ctrAborted = 1;
    smsc_log_error(_logger, "%s: state 0x%x, P_ABORT: '%s'", _logId,
                    _ctrState.value, _RCS_TC_PAbort->code2Txt(abortCause));
    unbindTCDialog();
    if (!doRefUser())
      return;
  }
  unRefUserAndDie(_RCS_TC_PAbort->mkhash(abortCause)); //die if requested(
}

//SCF sent DialogUAbort (some logic error on SCF side).
void MapDialogAC::onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                                    uint16_t userInfo_len, uint8_t *pUserInfo)
{
  uint32_t abortCause = (abortInfo_len == 1) ?
                        *pAbortInfo : TC_UAbortCause::userDefinedAS;
  {
    MutexGuard  grd(_sync);
    _ctrState.s.ctrAborted = 1;
    smsc_log_error(_logger, "%s: state 0x%x, U_ABORT: '%s'", _logId,
                    _ctrState.value, _RCS_TC_UAbort->code2Txt(abortCause));
    unbindTCDialog();
    if (!doRefUser())
      return;
  }
  unRefUserAndDie(_RCS_TC_UAbort->mkhash(abortCause)); //die if requested(
}

//Underlying layer unable to deliver message, just abort dialog
void MapDialogAC::onDialogNotice(uint8_t reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        uint8_t invId/* = 0*/, uint8_t opCode/* = 0*/)
{
  {
    MutexGuard  grd(_sync);
    _ctrState.s.ctrAborted = 1;
    std::string dstr;
    if (comp_kind != TcapEntity::tceNone) {
      format(dstr, "Invoke[%u]", invId);
      switch (comp_kind) {
      case TcapEntity::tceError:      dstr += ".Error"; break;
      case TcapEntity::tceResult:     dstr += ".Result"; break;
      case TcapEntity::tceResultNL:   dstr += ".ResultNL"; break;
      default:;
      }
      dstr += " not delivered.";
    }
    smsc_log_error(_logger, "%s: state 0x%x, NOTICE_IND: '%s', %s", _logId,
                   _ctrState.value, _RCS_TC_Report->code2Txt(reportCause),
                   dstr.c_str());
    unbindTCDialog();
    if (!doRefUser())
      return;
  }
  unRefUserAndDie(_RCS_TC_Report->mkhash(reportCause)); //die if requested(
}


//Remote peer sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on remote side.
void MapDialogAC::onDialogREnd(bool compPresent)
{
  RCHash  errcode = 0;
  {
    MutexGuard  grd(_sync);
    _ctrState.s.ctrFinished = 1;
    if (compPresent)
      return; //wait for ongoing Invoke result/error

    unbindTCDialog();
    if (!_ctrState.s.ctrResulted) {
      smsc_log_error(_logger, "%s: T_END_IND, state 0x%x", _logId, _ctrState.value);
      errcode = _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse);
    }
    if (!doRefUser())
      return;
  }
  unRefUserAndDie(errcode); //die if requested(
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */

//Called if Operation got ResultError
void MapDialogAC::onInvokeError(InvokeRFP pInv, TcapEntity * resE)
{
  {
    MutexGuard  grd(_sync);
    smsc_log_error(_logger, "%s: %s got a returnError(%u)", _logId,
                   pInv->idStr().c_str(), (unsigned)resE->getOpcode());

    _ctrState.s.ctrInited = MapDialogAC::operDone;
    unbindTCDialog();
    if (!doRefUser())
      return;
  }
  unRefUserAndDie(_RCS_MAPOpErrors->mkhash(resE->getOpcode())); //die if requested(
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapDialogAC::onInvokeLCancel(InvokeRFP pInv)
{
  {
    MutexGuard  grd(_sync);
    smsc_log_error(_logger, "%s: %s got a LCancel", _logId, pInv->idStr().c_str());
    _ctrState.s.ctrInited = MapDialogAC::operFailed;
    unbindTCDialog();
    if (!doRefUser())
      return;
  }
  unRefUserAndDie(_RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse)); //die if requested(
}


} //inap
} //inman
} //smsc

