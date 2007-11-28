#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <assert.h>

#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
using smsc::inman::comp::chsri::CHSendRoutingInfoArg;
using smsc::inman::comp::chsri::MAP_CH_SRI_OpCode;
using smsc::inman::comp::_RCS_MAPOpErrors;
using smsc::inman::comp::MAPServiceRC;
using smsc::inman::comp::_RCS_MAPService;

#include "inman/inap/TCAPErrors.hpp"
using smsc::inman::inap::TC_AbortCause;
using smsc::inman::inap::_RCS_TC_Abort;
using smsc::inman::inap::_RCS_TC_Report;

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {
/* ************************************************************************** *
 * class MapCHSRIDlg implementation:
 * ************************************************************************** */
MapCHSRIDlg::MapCHSRIDlg(TCSessionMA* pSession, CHSRIhandlerITF * sri_handler,
                        Logger * uselog/* = NULL*/)
    : sriHdl(sri_handler), session(pSession), sriId(0), dialog(NULL)
    , logger(uselog)
{
    assert(sri_handler && pSession);
    _sriState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap");
}

MapCHSRIDlg::~MapCHSRIDlg()
{
    endMapDlg();
}

void MapCHSRIDlg::endMapDlg(void)
{
    MutexGuard  grd(_sync);
    sriHdl = NULL;
    endTCap(true);
}

/* ------------------------------------------------------------------------ *
 * MAP_SEND_ROUTING_INFO interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::reqRoutingInfo(const TonNpiAddress & tnpi_adr, USHORT_T timeout/* = 0*/) throw(CustomException)
{
    MutexGuard  grd(_sync);
    dialog = session->openDialog(tnpi_adr);
    if (!dialog)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapSRI", "unable to create TC dialog");

    if (timeout)
        dialog->setInvokeTimeout(timeout);
    dialog->bindUser(this);
    sriId = (unsigned)dialog->getId();

    CHSendRoutingInfoArg     arg;
    arg.setGMSCorSCFaddress(session->getOwnAdr());
    arg.setSubscrMSISDN(tnpi_adr);

    dialog->sendInvoke(MAP_CH_SRI_OpCode::sendRoutingInfo, &arg);  //throws
    dialog->beginDialog(); //throws
    _sriState.s.ctrInited = MapCHSRIDlg::operInited;

    smsc_log_debug(logger, "MapSRI[%u]: quering %s info", sriId, tnpi_adr.getSignals());
}

void MapCHSRIDlg::reqRoutingInfo(const char * subcr_adr, USHORT_T timeout/* = 0*/) throw(CustomException)
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
    smsc_log_debug(logger, "MapSRI[%u]: Invoke[%u:%u] got a returnResultNL: %u",
        sriId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
        (unsigned)res->getOpcode());

    _sriState.s.ctrResulted = MapCHSRIDlg::operInited;
    try { reqRes.mergeSegment(res->getParam());
    } catch (const CustomException & exc) {
        smsc_log_error(logger, "MapSRI[%u]: %s", exc.what());
    }
}

void MapCHSRIDlg::onInvokeResult(InvokeRFP pInv, TcapEntity* res)
{
    unsigned do_end = 0;
    {
        MutexGuard  grd(_sync);
        smsc_log_debug(logger, "MapSRI[%u]: Invoke[%u:%u] got a returnResult: %u",
            sriId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)res->getOpcode());

        _sriState.s.ctrInited = _sriState.s.ctrResulted = MapCHSRIDlg::operDone;
        try { reqRes.mergeSegment(res->getParam());
        } catch (const CustomException & exc) {
            smsc_log_error(logger, "MapSRI[%u]: %s", exc.what());
        }
        sriHdl->onMapResult(&reqRes);
        if ((do_end = _sriState.s.ctrFinished) != 0)
            endTCap();
    }
    if (do_end)
        sriHdl->onEndMapDlg();
}

//Called if Operation got ResultError
void MapCHSRIDlg::onInvokeError(InvokeRFP pInv, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapSRI[%u]: Invoke[%u:%u] got a returnError: %u",
            sriId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)resE->getOpcode());

        _sriState.s.ctrInited = MapCHSRIDlg::operDone;
        endTCap();
    }
    sriHdl->onEndMapDlg(_RCS_MAPOpErrors->mkhash(resE->getOpcode()));
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapCHSRIDlg::onInvokeLCancel(InvokeRFP pInv)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapSRI[%u]: Invoke[%u:%u] got a LCancel",
            sriId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode());
        _sriState.s.ctrInited = MapCHSRIDlg::operFailed;
        endTCap();
    }
    sriHdl->onEndMapDlg(_RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse));
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void MapCHSRIDlg::onDialogContinue(bool compPresent)
{
    MutexGuard  grd(_sync);
    if (!compPresent)
        smsc_log_error(logger, "MapSRI[%u]: missing component in TC_CONT_IND", sriId);
     //else wait for ongoing Invoke result/error
    return;
}

//TCAP indicates DialogPAbort: either due to TC layer error on SCF/HLR side or
//because of local TC dialog timeout is expired (no T_END_IND from HLR).
void MapCHSRIDlg::onDialogPAbort(UCHAR_T abortCause)
{
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
        smsc_log_error(logger, "MapSRI[%u]: state 0x%x, P_ABORT: %s ", sriId,
                        _sriState.value, _RCS_TC_Abort->code2Txt(abortCause));
        endTCap();
    }
    sriHdl->onEndMapDlg(_RCS_TC_Abort->mkhash(abortCause));
}

//SCF sent DialogUAbort (some logic error on SCF side).
void MapCHSRIDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
        smsc_log_error(logger, "MapSRI[%u]: state 0x%x, U_ABORT: %s", sriId,
                        _sriState.value, !abortInfo_len ? "userInfo" :
                        _RCS_TC_Abort->code2Txt(*pAbortInfo));
        endTCap();
    }
    sriHdl->onEndMapDlg(_RCS_TC_Abort->mkhash(!abortInfo_len ?
                        TC_AbortCause::userAbort : *pAbortInfo));
}

//Underlying layer unable to deliver message, just abort dialog
void MapCHSRIDlg::onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        UCHAR_T invId/* = 0*/, UCHAR_T opCode/* = 0*/)
{
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
        std::string dstr;
        if (comp_kind != TcapEntity::tceNone) {
            format(dstr, ", Invoke[%u]", invId);
            switch (comp_kind) {
            case TcapEntity::tceError:      dstr += ".Error"; break;
            case TcapEntity::tceResult:     dstr += ".Result"; break;
            case TcapEntity::tceResultNL:   dstr += ".ResultNL"; break;
            default:;
            }
            dstr += " not delivered.";
        }
        smsc_log_error(logger, "MapSRI[%u]: NOTICE_IND at state 0x%x%s", sriId,
                       _sriState.value, dstr.c_str());
        endTCap();
    }
    sriHdl->onEndMapDlg(_RCS_TC_Report->mkhash(reportCause));
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapCHSRIDlg::onDialogREnd(bool compPresent)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrFinished = 1;
        if (!compPresent) {
            endTCap();
            if (!_sriState.s.ctrResulted) {
                smsc_log_error(logger, "MapSRI[%u]: T_END_IND, state 0x%x", sriId, _sriState.value);
                errcode = _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse);
            }
        }
    }
    if (!compPresent)
        sriHdl->onEndMapDlg(errcode);
    //else wait for ongoing Invoke result/error
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//ends TC dialog, releases Dialog()
void MapCHSRIDlg::endTCap(bool check_ref/* = false*/)
{
    if (dialog) {
        unsigned refNum = dialog->unbindUser();
        if (check_ref && refNum)
            smsc_log_warn(logger, "MapSRI[%u]: %u references from underlying TCDlg exists", sriId, refNum);
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            try {  // do TC_PREARRANGED if still active
                dialog->endDialog((_sriState.s.ctrInited < MapCHSRIDlg::operDone) ?
                                    Dialog::endPrearranged : Dialog::endBasic);
                smsc_log_debug(logger, "MapSRI[%u]: T_END_REQ, state: 0x%x", sriId,
                                _sriState.value);
            } catch (const std::exception & exc) {
                smsc_log_error(logger, "MapSRI[%u]: T_END_REQ: %s", sriId, exc.what());
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

