#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/inap/map_atih/DlgMapATSI.hpp"
using smsc::inman::comp::atih::MAPATIH_OpCode;
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
    : atsiHdl(atsi_handler), session(pSession), atsiId(0), dialog(NULL)
    , logger(uselog)
{
    _atsiState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Inap.MapATSIDlg");
}

MapATSIDlg::~MapATSIDlg()
{
    endATSI();
}

void MapATSIDlg::endATSI(void)
{
    MutexGuard  grd(_sync);
    atsiHdl = NULL;
    endTCap();
}

/* ------------------------------------------------------------------------ *
 * ATSIcontractor interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::subsciptionInterrogation(const char * subcr_adr,
                    bool imsi/* = false*/, uint16_t timeout/* = 0*/) throw(CustomException)
{
    MutexGuard  grd(_sync);

    if (!atsiHdl || !session)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapATSI", "invalid initialization");
    
    TonNpiAddress   tnAdr;
    if (!tnAdr.fromText(subcr_adr))
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapATSI: invalid subscriber addr", subcr_adr);

    dialog = session->openDialog(tnAdr, logger);
    if (!dialog)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapATSI", "unable to create TC dialog");

    if (timeout)
        dialog->setInvokeTimeout(timeout);
    dialog->bindUser(this);
    atsiId = (unsigned)dialog->getId();

    ATSIArg     arg;
    arg.setSCFaddress(session->getOwnAdr());
    if (!imsi)
        arg.setSubscriberId(tnAdr);
    else
        arg.setSubscriberId(subcr_adr, imsi);
    arg.setRequestedCSI(RequestedCAMEL_SubscriptionInfo_o_CSI);

    dialog->sendInvoke(MAPATIH_OpCode::anyTimeSubscriptionInterrogation, &arg); //throws
    dialog->beginDialog(); //throws
    _atsiState.s.ctrInited = MapATSIDlg::operInited;
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::onInvokeResult(InvokeRFP pInv, TcapEntity* res)
{
    unsigned do_end = 0;
    {
        MutexGuard  grd(_sync);
        smsc_log_debug(logger, "MapATSI[%u]: Invoke[%u:%u] got a returnResult: %u",
            atsiId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)res->getOpcode());

        _atsiState.s.ctrResulted = 1;
        _atsiState.s.ctrInited = MapATSIDlg::operDone;
        ATSIRes * resComp = static_cast<ATSIRes *>(res->getParam());
        atsiHdl->onATSIResult(resComp);
        if ((do_end = _atsiState.s.ctrFinished) != 0)
            endTCap();
    }
    if (do_end)
        atsiHdl->onEndATSI();
}

//Called if Operation got ResultError
void MapATSIDlg::onInvokeError(InvokeRFP pInv, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapATSI[%u]: Invoke[%u:%u] got a returnError: %u",
            atsiId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)resE->getOpcode());

        _atsiState.s.ctrInited = MapATSIDlg::operDone;
        endTCap();
    }
    atsiHdl->onEndATSI(_RCS_MAPOpErrors->mkhash(resE->getOpcode()));
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapATSIDlg::onInvokeLCancel(InvokeRFP pInv)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapATSI[%u]: Invoke[%u:%u] got a LCancel",
            atsiId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode());
        _atsiState.s.ctrInited = MapATSIDlg::operFailed;
        endTCap();
    }
    atsiHdl->onEndATSI(_RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse));
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface
 * ------------------------------------------------------------------------ */
//SCF sent ContinueDialog.
void MapATSIDlg::onDialogContinue(bool compPresent)
{
    MutexGuard  grd(_sync);
    if (!compPresent)
        smsc_log_error(logger, "MapATSI[%u]: missing component in TC_CONT_IND", atsiId);
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
        smsc_log_error(logger, "MapATSI[%u]: state 0x%x, P_ABORT: '%s'", atsiId,
                        _atsiState.value, _RCS_TC_PAbort->code2Txt(abortCause));
        endTCap();
    }
    atsiHdl->onEndATSI(_RCS_TC_PAbort->mkhash(abortCause));
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
        smsc_log_error(logger, "MapSRI[%u]: state 0x%x, U_ABORT: '%s'", atsiId,
                        _atsiState.value, _RCS_TC_UAbort->code2Txt(abortCause));
        endTCap();
    }
    atsiHdl->onEndATSI(_RCS_TC_UAbort->mkhash(abortCause));
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
        smsc_log_error(logger, "MapATSI[%u]: state 0x%x, NOTICE_IND: '%s', %s", atsiId,
                       _atsiState.value, _RCS_TC_Report->code2Txt(reportCause),
                       dstr.c_str());
        endTCap();
    }
    atsiHdl->onEndATSI(_RCS_TC_Report->mkhash(reportCause));
}


//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapATSIDlg::onDialogREnd(bool compPresent)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _atsiState.s.ctrFinished = 1;
        if (!compPresent) {
            endTCap();
            if (!_atsiState.s.ctrResulted) {
                smsc_log_error(logger, "MapATSI[%u]: T_END_IND, state 0x%x", atsiId, _atsiState.value);
                errcode = _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse);
            }
        }
    }
    if (!compPresent)
        atsiHdl->onEndATSI(errcode);
    //else wait for ongoing Invoke result/error
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//ends TC dialog, releases Dialog()
void MapATSIDlg::endTCap(void)
{
    while (dialog && !dialog->unbindUser()) //TCDlg refers this object
        _sync.wait();

    if (dialog) {
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            try {    // do TC_PREARRANGED if still active
                dialog->endDialog((_atsiState.s.ctrInited < MapATSIDlg::operDone) ?
                                    Dialog::endPrearranged : Dialog::endBasic);
                smsc_log_debug(logger, "MapATSI[%u]: T_END_REQ, state: 0x%x", atsiId,
                                _atsiState.value);
            } catch (std::exception & exc) {
                smsc_log_error(logger, "MapATSI[%u]: T_END_REQ: %s", atsiId, exc.what());
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

