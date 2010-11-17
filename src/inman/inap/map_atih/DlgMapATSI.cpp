#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/map_atih/DlgMapATSI.hpp"
using smsc::inman::comp::atih::MAP_ATIH;
using smsc::inman::comp::_RCS_MAPOpErrors;
using smsc::inman::comp::MAPServiceRC;
using smsc::inman::comp::_RCS_MAPService;

#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::TC_PAbortCause;
using smsc::inman::inap::_RCS_TC_PAbort;
using smsc::inman::inap::TC_UAbortCause;
using smsc::inman::inap::_RCS_TC_UAbort;
using smsc::inman::inap::_RCS_TC_Report;

namespace smsc {
namespace inman {
namespace inap {
namespace atih {
/* ************************************************************************** *
 * class MapATSIDlg implementation:
 * ************************************************************************** */
MapATSIDlg::MapATSIDlg(TCSessionMA* pSession, ATSIhandlerITF * atsi_handler,
                        Logger * uselog/* = NULL*/)
    : _selfRef(1), _dieing(false)
    , _logPfx("MapATSI"), dialog(NULL), session(pSession), atsiHdl(atsi_handler)
    , logger(uselog)
{
  _atsiState.value = 0;
  if (!logger)
    logger = Logger::getInstance("smsc.inman.Inap.MapATSIDlg");
  strcpy(_logId, _logPfx);
}

MapATSIDlg::~MapATSIDlg()
{
  endMapDlg();
}

void MapATSIDlg::destroy(void)
{
  bool doDel = false;
  {
    MutexGuard  grd(_sync);
    if (_selfRef) {
      if (!(--_selfRef))
        doDel = (_dieing = true);
    }
  }
  if (doDel)
    delete this;
}

void MapATSIDlg::dieIfRequested(void)
{
  bool doDel = false;
  {
    MutexGuard  grd(_sync);
    doDel = _dieing;
  }
  if (doDel)
    delete this;
}

//Attempts to unbind TC User.
//Returns true on succsess, false result means that this object has 
//established references to handler.
bool MapATSIDlg::Unbind(void)
{
  MutexGuard  grd(_sync);
  return _dieing ? true : atsiHdl.Unref();
}

void MapATSIDlg::endMapDlg(void)
{
  MutexGuard  grd(_sync);
  if (!_dieing)
    resetMapDlg();
}

/* ------------------------------------------------------------------------ *
 * ATSIcontractor interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::subsciptionInterrogation(const RequestedSubscription & req_cfg,
                                  const TonNpiAddress & subcr_adr,
                                  uint16_t timeout/* = 0*/)
    throw(CustomException)
{
  MutexGuard  grd(_sync);

  if (!atsiHdl.get() || !session)
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapATSI", "invalid initialization");

  if (!subcr_adr.interISDN())
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapATSI: invalid subscriber addr", subcr_adr.getSignals());

  dialog = session->openDialog(subcr_adr, logger);
  if (!dialog)
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapATSI", "unable to create TC dialog");

  if (timeout)
    dialog->setInvokeTimeout(timeout);
  dialog->bindUser(this);
  atsiId = dialog->getId();
  snprintf(_logId, sizeof(_logId)-1, "%s[%u:%Xh]", _logPfx,
           (unsigned)atsiId.tcInstId, (unsigned)atsiId.dlgId);

  ATSIArg     arg(req_cfg, logger);
  arg.setSCFaddress(session->getOwnAdr());
  arg.setSubscriberId(subcr_adr);

  dialog->sendInvoke(MAP_ATIH::anyTimeSubscriptionInterrogation, &arg); //throws
  dialog->beginDialog(); //throws
  _atsiState.s.ctrInited = MapATSIDlg::operInited;
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::onInvokeResult(InvokeRFP pInv, TcapEntity* res)
{
  unsigned do_end = 0;
  ATSIRes * resComp = static_cast<ATSIRes *>(res->getParam());
  {
    MutexGuard  grd(_sync);
    if (resComp)
      smsc_log_debug(logger, "%s: Invoke[%u:%u] got a returnResult(%u)",
                    _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
                    (unsigned)res->getOpcode());
    else
      smsc_log_error(logger, "%s: Invoke[%u:%u] got a returnResult(%u) with missing parameter",
                    _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
                    (unsigned)res->getOpcode());

    _atsiState.s.ctrResulted = 1;
    _atsiState.s.ctrInited = MapATSIDlg::operDone;
    if ((do_end = _atsiState.s.ctrFinished) != 0)
      endTCap();
    if (!doRefHdl())
      return;
  }
  if (resComp)
    atsiHdl->onATSIResult(*resComp);
  if (do_end)
    atsiHdl->onEndATSI(); //may call destroy()
  unRefHdl();
  dieIfRequested();
}

//Called if Operation got ResultError
void MapATSIDlg::onInvokeError(InvokeRFP pInv, TcapEntity * resE)
{
  {
    MutexGuard  grd(_sync);
    smsc_log_error(logger, "%s: Invoke[%u:%u] got a returnError(%u)",
        _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
        (unsigned)resE->getOpcode());

    _atsiState.s.ctrInited = MapATSIDlg::operDone;
    endTCap();
    if (!doRefHdl())
      return;
  }
  //may call destroy()
  atsiHdl->onEndATSI(_RCS_MAPOpErrors->mkhash(resE->getOpcode()));
  unRefHdl();
  dieIfRequested();
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapATSIDlg::onInvokeLCancel(InvokeRFP pInv)
{
  {
    MutexGuard  grd(_sync);
    smsc_log_error(logger, "%s: Invoke[%u:%u] got a LCancel",
        _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode());
    _atsiState.s.ctrInited = MapATSIDlg::operFailed;
    endTCap();
    if (!doRefHdl())
      return;
  }
//may call destroy()
  atsiHdl->onEndATSI(_RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse));
  unRefHdl();
  dieIfRequested();
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void MapATSIDlg::onDialogContinue(bool compPresent)
{
  MutexGuard  grd(_sync);
  if (!compPresent) {
    smsc_log_warn(logger, "%s: missing component in TC_CONT_IND", _logId);
  }
   //else wait for ongoing Invoke result/error
  return;
}

//TCAP indicates DialogPAbort: either due to TC layer error on SCF/HLR side or
//because of local TC dialog timeout is expired (no T_END_IND from HLR).
void MapATSIDlg::onDialogPAbort(uint8_t abortCause)
{
  {
    MutexGuard  grd(_sync);
    _atsiState.s.ctrAborted = 1;
    smsc_log_error(logger, "%s: state 0x%x, P_ABORT: '%s'", _logId,
                    _atsiState.value, _RCS_TC_PAbort->code2Txt(abortCause));
    endTCap();
    if (!doRefHdl())
      return;
  }
  //may call destroy()
  atsiHdl->onEndATSI(_RCS_TC_PAbort->mkhash(abortCause));
  unRefHdl();
  dieIfRequested();
}

//SCF sent DialogUAbort (some logic error on SCF side).
void MapATSIDlg::onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                                    uint16_t userInfo_len, uint8_t *pUserInfo)
{
  uint32_t abortCause = (abortInfo_len == 1) ?
      *pAbortInfo : TC_UAbortCause::userDefinedAS;
  {
    MutexGuard  grd(_sync);
    _atsiState.s.ctrAborted = 1;
    smsc_log_error(logger, "%s: state 0x%x, U_ABORT: '%s'", _logId,
                    _atsiState.value, _RCS_TC_UAbort->code2Txt(abortCause));
    endTCap();
    if (!doRefHdl())
      return;
  }
  //may call destroy()
  atsiHdl->onEndATSI(_RCS_TC_UAbort->mkhash(abortCause));
  unRefHdl();
  dieIfRequested();
}

//Underlying layer unable to deliver message, just abort dialog
void MapATSIDlg::onDialogNotice(uint8_t reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        uint8_t invId/* = 0*/, uint8_t opCode/* = 0*/)
{
  {
    MutexGuard  grd(_sync);
    _atsiState.s.ctrAborted = 1;
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
    smsc_log_error(logger, "%s: state 0x%x, NOTICE_IND: '%s', %s", _logId,
                   _atsiState.value, _RCS_TC_Report->code2Txt(reportCause),
                   dstr.c_str());
    endTCap();
    if (!doRefHdl())
      return;
  }
  //may call destroy()
  atsiHdl->onEndATSI(_RCS_TC_Report->mkhash(reportCause));
  unRefHdl();
  dieIfRequested();
}


//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapATSIDlg::onDialogREnd(bool compPresent)
{
  RCHash  errcode = 0;
  {
    MutexGuard  grd(_sync);
    _atsiState.s.ctrFinished = 1;
    if (compPresent)
      return; //wait for ongoing Invoke result/error

    endTCap();
    if (!_atsiState.s.ctrResulted) {
      smsc_log_error(logger, "%s: T_END_IND, state 0x%x", _logId, _atsiState.value);
      errcode = _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse);
    }
    if (!doRefHdl())
      return;
  }
  //may call destroy()
  atsiHdl->onEndATSI(errcode);
  unRefHdl();
  dieIfRequested();
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//NOTE: _sync must be locked upon entry
bool MapATSIDlg::doRefHdl(void)
{
  if (atsiHdl.Lock()) {
    ++_selfRef;
    return true;
  }
  return false;
}

void MapATSIDlg::unRefHdl(void)
{
  MutexGuard tmp(_sync);
  --_selfRef;
  atsiHdl.UnLock();
  if (atsiHdl.get())
    atsiHdl->Awake();
}

void MapATSIDlg::resetMapDlg(void)
{
  if (atsiHdl.get()) {
    smsc_log_warn(logger, "%s: resetMapDlg(): reference to ATSI handler exists", _logId);
  }
  atsiHdl.Reset(NULL);
  endTCap();
}


//ends TC dialog, releases Dialog()
void MapATSIDlg::endTCap(void)
{
  while (dialog && !dialog->unbindUser()) //TCDlg refers this object
    _sync.wait();

  if (dialog) {
    if (!dialog->checkState(TCDlgState::dlgClosed)) {
      try {    // do TC_PREARRANGED if still active
        dialog->endDialog((_atsiState.s.ctrInited < MapATSIDlg::operDone) ?
                            Dialog::endPrearranged : Dialog::endBasic);
        smsc_log_debug(logger, "%s: T_END_REQ, state: 0x%x", _logId,
                        _atsiState.value);
      } catch (const std::exception & exc) {
        smsc_log_error(logger, "%s: T_END_REQ: %s", _logId, exc.what());
        dialog->releaseAllInvokes();
      }
    }
    session->releaseDialog(dialog);
    dialog = NULL;
  }
}

} //atih
} //inap
} //inman
} //smsc

