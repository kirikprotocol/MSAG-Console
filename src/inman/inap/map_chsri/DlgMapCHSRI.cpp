static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
using smsc::inman::comp::chsri::CHSendRoutingInfoArg;

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {

using smsc::inman::comp::chsri::MAP_CH_SRI_OpCode;

/* ************************************************************************** *
 * class MapCHSRIDlg implementation:
 * ************************************************************************** */
MapCHSRIDlg::MapCHSRIDlg(SSNSession* pSession, CHSRIhandler * sri_handler,
                        Logger * uselog/* = NULL*/)
    : sriHdl(sri_handler), session(pSession), sriId(0), dialog(NULL)
    , logger(uselog)
{
    assert(sri_handler && pSession);
    _sriState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.Inap.MapCHSRIDlg");
}

MapCHSRIDlg::~MapCHSRIDlg()
{
    endMapDlg();
}

void MapCHSRIDlg::endMapDlg(void)
{
    MutexGuard  grd(_sync);
    sriHdl = NULL;
    endTCap();
}

/* ------------------------------------------------------------------------ *
 * MAP_SEND_ROUTING_INFO interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::reqRoutingInfo(const char * subcr_adr, USHORT_T timeout/* = 0*/) throw(CustomException)
{
    MutexGuard  grd(_sync);
    TonNpiAddress   tnAdr;
    if (!tnAdr.fromText(subcr_adr))
        throw CustomException("inalid subscriberID", -1, subcr_adr);

    dialog = session->openDialog(tnAdr.getSignals());
    if (!dialog)
        throw CustomException("MapSRI[0]: Unable to create TC Dialog");
    if (timeout)
        dialog->setInvokeTimeout(timeout);
    dialog->addListener(this);
    sriId = (unsigned)dialog->getId();

    CHSendRoutingInfoArg     arg;
    arg.setGMSCorSCFaddress(session->getOwnAdr());
    arg.setSubscrMSISDN(tnAdr);

    Invoke* op = dialog->initInvoke(MAP_CH_SRI_OpCode::sendRoutingInfo, this);
    op->setParam(&arg);
    dialog->sendInvoke(op);

    dialog->beginDialog();
    _sriState.s.ctrInited = MAP_OPER_INITED;
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapCHSRIDlg::onInvokeResultNL(Invoke* op, TcapEntity* res)
{
    MutexGuard  grd(_sync);
    smsc_log_debug(logger, "MapSRI[%u]: Invoke[%u:%u] got a returnResultNL: %u",
        sriId, (unsigned)op->getId(), (unsigned)op->getOpcode(),
        (unsigned)res->getOpcode());

    _sriState.s.ctrResulted = MAP_OPER_INITED;
    try { reqRes.mergeSegment(res->getParam());
    } catch (CustomException & exc) {
        smsc_log_error(logger, "MapSRI[%u]: %s", exc.what());
    }
}

void MapCHSRIDlg::onInvokeResult(Invoke* op, TcapEntity* res)
{
    unsigned do_end = 0;
    {
        MutexGuard  grd(_sync);
        smsc_log_debug(logger, "MapSRI[%u]: Invoke[%u:%u] got a returnResult: %u",
            sriId, (unsigned)op->getId(), (unsigned)op->getOpcode(),
            (unsigned)res->getOpcode());

        _sriState.s.ctrResulted = MAP_OPER_DONE;
        try { reqRes.mergeSegment(res->getParam());
        } catch (CustomException & exc) {
            smsc_log_error(logger, "MapSRI[%u]: %s", exc.what());
        }
        sriHdl->onMapResult(&reqRes);
        if ((do_end = _sriState.s.ctrFinished) != 0)
            endTCap();
    }
    if (do_end)
        sriHdl->onEndMapDlg(0, smsc::inman::errOk);
}

//Called if Operation got ResultError
void MapCHSRIDlg::onInvokeError(Invoke *op, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapSRI[%u]: Invoke[%u:%u] got a returnError: %u",
            sriId, (unsigned)op->getId(), (unsigned)op->getOpcode(),
            (unsigned)resE->getOpcode());

        _sriState.s.ctrInited = MAP_OPER_FAIL;
        endTCap();
    }
    sriHdl->onEndMapDlg(resE->getOpcode(), smsc::inman::errMAP);
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapCHSRIDlg::onInvokeLCancel(Invoke *op)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "MapSRI[%u]: Invoke[%u:%u] got a LCancel",
            sriId, (unsigned)op->getId(), (unsigned)op->getOpcode());
        endTCap();
    }
    sriHdl->onEndMapDlg(MapCHSRIDlg::chsriServiceResponse, smsc::inman::errMAPuser);
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
        smsc_log_error(logger, "MapSRI[%u]: P_ABORT at state 0x%x", sriId, _sriState.value);
        endTCap();
    }
    sriHdl->onEndMapDlg(abortCause, smsc::inman::errTCAP);
}

//SCF sent DialogUAbort (some logic error on SCF side).
void MapCHSRIDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrAborted = 1;
        smsc_log_error(logger, "MapSRI[%u]: U_ABORT at state 0x%x", sriId, _sriState.value);
        endTCap();
    }
    sriHdl->onEndMapDlg((abortInfo_len == 1) ? *pAbortInfo : Dialog::tcUserGeneralError,
                       smsc::inman::errTCuser);
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapCHSRIDlg::onDialogREnd(bool compPresent)
{
    InmanErrorType layer = smsc::inman::errOk;
    unsigned char  errcode = 0;
    {
        MutexGuard  grd(_sync);
        _sriState.s.ctrFinished = 1;
        if (!compPresent) {
            endTCap();
            if (!_sriState.s.ctrResulted) {
                smsc_log_error(logger, "MapSRI[%u]: T_END_IND, state 0x%x", sriId, _sriState.value);
                layer = smsc::inman::errMAPuser;
                errcode = MapCHSRIDlg::chsriServiceResponse;
            }
        }
    }
    if (!compPresent)
        sriHdl->onEndMapDlg(errcode, layer);
    //else wait for ongoing Invoke result/error
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
//ends TC dialog, releases Dialog()
void MapCHSRIDlg::endTCap(void)
{
    if (dialog) {
        dialog->removeListener(this);
        if (!(dialog->getState().value & TC_DLG_CLOSED_MASK)) {
            //see 3GPP 29.078 14.1.2.1.3 smsSSF-to-gsmSCF SMS related messages
            try {
                dialog->endDialog(); // do TC_BasicEnd if still active
                smsc_log_debug(logger, "MapSRI[%u]: T_END_REQ, state: 0x%x", sriId,
                                _sriState.value);
            } catch (std::exception & exc) {
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

