#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************* *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST service:
 * dialog implementation (over TCAP dialog)
 * ************************************************************************* */

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "inman/common/adrutil.hpp"
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::packMAPAddress2OCTS;

#include "inman/inap/map_uss/DlgMapUSS.hpp"
using smsc::inman::comp::uss::MAPUSS_OpCode;
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
namespace uss {

/* makeUI(): composes UserInfo for TC dialog portion, containing MAP-OpenInfo PDU
 * according to following notation:

UserInfo ::= [UNIVERSAL 8] IMPLICIT SEQUENCE {
     direct-reference	OBJECT IDENTIFIER (map-DialogueAS),
     single-ASN1-type	[0] CHOICE {
	map-accept	[0] MAP-OpenInfo
     }
}

map-DialogueAS  OBJECT IDENTIFIER ::=
	{gsm-NetworkId as-Id(1) map-DialoguePDU(1) version1(1)}

gsm-NetworkId OBJECT IDENTIFIER ::=
	{ itu-t(0) identified-organization(4) etsi(0) mobileDomain(0)
	gsm-Network(1) }

MAP-OpenInfo ::= SEQUENCE {
	destinationReference	[0] AddressString	OPTIONAL,
        -- IMSI in case of id_ac_map_networkUnstructuredSs_v2 ops (cl. 7.3.1)
	originationReference	[1] AddressString	OPTIONAL,
        -- ISDN Address in case of id_ac_map_networkUnstructuredSs_v2 ops (cl. 7.3.1)
	...,
	extensionContainer	ExtensionContainer	OPTIONAL
	-- extensionContainer must not be used in version 2
	}

 * GVR NOTE: The Ericcson MAP implementation requires the ISDN address of 
 * destinationReference to be passed as vendor specific extension:
 *
 *   destinationISDN	[2] AddressString
 *  
 * References:
 * 1) Q.773: Transaction capabilities formats and encoding,
 *    clause 4.2.3 Dialogue Portion
 *
 * 2) X690: Specification of Basic Encoding Rules (BER)
 *    clause 8.18 Encoding of a value of the external type
 *
 * 3) 3GPP TS 29.02 (ETSI TS 129 002): Mobile Application Part (MAP) 
 *    clauses 7.3.1, 17.*
 */
static void makeUI(std::vector<unsigned char> & ui, const TonNpiAddress & own_adr,
                   const TonNpiAddress * msadr, const TonNpiAddress * ms_imsi)
{
    //BER encoded map-DialogueAS OID + single-ASN1-type tag [0]
    static const UCHAR_T ui_oid[] = {06,0x07,0x04,0x00,0x00,0x01,0x01,0x01,0x01,0xA0};

    unsigned addrLen = 0;
    unsigned char addrOcts[sizeof(TONNPI_ADDRESS_OCTS) + 1];
    ui.clear();

    if (msadr) {
        addrLen = packMAPAddress2OCTS(*msadr, addrOcts);
        ui.insert(ui.begin(), addrOcts, addrOcts + addrLen);
        ui.insert(ui.begin(), (unsigned char)addrLen);
        ui.insert(ui.begin(), 0x82); //destinationISDN [2]
    }

    addrLen = packMAPAddress2OCTS(own_adr, addrOcts);
    ui.insert(ui.begin(), addrOcts, addrOcts + addrLen);
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0x81); //originationReference	[1]

    if (ms_imsi) {
        addrLen = packMAPAddress2OCTS(*ms_imsi, addrOcts);
        ui.insert(ui.begin(), addrOcts, addrOcts + addrLen);
        ui.insert(ui.begin(), (unsigned char)addrLen);
        ui.insert(ui.begin(), 0x80); //destinationReference [0]
    }

    //NOTE: total encoding size is less then 127, so encode it as small number
    addrLen = (unsigned)ui.size();
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0xA0); //[0] MAP-OpenInfo

    addrLen = (unsigned)ui.size();
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), ui_oid, ui_oid + sizeof(ui_oid)); //map-DialogueAS OID

    addrLen = (unsigned)ui.size();
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0x28); //UserInfo ::= IMPLICIT [UNIVERSAL 8]
}

/* ************************************************************************** *
 * class MapUSSDlg implementation:
 * ************************************************************************** */
MapUSSDlg::MapUSSDlg(TCSessionSR* pSession, USSDhandlerITF * res_handler,
                        Logger * uselog/* = NULL*/)
    : _logPfx("MapUSS"), dialog(NULL), session(pSession), resHdl(res_handler)
    , logger(uselog)
{
    dlgState.value = 0;
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap");

    if (!resHdl || !session)
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                    "MapUSS", "invalid initialization");
    if (!(dialog = session->openDialog(logger)))
        throw CustomException((int)_RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit),
                            "MapUSS", "unable to create TC dialog");
    dlgId = dialog->getId();
    snprintf(_logId, sizeof(_logId)-1, "%s[%u:%Xh]", _logPfx,
             (unsigned)dlgId.tcInstId, (unsigned)dlgId.dlgId);

    dialog->bindUser(this);
}

MapUSSDlg::~MapUSSDlg()
{
    endMapDlg();
}

void MapUSSDlg::endMapDlg(void)
{
    MutexGuard  grd(_sync);
    resHdl = NULL;
    endTCap();
}

/* ------------------------------------------------------------------------ *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST interface
 * ------------------------------------------------------------------------ */
//composes SS request data from plain text(ASCIIZ BY default).
void MapUSSDlg::requestSS(const char * txt_data,
                            const TonNpiAddress * subsc_adr/* = NULL*/,
                            const char * subscr_imsi/* = NULL*/) throw (CustomException)
{
    //create Component for TCAP invoke
    ProcessUSSRequestArg    arg(logger);
    arg.setUSSData((const unsigned char*)txt_data);
    smsc_log_debug(logger, "%s: USS request: '%s'", _logId, txt_data);
    initSSDialog(arg, subsc_adr, subscr_imsi);
}

//composes SS request data from preencoded binary data which encoding is identified by dcs.
void MapUSSDlg::requestSS(const std::vector<unsigned char> & rq_data, unsigned char dcs,
                    const TonNpiAddress * subsc_adr/* = NULL*/, const char * subscr_imsi/* = NULL*/) throw (CustomException)
{
    //create Component for TCAP invoke
    ProcessUSSRequestArg    arg(logger);
    arg.setRAWUSSData(dcs, &rq_data[0], (unsigned)rq_data.size());
    smsc_log_debug(logger, "%s: USS request: 0x%s", _logId,
                    DumpHex(rq_data.size(), &rq_data[0]).c_str());

    initSSDialog(arg, subsc_adr, subscr_imsi);
}

/* ------------------------------------------------------------------------ *
 * InvokeListener interface
 * ------------------------------------------------------------------------ */
void MapUSSDlg::onInvokeResultNL(InvokeRFP pInv, TcapEntity* res)
{
    MutexGuard  grd(_sync);
    smsc_log_warn(logger, "%s: Invoke[%u:%u] got a ResultNL: %u",
        _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
        (unsigned)res->getOpcode());

    dlgState.s.ctrResulted = MapUSSDlg::operInited;
    reqRes.reset((MAPUSS2CompAC*)(res->getParam()));
    res->setParam(NULL);
}

void MapUSSDlg::onInvokeResult(InvokeRFP pInv, TcapEntity* res)
{
    unsigned do_end = 0;
    {
        MutexGuard  grd(_sync);
        smsc_log_debug(logger, "%s: Invoke[%u:%u] got a Result: %u",
            _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)res->getOpcode());

        dlgState.s.ctrInited = dlgState.s.ctrResulted = MapUSSDlg::operDone;
        reqRes.reset((MAPUSS2CompAC*)(res->getParam()));
        res->setParam(NULL);
        resHdl->onMapResult(reqRes.get());
        if ((do_end = dlgState.s.ctrFinished) != 0)
            endTCap();
    }
    if (do_end)
        resHdl->onEndMapDlg();
}

//Called if Operation got ResultError
void MapUSSDlg::onInvokeError(InvokeRFP pInv, TcapEntity * resE)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "%s: Invoke[%u:%u] got a Error: %u",
            _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode(),
            (unsigned)resE->getOpcode());

        dlgState.s.ctrInited = MapUSSDlg::operDone;
        endTCap();
    }
    resHdl->onEndMapDlg(_RCS_MAPOpErrors->mkhash(resE->getOpcode()));
}

//Called if Operation got L_CANCEL, possibly while waiting result
void MapUSSDlg::onInvokeLCancel(InvokeRFP pInv)
{
    {
        MutexGuard  grd(_sync);
        smsc_log_error(logger, "%s: Invoke[%u:%u] got a LCancel",
            _logId, (unsigned)pInv->getId(), (unsigned)pInv->getOpcode());
        dlgState.s.ctrInited = MapUSSDlg::operFailed;
        endTCap();
    }
    resHdl->onEndMapDlg(_RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse));
}

/* ------------------------------------------------------------------------ *
 * DialogListener interface: remote point is SCF/HLR
 * ------------------------------------------------------------------------ */
//remote point sent ContinueDialog.
void MapUSSDlg::onDialogContinue(bool compPresent)
{
    MutexGuard  grd(_sync);
    if (!compPresent) {
        smsc_log_error(logger, "%s: missing component in TC_CONT_IND", _logId);
    }
     //else wait for ongoing Invoke result/error
    return;
}

//TC indicates DialogPAbort: either because of remote TC doesn't provide
//requested service or because of local TC dialog timeout is expired
//(no T_END_IND from remote point).
void MapUSSDlg::onDialogPAbort(UCHAR_T abortCause)
{
    {
        MutexGuard  grd(_sync);
        dlgState.s.ctrAborted = 1;
        smsc_log_error(logger, "%s: state 0x%x, P_ABORT: '%s'", _logId,
                        dlgState.value, _RCS_TC_PAbort->code2Txt(abortCause));
        endTCap();
    }
    resHdl->onEndMapDlg(_RCS_TC_PAbort->mkhash(abortCause));
}

//SCF/HLR sent DialogUAbort (some logic error on remote point).
void MapUSSDlg::onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo)
{
    uint32_t abortCause = (abortInfo_len == 1) ?
        *pAbortInfo : TC_UAbortCause::userDefinedAS;
    {
        MutexGuard  grd(_sync);
        dlgState.s.ctrAborted = 1;
        smsc_log_error(logger, "%s: state 0x%x, U_ABORT: '%s'", _logId,
                        dlgState.value, _RCS_TC_UAbort->code2Txt(abortCause));
        endTCap();
    }
    resHdl->onEndMapDlg(_RCS_TC_UAbort->mkhash(abortCause));
}

//Underlying layer unable to deliver message, just abort dialog
void MapUSSDlg::onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind/* = TcapEntity::tceNone*/,
                        UCHAR_T invId/* = 0*/, UCHAR_T opCode/* = 0*/)
{
    {
        MutexGuard  grd(_sync);
        dlgState.s.ctrAborted = 1;
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
                       dlgState.value, _RCS_TC_Report->code2Txt(reportCause),
                       dstr.c_str());
        endTCap();
    }
    resHdl->onEndMapDlg(_RCS_TC_Report->mkhash(reportCause));
}

//SCF sent DialogEnd, it's either succsesfull contract completion,
//or some logic error (f.ex. timeout expiration) on SSF side.
void MapUSSDlg::onDialogREnd(bool compPresent)
{
    RCHash  errcode = 0;
    {
        MutexGuard  grd(_sync);
        dlgState.s.ctrFinished = 1;
        if (!compPresent) {
            endTCap();
            if (!dlgState.s.ctrResulted) {
                smsc_log_error(logger, "%s: T_END_IND, state 0x%x", _logId, dlgState.value);
                errcode = _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse);
            }
        }
    }
    if (!compPresent)
        resHdl->onEndMapDlg(errcode);
    //else wait for ongoing Invoke result/error
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
void MapUSSDlg::initSSDialog(ProcessUSSRequestArg & arg, 
                             const TonNpiAddress * subsc_adr/* = NULL*/,
                             const char * subscr_imsi/* = NULL*/) throw (CustomException)
{
    //create TCAP Dialog
    MutexGuard  grd(_sync);
    if (subsc_adr)
        arg.setMSISDNadr(*subsc_adr);
    dialog->sendInvoke(MAPUSS_OpCode::processUSS_Request, &arg);
    //GVR NOTE: though MAP specifies that msISDN address in USS request may
    //present in component portion of TCAP invoke, the Ericsson tools transfers
    //and expects it only in user info section (see makeUI() comments).
    if (subsc_adr || (subscr_imsi && *subscr_imsi)) {    //prepare user info
        TonNpiAddress msImsi;
        if (subscr_imsi && *subscr_imsi) { //verify IMSI
            if (!msImsi.fromText(subscr_imsi))
                throw CustomException("MapUSSDlg: invalid IMSI: '%s'", subscr_imsi);
            //set TonNPI to International 'Land Mobile' (E.212) for transmission in UserInfo
            msImsi.numPlanInd = 0x06;
            msImsi.typeOfNumber = 0x01;
        }
        std::vector<unsigned char>   ui4;
        makeUI(ui4, session->getOwnAdr(), subsc_adr, msImsi.empty() ? NULL : &msImsi);
        dialog->beginDialog(&ui4[0], (USHORT_T)ui4.size()); //throws
    } else
        dialog->beginDialog(); //throws
    dlgState.s.ctrInited = MapUSSDlg::operInited;
}

//ends TC dialog, releases Dialog()
void MapUSSDlg::endTCap(void)
{
    while (dialog && !dialog->unbindUser()) //TCDlg refers this object
        _sync.wait();

    if (dialog) {
        if (!dialog->checkState(TCDlgState::dlgClosed)) {
            try {   // do TC_PREARRANGED if still active
                dialog->endDialog((dlgState.s.ctrInited < MapUSSDlg::operDone) ?
                                    Dialog::endPrearranged : Dialog::endBasic);
                smsc_log_debug(logger, "%s: T_END_REQ, state: 0x%x", _logId,
                                dlgState.value);
            } catch (std::exception & exc) {
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


