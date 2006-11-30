static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/inap/map_atih/DlgMapATSI.hpp"

namespace smsc {
namespace inman {
namespace inap {
namespace atih {

using smsc::inman::comp::atih::MAPATIH_OpCode;

/* ************************************************************************** *
 * class MapATSIDlg implementation:
 * ************************************************************************** */
MapATSIDlg::MapATSIDlg(TCSessionMA* pSession, ATSIhandler * atsi_handler,
                        Logger * uselog/* = NULL*/)
    : atsiHdl(atsi_handler), session(pSession), atsiId(0), dialog(NULL)
    , logger(uselog)
{
    assert(atsi_handler && pSession);
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
                    bool imsi/* = false*/, USHORT_T timeout/* = 0*/) throw(CustomException)
{
    MutexGuard  grd(_sync);
    TonNpiAddress   tnAdr;
    if (!tnAdr.fromText(subcr_adr))
        throw CustomException(-1, "inalid subscriberID", subcr_adr);

    dialog = session->openDialog(tnAdr);
    if (!dialog)
        throw CustomException("MapATSI[0]: Unable to create TC Dialog");
    if (timeout)
        dialog->setInvokeTimeout(timeout);
    dialog->addListener(this);
    atsiId = (unsigned)dialog->getId();

    ATSIArg     arg;
    arg.setSCFaddress(session->getOwnAdr());
    if (!imsi)
        arg.setSubscriberId(tnAdr);
    else
        arg.setSubscriberId(subcr_adr, imsi);
    arg.setRequestedCSI(RequestedCAMEL_SubscriptionInfo_o_CSI);

    Invoke* op = dialog->initInvoke(MAPATIH_OpCode::anyTimeSubscriptionInterrogation, this);
    op->setParam(&arg);
    dialog->sendInvoke(op);

    dialog->beginDialog();
    _atsiState.s.ctrInited = MAP_OPER_INITED;
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapATSIDlg::onInvokeResult(Invoke* op, TcapEntity* res)
{
    unsigned do_end = 0;
    {
        MutexGuard  grd(_sync);
        smsc_log_debug(logger, "MapATSI[%u]: Invoke[%u:%u] got a returnResult: %u",
            atsiId, (unsigned)op->getId(), (unsigned)op->getOpcode(),
            (unsigned)res->getOpcode());

        _atsiState.s.ctrResulted = 1;
        ATSIRes * resComp = static_cast<ATSIRes *>(res->getParam());
        atsiHdl->onATSIResult(resComp);
        if ((do_end = _atsiState.s.ctrFinished) != 0)
            endTCap();
    }
    if (do_end)
        atsiHdl->onEndATSI(0, smsc::inman::errOk);
}

//Called if Operation got ResultError
void MapATSIDlg::onInvokeError(Invoke *op, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapATSI[%u]: Invoke[%u:%u] got a returnError: %u",
            atsiId, (unsigned)op->getId(), (unsigned)op->getOpcode(),
            (unsigned)resE->getOpcode());

        _atsiState.s.ctrInited = MAP_OPER_FAIL;
        endTCap();
    }
    atsiHdl->onEndATSI(resE->getOpcode(), smsc::inman::errMAP);
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapATSIDlg::onInvokeLCancel(Invoke *op)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapATSI[%u]: Invoke[%u:%u] got a LCancel",
            atsiId, (unsigned)op->getId(), (unsigned)op->getOpcode());
        endTCap();
    }
    atsiHdl->onEndATSI(MapATSIDlg::atsiServiceResponse, smsc::inman::errMAPuser);
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
void MapATSIDlg::onDialogPAbort(UCHAR_T abortCause)
{
    {
        MutexGuard  grd(_sync);
        _atsiState.s.ctrAborted = 1;
        smsc_log_error(logger, "MapATSI[%u]: P_ABORT at state 0x%x", atsiId, _atsiState.value);
        endTCap();
    }
    atsiHdl->onEndATSI(abortCause, smsc::inman::errTCAP);
}

//SCF sent DialogUAbort (some logic error on SCF side).
void MapATSIDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    {
        MutexGuard  grd(_sync);
        _atsiState.s.ctrAborted = 1;
        smsc_log_error(logger, "MapATSI[%u]: U_ABORT at state 0x%x", atsiId, _atsiState.value);
        endTCap();
    }
    atsiHdl->onEndATSI((abortInfo_len == 1) ? *pAbortInfo : Dialog::tcUserGeneralError,
                       smsc::inman::errTCuser);
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapATSIDlg::onDialogREnd(bool compPresent)
{
    InmanErrorType layer = smsc::inman::errOk;
    unsigned char  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _atsiState.s.ctrFinished = 1;
        if (!compPresent) {
            endTCap();
            if (!_atsiState.s.ctrResulted) {
                smsc_log_error(logger, "MapATSI[%u]: T_END_IND, state 0x%x", atsiId, _atsiState.value);
                layer = smsc::inman::errMAPuser;
                errcode = MapATSIDlg::atsiServiceResponse;
            }
        }
    }
    if (!compPresent)
        atsiHdl->onEndATSI(errcode, layer);
    //else wait for ongoing Invoke result/error
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//ends TC dialog, releases Dialog()
void MapATSIDlg::endTCap(void)
{
    if (dialog) {
        dialog->removeListener(this);
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            //see 3GPP 29.078 14.1.2.1.3 smsSSF-to-gsmSCF SMS related messages
            try {
                dialog->endDialog(); // do TC_BasicEnd if still active
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

