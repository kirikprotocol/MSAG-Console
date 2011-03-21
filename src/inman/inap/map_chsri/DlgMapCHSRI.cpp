#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
using smsc::inman::comp::chsri::CHSendRoutingInfoArg;
using smsc::inman::comp::chsri::MAP_CH_SRI_OpCode;

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {
/* ************************************************************************** *
 * class MapCHSRIDlg implementation:
 * ************************************************************************** */

//Sets TC Dialog registry and MAP user for this dialog
void MapCHSRIDlg::init(TCSessionMA & tc_sess, CHSRIhandlerITF & res_hdl, Logger * use_log/* = NULL*/)
{
  MutexGuard  grd(_sync);
  _tcSess = &tc_sess;
  bindUser(res_hdl);
  if (use_log)
    _logger = use_log;
}

/* ------------------------------------------------------------------------ *
 * MAP_SEND_ROUTING_INFO interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::reqRoutingInfo(const TonNpiAddress & tnpi_adr, uint16_t timeout_sec/* = 0*/)
  throw(CustomException)
{
  MutexGuard  grd(_sync);

  if (!sriHdl() || !_tcSess)
    throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapSRI", "invalid initialization");

  _tcDlg = _tcSess->openDialog(tnpi_adr, _logger);
  if (!_tcDlg)
      throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapSRI", "unable to create TC dialog");

  bindTCDialog(timeout_sec);

  CHSendRoutingInfoArg     arg;
  arg.setGMSCorSCFaddress(_tcSess->getOwnAdr());
  arg.setSubscrMSISDN(tnpi_adr);

  _tcDlg->sendInvoke(MAP_CH_SRI_OpCode::sendRoutingInfo, &arg);  //throws
  _tcDlg->beginDialog(); //throws
  _ctrState.s.ctrInited = MapCHSRIDlg::operInited;
}

void MapCHSRIDlg::reqRoutingInfo(const char * subcr_adr, uint16_t timeout_sec/* = 0*/)
  throw(CustomException)
{
  TonNpiAddress   tnAdr;
  if (!tnAdr.fromText(subcr_adr))
    throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                  "MapSRI: inalid subscriber addr", subcr_adr);
  reqRoutingInfo(tnAdr, timeout_sec);
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::onInvokeResultNL(InvokeRFP pInv, TcapEntity* res)
{
  MutexGuard  grd(_sync);
  smsc_log_debug(_logger, "%s: %s got a returnResultNL: %u",
                 _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());

  _ctrState.s.ctrResulted = MapDialogAC::operInited;
  try { _reqRes.mergeSegment(res->getParam());
  } catch (const CustomException & exc) {
    smsc_log_error(_logger, "%s: %s", _logId, exc.what());
  }
}

void MapCHSRIDlg::onInvokeResult(InvokeRFP pInv, TcapEntity * res)
{
  unsigned do_end = 0;
  CHSendRoutingInfoRes * resComp = static_cast<CHSendRoutingInfoRes *>(res->getParam());
  {
    MutexGuard  grd(_sync);
    if (resComp)
      smsc_log_debug(_logger, "%s: %s got a returnResult(%u)",
                    _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());
    else
      smsc_log_error(_logger, "%s: %s got a returnResult(%u) with missing parameter",
                    _logId, pInv->idStr().c_str(), (unsigned)res->getOpcode());

    _ctrState.s.ctrInited = _ctrState.s.ctrResulted = MapDialogAC::operDone;
    try { _reqRes.mergeSegment(res->getParam());
    } catch (const CustomException & exc) {
      smsc_log_error(_logger, "%s: %s", _logId, exc.what());
    }
    if ((do_end = _ctrState.s.ctrFinished) != 0)
      unbindTCDialog();
    if (!doRefUser())
      return;
  }
  sriHdl()->onMapResult(_reqRes);

  if (!do_end) {
    MutexGuard  grd(_sync);
    doUnrefUser();
  } else {
    //may call releaseThis()/finalizeObj();
    if (_resHdl->onDialogEnd(*this) != ObjFinalizerIface::objDestroyed)
      unRefAndDie(); //die if requested
  }
}

} //chsri
} //inap
} //inman
} //smsc

