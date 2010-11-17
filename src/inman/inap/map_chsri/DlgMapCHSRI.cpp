#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
using smsc::inman::comp::chsri::CHSendRoutingInfoArg;
using smsc::inman::comp::chsri::MAP_CH_SRI_OpCode;
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
namespace chsri {
/* ************************************************************************** *
 * class MapCHSRIDlg implementation:
 * ************************************************************************** */
MapCHSRIDlg::MapCHSRIDlg(TCSessionMA * pSession, CHSRIhandlerITF * sri_handler,
                        Logger * uselog/* = NULL*/)
  : _selfRef(1), _dieing(false)
  , _logPfx("MapSRI"), dialog(NULL), session(pSession), logger(uselog)
  , sriHdl(sri_handler)
{
    _sriState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap");
    strcpy(_logId, _logPfx);
}

//May be called only after successfull Unbind() call
MapCHSRIDlg::~MapCHSRIDlg()
{
    endMapDlg();
}

void MapCHSRIDlg::destroy(void)
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

void MapCHSRIDlg::dieIfRequested(void)
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
bool MapCHSRIDlg::Unbind(void)
{
  MutexGuard  grd(_sync);
  return _dieing ? true : sriHdl.Unref();
}

//May be called only after successfull Unbind() call
void MapCHSRIDlg::endMapDlg(void)
{
  MutexGuard  grd(_sync);
  if (!_dieing)
    resetMapDlg();
}

/* ------------------------------------------------------------------------ *
 * MAP_SEND_ROUTING_INFO interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::reqRoutingInfo(const TonNpiAddress & tnpi_adr, uint16_t timeout/* = 0*/) throw(CustomException)
{
    MutexGuard  grd(_sync);

    if (!sriHdl.get() || !session)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapSRI", "invalid initialization");

    dialog = session->openDialog(tnpi_adr, logger);
    if (!dialog)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapSRI", "unable to create TC dialog");

    if (timeout)
        dialog->setInvokeTimeout(timeout);
    dialog->bindUser(this);
    sriId = dialog->getId();
    snprintf(_logId, sizeof(_logId)-1, "%s[%u:%Xh]", _logPfx,
             (unsigned)sriId.tcInstId, (unsigned)sriId.dlgId);


    CHSendRoutingInfoArg     arg;
    arg.setGMSCorSCFaddress(session->getOwnAdr());
    arg.setSubscrMSISDN(tnpi_adr);

    dialog->sendInvoke(MAP_CH_SRI_OpCode::sendRoutingInfo, &arg);  //throws
    dialog->beginDialog(); //throws
    _sriState.s.ctrInited = MapCHSRIDlg::operInited;

    smsc_log_debug(logger, "%s: quering %s info", _logId, tnpi_adr.getSignals());
}

void MapCHSRIDlg::reqRoutingInfo(const char * subcr_adr, uint16_t timeout/* = 0*/) throw(CustomException)
{
    TonNpiAddress   tnAdr;
    if (!tnAdr.fromText(subcr_adr))
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapSRI: inalid subscriber addr", subcr_adr);
    reqRoutingInfo(tnAdr, timeout);
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::onInvokeResultNL(InvokeRFP pInv, TcapEntity* res)
{
    MutexGuard  grd(_sync);
    if (_dieing)
      return;

    smsc_log_debug(logger, "%s: Invoke[%u:%u] got a returnResultNL: %u",
         _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
        (unsigned)res->getOpcode());

    _sriState.s.ctrResulted = MapCHSRIDlg::operInited;
    try { reqRes.mergeSegment(res->getParam());
    } catch (const CustomException & exc) {
        smsc_log_error(logger, "%s: %s", exc.what());
    }
}

void MapCHSRIDlg::onInvokeResult(InvokeRFP pInv, TcapEntity* res)
{
    unsigned do_end = 0;
    {
        MutexGuard  grd(_sync);
        smsc_log_debug(logger, "%s: Invoke[%u:%u] got a returnResult: %u",
             _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)res->getOpcode());

        _sriState.s.ctrInited = _sriState.s.ctrResulted = MapCHSRIDlg::operDone;
        try { reqRes.mergeSegment(res->getParam());
        } catch (const CustomException & exc) {
            smsc_log_error(logger, "%s: %s", exc.what());
        }
        if ((do_end = _sriState.s.ctrFinished) != 0)
            endTCap();
        if (!doRefHdl())
            return;
    }
    sriHdl->onMapResult(reqRes);
    if (do_end)
        sriHdl->onEndMapDlg(); //may call destroy()
    unRefHdl();
    dieIfRequested();
}

//Called if Operation got ResultError
void MapCHSRIDlg::onInvokeError(InvokeRFP pInv, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "%s: Invoke[%u:%u] got a returnError: %u",
             _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)resE->getOpcode());

        _sriState.s.ctrInited = MapCHSRIDlg::operDone;
        endTCap();
        if (!doRefHdl())
            return;
    }
     //may call destroy()
    sriHdl->onEndMapDlg(_RCS_MAPOpErrors->mkhash(resE->getOpcode()));
    unRefHdl();
    dieIfRequested();
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapCHSRIDlg::onInvokeLCancel(InvokeRFP pInv)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "%s: Invoke[%u:%u] got a LCancel",
             _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode());
        _sriState.s.ctrInited = MapCHSRIDlg::operFailed;
        endTCap();
        if (!doRefHdl())
            return;
    }
     //may call destroy()
    sriHdl->onEndMapDlg(_RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse));
    unRefHdl();
    dieIfRequested();
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void MapCHSRIDlg::onDialogContinue(bool compPresent)
{
    MutexGuard  grd(_sync);
    if (!compPresent) {
        smsc_log_error(logger, "%s: missing component in TC_CONT_IND", _logId);
    }
     //else wait for ongoing Invoke result/error
    return;
}

//TCAP indicates DialogPAbort: either due to TC layer error on SCF/HLR side or
//because of local TC dialog timeout is expired (no T_END_IND from HLR).
void MapCHSRIDlg::onDialogPAbort(uint8_t abortCause)
{
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
        smsc_log_error(logger, "%s: state 0x%x, P_ABORT: '%s'", _logId,
                        _sriState.value, _RCS_TC_PAbort->code2Txt(abortCause));
        endTCap();
        if (!doRefHdl())
            return;
    }
     //may call destroy()
    sriHdl->onEndMapDlg(_RCS_TC_PAbort->mkhash(abortCause));
    unRefHdl();
    dieIfRequested();
}

//SCF sent DialogUAbort (some logic error on SCF side).
void MapCHSRIDlg::onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                                    uint16_t userInfo_len, uint8_t *pUserInfo)
{
    uint32_t abortCause = (abortInfo_len == 1) ?
        *pAbortInfo : TC_UAbortCause::userDefinedAS;
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
        smsc_log_error(logger, "%s: state 0x%x, U_ABORT: '%s'", _logId,
                        _sriState.value, _RCS_TC_UAbort->code2Txt(abortCause));
        endTCap();
        if (!doRefHdl())
            return;
    }
     //may call destroy()
    sriHdl->onEndMapDlg(_RCS_TC_UAbort->mkhash(abortCause));
    unRefHdl();
    dieIfRequested();
}

//Underlying layer unable to deliver message, just abort dialog
void MapCHSRIDlg::onDialogNotice(uint8_t reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        uint8_t invId/* = 0*/, uint8_t opCode/* = 0*/)
{
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
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
                       _sriState.value, _RCS_TC_Report->code2Txt(reportCause),
                       dstr.c_str());
        endTCap();
        if (!doRefHdl())
            return;
    }
     //may call destroy()
    sriHdl->onEndMapDlg(_RCS_TC_Report->mkhash(reportCause));
    unRefHdl();
    dieIfRequested();
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapCHSRIDlg::onDialogREnd(bool compPresent)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrFinished = 1;
        if (compPresent)
            return; //wait for ongoing Invoke result/error

        endTCap();
        if (!_sriState.s.ctrResulted) {
            smsc_log_error(logger, "%s: T_END_IND, state 0x%x", _logId, _sriState.value);
            errcode = _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse);
        }
        if (!doRefHdl())
            return;
    }
     //may call destroy()
    sriHdl->onEndMapDlg(errcode);
    unRefHdl();
    dieIfRequested();
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//NOTE: _sync must be locked upon entry
bool MapCHSRIDlg::doRefHdl(void)
{
  if (sriHdl.Lock()) {
    ++_selfRef;
    return true;
  }
  return false;
}

void MapCHSRIDlg::unRefHdl(void)
{
    MutexGuard tmp(_sync);
    sriHdl.UnLock();
    if (sriHdl.get())
        sriHdl->Awake();
}

void MapCHSRIDlg::resetMapDlg(void)
{
  if (sriHdl.get()) {
    smsc_log_warn(logger, "%s: resetMapDlg(): reference to CH-SRI handler exists", _logId);
  }
  sriHdl.Reset(NULL);
  endTCap();
}

//Ends TC dialog, releases Dialog()
//NOTE: _sync MUST BE locked upon entry
void MapCHSRIDlg::endTCap(void)
{
    while (dialog && !dialog->unbindUser()) //TCDlg refers this object
        _sync.wait();

    if (dialog) {
        if (!dialog->checkState(TCDlgState::dlgClosed)) {
            try {  // do TC_PREARRANGED if still active
                dialog->endDialog((_sriState.s.ctrInited < MapCHSRIDlg::operDone) ?
                                    Dialog::endPrearranged : Dialog::endBasic);
                smsc_log_debug(logger, "%s: T_END_REQ, state: 0x%x", _logId,
                                _sriState.value);
            } catch (const std::exception & exc) {
                smsc_log_error(logger, "%s: T_END_REQ: %s", _logId, exc.what());
                dialog->releaseAllInvokes();
            }
        }
        session->releaseDialog(dialog);
        dialog = NULL;
    }
}

} //chsri
} //inap
} //inman
} //smsc

