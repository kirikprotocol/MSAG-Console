#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/map_atih/DlgMapATSI.hpp"
using smsc::inman::comp::atih::MAP_ATIH;

namespace smsc {
namespace inman {
namespace inap {
namespace atih {
/* ************************************************************************** *
 * class MapATSIDlg implementation:
 * ************************************************************************** */

//Sets TC Dialog registry and MAP user for this dialog
void MapATSIDlg::init(TCSessionMA & tc_sess, ATSIhandlerITF & res_hdl, Logger * use_log/* = NULL*/)
{
  MutexGuard  grd(_sync);
  _tcSess = &tc_sess;
  bindUser(res_hdl);
  if (use_log)
    _logger = use_log;
}

/* ------------------------------------------------------------------------ *
 * ATSIcontractor interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::subsciptionInterrogation(const RequestedSubscription & req_cfg,
                                  const TonNpiAddress & subcr_adr,
                                  uint16_t timeout_sec/* = 0*/)
  throw(CustomException)
{
  MutexGuard  grd(_sync);

  if (!atsiHdl() || !_tcSess)
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  _logPfx, "invalid initialization");

  if (!subcr_adr.interISDN())
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapATSI: invalid subscriber addr", subcr_adr.getSignals());

  _tcDlg = _tcSess->openDialog(subcr_adr, _logger);
  if (!_tcDlg)
    throw CustomException((int32_t)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                          _logPfx, "unable to create TC dialog");
  bindTCDialog(timeout_sec);

  ATSIArg     arg(req_cfg, _logger);
  arg.setSCFaddress(_tcSess->getOwnAdr());
  arg.setSubscriberId(subcr_adr);

  _tcDlg->sendInvoke(MAP_ATIH::anyTimeSubscriptionInterrogation, &arg); //throws
  _tcDlg->beginDialog(); //throws
  _ctrState.s.ctrInited = MapDialogAC::operInited;
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::onInvokeResultNL(InvokeRFP pInv, TcapEntity* res)
{
  MutexGuard  grd(_sync);
  smsc_log_warn(_logger, "%s: unexoected TC_RESULT_NL_IND for %s", _logId,
                pInv->idStr().c_str());
}

void MapATSIDlg::onInvokeResult(InvokeRFP pInv, TcapEntity * res)
{
  unsigned do_end = 0;
  ATSIRes * resComp = static_cast<ATSIRes *>(res->getParam());
  {
    MutexGuard  grd(_sync);
    if (resComp)
      smsc_log_debug(_logger, "%s: %s got a returnResult(%u)",
                    _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());
    else
      smsc_log_error(_logger, "%s: %s got a returnResult(%u) with missing parameter",
                    _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());

    _ctrState.s.ctrResulted = 1;
    _ctrState.s.ctrInited = MapDialogAC::operDone;
    if ((do_end = _ctrState.s.ctrFinished) != 0)
      unbindTCDialog();
    if (!doRefHdl())
      return;
  }
  if (resComp)
    atsiHdl()->onATSIResult(*resComp);
  if (do_end)
    atsiHdl()->onDialogEnd(); //may call releaseThis()
  unRefHdl(); //die if requested(
}

} //atih
} //inap
} //inman
} //smsc

