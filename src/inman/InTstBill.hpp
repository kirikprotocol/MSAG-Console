/* ************************************************************************** *
 * INMan testing console: Billing protocol client
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_BILLING__
#define __SMSC_INMAN_TEST_BILLING__
#ident "$Id$"

#include "inman/InTstDefs.hpp"
using smsc::inman::test::TSTFacadeAC;
using smsc::inman::test::AbonentsDB;

#include "inman/interaction/msgbill/MsgBilling.hpp"
using smsc::inman::interaction::INPSerializer;
using smsc::inman::interaction::INPCSBilling;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::DeliveredSmsData;
using smsc::inman::interaction::SMSCBillingHandlerITF;
using smsc::inman::interaction::CsBillingHdr_dlg;
using smsc::inman::interaction::SPckChargeSms;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::SPckDeliverySmsResult;
using smsc::inman::interaction::SPckDeliveredSmsData;

namespace smsc  {
namespace inman {
namespace test {

using smsc::inman::cdr::CDRRecord;

/* ************************************************************************** *
 * class INDialog: INMan billing dialog params and result
 * ************************************************************************** */
struct INDialogCfg {
    unsigned            abId;   //abonent's id from AbonentsDB
    unsigned            dstId;  //destination address id form TonNpiDB
    bool                ussdOp;
    uint32_t            xsmsIds; //SMS Extra services id
    CDRRecord::ChargingPolicy   chgPolicy;
    CDRRecord::ChargingType     chgType;
    bool                forcedCDR;
    std::string         locMSC; //abonent's location MSC address:
                                //  TonNpiAddress or string

    //NOTE: PRE_ABONENTS_NUM >= 2
    INDialogCfg() : abId(1), dstId(2), ussdOp(false)
      , xsmsIds(0), chgPolicy(CDRRecord::ON_DELIVERY), chgType(CDRRecord::MO_Charge)
      , forcedCDR(false), locMSC(".1.1.79139860001")
    { }
};

class INDialog {
public:
    enum DlgState { dIdle = 0, dCharged = 1, dApproved, dReported };

    INDialog(unsigned int dlg_id, const INDialogCfg * use_cfg, bool batch_mode = false,
             uint32_t dlvr_res = 1016)
        : did(dlg_id), cfg(*use_cfg), batchMode(batch_mode), dlvrRes(dlvr_res)
        , state(dIdle)
    {}

    void    setChargePolicy(CDRRecord::ChargingPolicy chg_pol) { cfg.chgPolicy = chg_pol; }
    void    setChargeType(CDRRecord::ChargingType chg_type) { cfg.chgType = chg_type; }
    void    setState(DlgState new_state) { state = new_state; }
    void    setLocMSC(const std::string & use_adr) { cfg.locMSC = use_adr; }
    void    setForcedCDR(bool forced_cdr = true) { cfg.forcedCDR = forced_cdr; }

    DlgState getState(void) const { return state; }
    uint32_t getDlvrResult(void) const { return dlvrRes; }
    bool    isBatchMode(void) const { return batchMode; }
    const INDialogCfg * getConfig(void) const { return &cfg; }

protected:
    unsigned int        did;
    DlgState            state;
    bool                batchMode;
    INDialogCfg         cfg;
    uint32_t            dlvrRes; //failure by default 
};

/* ************************************************************************** *
 * class BillFacade: INMan billing dialogs controlling registry
 * ************************************************************************** */
class BillFacade : public TSTFacadeAC, SMSCBillingHandlerITF {
protected:
    typedef std::map<unsigned int, INDialog*> INDialogsMap;

    uint32_t  _msg_ref;
    uint64_t  _msg_id;

    unsigned      _maxDlgId;
    INDialogsMap  _Dialogs;
    INDialogCfg   _dlgCfg;
    AbonentsDB *  _abDB;

public:
    BillFacade(ConnectSrv * conn_srv, Logger * use_log = NULL)
        : TSTFacadeAC(conn_srv, use_log), _maxDlgId(0)
        , _msg_ref(0x0100), _msg_id(0x010203040000ULL)
        , _abDB(AbonentsDB::getInstance())
    { 
        strcpy(_logId, "TFBill");
        INPSerializer::getInstance()->registerCmdSet(INPCSBilling::getInstance());
    }

    virtual ~BillFacade()
    { 
        MutexGuard grd(_sync);
        do_disconnect();
        if (!_Dialogs.empty()) {
            INDialogsMap::const_iterator it;
            for (it = _Dialogs.begin(); it != _Dialogs.end(); it++) {
                INDialog * dlg = (*it).second;
                delete dlg;
            }
            _Dialogs.clear();
        }
    }

    // -- INDialog template params -- //
    const INDialogCfg * getDlgConfig(void) const { return &_dlgCfg; }
    void  printDlgConfig(void) const
    {
        const AbonentInfo * abi = _abDB->getAbnInfo(_dlgCfg.abId);
        const AbonentInfo * dAdr = _abDB->getAbnInfo(_dlgCfg.dstId);
        fprintf(stdout, "INDialog config:\n"
                "  LocationMSC: %s\n"
                "  OrigAdr[%u]: %s (%s)\n"
                "  bearerType : dp%s\n"
                "  DestAdr[%u]: %s (%s)\n"
                "  chargePol  : %s\n"
                "  chargeType : %s\n"
                "  forcedCDR  : %s\n"
                "  SMSExtra: %u\n",
                _dlgCfg.locMSC.c_str(),
                _dlgCfg.abId, (abi->msIsdn.toString()).c_str(), abi->type2Str(),
                _dlgCfg.ussdOp ? "USSD" : "SMS",
                _dlgCfg.dstId, (dAdr->msIsdn.toString()).c_str(), dAdr->type2Str(),
                CDRRecord::nmPolicy(_dlgCfg.chgPolicy),
                _dlgCfg.chgType ? "MT" : "MO", _dlgCfg.forcedCDR ? "ON" : "OFF",
                _dlgCfg.xsmsIds);
    }

    void setUssdOp(bool op) { _dlgCfg.ussdOp = op; }
    void setSmsXIds(uint32_t srv_ids) { _dlgCfg.xsmsIds = srv_ids; }
    void setChargePolicy(CDRRecord::ChargingPolicy chg_pol) { _dlgCfg.chgPolicy = chg_pol; }
    void setChargeType(CDRRecord::ChargingType chg_typ) { _dlgCfg.chgType = chg_typ; }
    void setForcedCDR(bool forced_cdr = true) { _dlgCfg.forcedCDR = forced_cdr; }
    void setLocMSC(const std::string & use_adr) { _dlgCfg.locMSC = use_adr; }

    bool setAbonentId(unsigned ab_id, bool orig_abn = true)
    {
        if (!_abDB->getAbnInfo(ab_id))
            return false;
        if (orig_abn)
            _dlgCfg.abId = ab_id;
        else
            _dlgCfg.dstId = ab_id;
        return true;
    }
    
    // -- INDialog management methods -- //
    unsigned getNextDialogId(void) { return ++_maxDlgId; }

    unsigned int initDialog(unsigned int did = 0, bool batch_mode = false,
                            uint32_t delivery = 1016, INDialogCfg * use_cfg = NULL)
    {
        MutexGuard grd(_sync);
        if (!did)
            did = getNextDialogId();
        if (!use_cfg)
            use_cfg = &_dlgCfg;
        if (!_abDB->getAbnInfo(use_cfg->abId)) {
            smsc_log_error(logger, "WRN: unknown abId: %u, using #1", use_cfg->abId);
            fprintf(stdout, "\nWRN: unknown abId: %u, using #1", use_cfg->abId);
            use_cfg->abId = 1;
        }
        INDialog * dlg = new INDialog(did, use_cfg, batch_mode, delivery);
        _Dialogs.insert(INDialogsMap::value_type(did, dlg));
        return did;
    }

    INDialog * findDialog(unsigned int did)
    {
        MutexGuard grd(_sync);
        INDialogsMap::const_iterator it = _Dialogs.find(did);
        return (it != _Dialogs.end()) ? (*it).second : NULL;
    }

    // -- INMan commands composition and sending methods -- //
    void composeChargeSms(ChargeSms& op, const INDialogCfg * dlg_cfg)
    {
        const AbonentInfo * dAdr = _abDB->getAbnInfo(dlg_cfg->dstId);
        op.setDestinationSubscriberNumber(dAdr->msIsdn.toString().c_str());

        const AbonentInfo * abi = _abDB->getAbnInfo(dlg_cfg->abId);
        op.setCallingPartyNumber(abi->msIsdn.toString().c_str());
        op.setCallingIMSI(abi->abImsi.c_str());
        if (dlg_cfg->forcedCDR)
          op.setForcedCDR();
        op.setLocationInformationMSC(dlg_cfg->locMSC.c_str());
//        op.setLocationInformationMSC(abi->msIsdn.interISDN() ?
//                                     ".1.1.79139860001" : "");
//        op.setLocationInformationMSC("GT_SMSXC");

        op.setSMSCAddress(".1.1.79029869990");

        op.setSubmitTimeTZ(time(NULL));
        op.setTPShortMessageSpecificInfo(0x11);
        op.setTPValidityPeriod(60*5);
        op.setTPProtocolIdentifier(0x00);
        op.setTPDataCodingScheme(0x08);
        //data for CDR
        op.setCallingSMEid("MAP_PROXY");
        op.setRouteId("sibinco.sms > plmn.kem");
        op.setServiceId(1234);
        op.setServiceType("InTst");
        op.setUserMsgRef(++_msg_ref);
        op.setMsgId(++_msg_id);
        op.setServiceOp(dlg_cfg->ussdOp ? 0 : -1);
        op.setMsgLength(160);
        if (dlg_cfg->xsmsIds)
            op.setSmsXSrvs(dlg_cfg->xsmsIds);
        if (dlg_cfg->chgPolicy == CDRRecord::ON_SUBMIT)
            op.setChargeOnSubmit();
        if (dlg_cfg->chgType)
            op.setMTcharge();
    }

    void composeDeliveredSmsData(DeliveredSmsData& op, const INDialogCfg * dlg_cfg)
    {
        const AbonentInfo * dAdr = _abDB->getAbnInfo(dlg_cfg->dstId);
        op.setDestinationSubscriberNumber(dAdr->msIsdn.toString().c_str());

        const AbonentInfo * abi = _abDB->getAbnInfo(dlg_cfg->abId);
        op.setCallingPartyNumber(abi->msIsdn.toString().c_str());
        op.setCallingIMSI(abi->abImsi.c_str());
        if (dlg_cfg->forcedCDR)
          op.setForcedCDR();
        op.setLocationInformationMSC(dlg_cfg->locMSC.c_str());
//        op.setLocationInformationMSC(abi->msIsdn.interISDN() ?
//                                     ".1.1.79139860001" : "");
//        op.setLocationInformationMSC("GT_SMSXC"); 
        op.setSMSCAddress(".1.1.79029869990");

        op.setSubmitTimeTZ(time(NULL));
        op.setTPShortMessageSpecificInfo(0x11);
        op.setTPValidityPeriod(60*5);
        op.setTPProtocolIdentifier(0x00);
        op.setTPDataCodingScheme(0x08);
        //data for CDR
        op.setCallingSMEid("MAP_PROXY");
        op.setRouteId("sibinco.sms > plmn.kem");
        op.setServiceId(1234);
        op.setUserMsgRef(++_msg_ref);
        op.setMsgId(++_msg_id);
        op.setServiceOp(dlg_cfg->ussdOp ? 0 : -1);
        op.setMsgLength(160);
        if (dlg_cfg->xsmsIds)
            op.setSmsXSrvs(dlg_cfg->xsmsIds);
        if (dlg_cfg->chgPolicy == CDRRecord::ON_SUBMIT_COLLECTED)
            op.setChargeOnSubmit();

        //fill delivery fields for CDR creation
        if (dAdr->getImsi())
            op.setDestIMSI(dAdr->getImsi());
        op.setDestMSC(dAdr->msIsdn.interISDN() ? ".1.1.79139860001" : "");
        op.setDestSMEid("DST_MAP_PROXY");
        op.setDivertedAdr(dAdr->msIsdn.toString().c_str());
        op.setDeliveryTime(time(NULL));
        if (dlg_cfg->chgType)
            op.setMTcharge();
    }

    void composeDeliverySmsResult(DeliverySmsResult& op, const INDialogCfg * dlg_cfg)
    {
        const AbonentInfo * dAdr = _abDB->getAbnInfo(dlg_cfg->dstId);
        //fill fields for CDR creation
        if (dAdr->getImsi())
            op.setDestIMSI(dAdr->getImsi());
        op.setDestMSC(dAdr->msIsdn.interISDN() ? ".1.1.79139860001" : "");
        op.setDestSMEid("DST_MAP_PROXY");
        op.setDivertedAdr(dAdr->msIsdn.toString().c_str());
        op.setDeliveryTime(time(NULL));
    }

    void sendChargeSms(unsigned int dlgId, uint32_t num_bytes = 0)
    {   
        std::string msg;
        const INDialogCfg * dlg_cfg = &_dlgCfg;
        INDialog * dlg = findDialog(dlgId);
        if (!dlg) {
            msg = format("WRN: Dialog[%u] is unknown!", dlgId);
            Prompt(Logger::LEVEL_DEBUG, msg);
        } else {
            dlg_cfg = dlg->getConfig();
            //fix charging mode
            if (dlg->getConfig()->chgPolicy == CDRRecord::ON_DATA_COLLECTED)
                dlg->setChargePolicy(CDRRecord::ON_DELIVERY);
        }
        //compose ChargeSms
        CDRRecord       cdr;
        SPckChargeSms   pck;
        pck.Hdr().dlgId = dlgId;
        composeChargeSms(pck.Cmd(), dlg_cfg);
        pck.Cmd().export2CDR(cdr);

        char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
        int n = 0;
        if (num_bytes)
            n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
        tbuf[n] = 0;

        msg = format("--> %sChargeSms[%u] %s: %s -> %s .., %s", tbuf, dlgId,
                     cdr.dpType().c_str(), cdr._srcAdr.c_str(), cdr._dstAdr.c_str(),
                    cdr.nmPolicy());
        Prompt(Logger::LEVEL_DEBUG, msg);
        if (sendPckPart(&pck, num_bytes) && dlg) { // 0 - forces sending whole packet
            if (dlg->getState() == INDialog::dIdle)
                dlg->setState(INDialog::dCharged);
            else {
                msg =  format("WRN: Dialog[%u] state is %u!", dlg->getState());
                Prompt(Logger::LEVEL_DEBUG, msg);
            }
        }
    }

    void sendDeliveredSmsData(unsigned int dlgId, uint32_t num_bytes = 0)
    {
        std::string msg;
        const INDialogCfg * dlg_cfg = &_dlgCfg;
        INDialog * dlg = findDialog(dlgId);
        if (!dlg) {
            msg = format("WRN: Dialog[%u] is unknown!", dlgId);
            Prompt(Logger::LEVEL_DEBUG, msg);
        } else {
            dlg_cfg = dlg->getConfig();
        }
        //compose ChargeSms
        CDRRecord       cdr;
        SPckDeliveredSmsData   pck;
        pck.Hdr().dlgId = dlgId;
        composeDeliveredSmsData(pck.Cmd(), dlg_cfg);
        pck.Cmd().setResultValue(dlg->getDlvrResult());
        pck.Cmd().export2CDR(cdr);

        char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
        int n = 0;
        if (num_bytes)
            n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
        tbuf[n] = 0;

        msg = format("--> %sDeliveredSmsData[%u] %s: %s -> %s ..., %s", tbuf, dlgId,
                     cdr.dpType().c_str(), cdr._srcAdr.c_str(), cdr._dstAdr.c_str(),
                     cdr.nmPolicy());
        Prompt(Logger::LEVEL_DEBUG, msg);
        if (sendPckPart(&pck, num_bytes) && dlg) { // 0 - forces sending whole packet
            if (dlg->getState() == INDialog::dIdle)
                dlg->setState(INDialog::dReported);
            else {
                msg =  format("WRN: Dialog[%u] state is %u!", dlg->getState());
                Prompt(Logger::LEVEL_DEBUG, msg);
            }
        }

    }

    void sendDeliverySmsResult(unsigned int dlgId, uint32_t deliveryStatus, uint32_t num_bytes = 0)
    {
        std::string msg;
        const INDialogCfg * dlg_cfg = &_dlgCfg;
        INDialog * dlg = findDialog(dlgId);
        if (!dlg) {
            msg =  format("WRN: Dialog[%u] is unknown!", dlgId);
            Prompt(Logger::LEVEL_DEBUG, msg);
        } else {
            dlg_cfg = dlg->getConfig();
        }
        //compose DeliverySmsResult
        SPckDeliverySmsResult   pck;
        pck.Hdr().dlgId = dlgId;
        pck.Cmd().setResultValue(deliveryStatus);
        composeDeliverySmsResult(pck.Cmd(), dlg_cfg);

        char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
        int n = 0;
        if (num_bytes)
            n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
        tbuf[n] = 0;

        msg = format("--> %sDeliverySmsResult[%u]: DELIVERY_%s", tbuf,
                     dlgId, !deliveryStatus ? "SUCCEEDED" : "FAILED");
        Prompt(Logger::LEVEL_DEBUG, msg);
        if (sendPckPart(&pck, num_bytes) && dlg) { // 0 - forces sending whole packet
            if (dlg->getState() == INDialog::dApproved)
                dlg->setState(INDialog::dReported);
            else {
                msg =  format("WRN: Dialog[%u] state is %u!", dlg->getState());
                Prompt(Logger::LEVEL_DEBUG, msg);
            }
        }
    }

    // -- ConnectListenerITF implementation
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
    /*throw(std::exception) */
    {
        //check service header
        INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
        //check for header
        if (!pck->pHdr() || ((pck->pHdr())->Id() != INPCSBilling::HDR_DIALOG)) {
            std::string msg;
            format(msg, "ERR: unknown cmd header: %u", (pck->pHdr())->Id());
            Prompt(Logger::LEVEL_ERROR, msg);
        } else {
            if ((pck->pCmd())->Id() == INPCSBilling::CHARGE_SMS_RESULT_TAG) {
                ChargeSmsResult * cmd = static_cast<ChargeSmsResult*>(pck->pCmd());
                CsBillingHdr_dlg * hdr = static_cast<CsBillingHdr_dlg*>(pck->pHdr());

                bool goon = true;
                try { cmd->loadDataBuf(); }
                catch (SerializerException& exc) {
                    std::string msg;
                    format(msg, "ERR: corrupted cmd %u (dlgId: %u): %s",
                            cmd->Id(), hdr->Id(), exc.what());
                    Prompt(Logger::LEVEL_ERROR, msg);
                    goon = false;
                }
                if (goon)
                    onChargeSmsResult(cmd, hdr);
            } else {
                std::string msg;
                format(msg, "ERR: unknown command recieved: %u", (pck->pCmd())->Id());
                Prompt(Logger::LEVEL_ERROR, msg);
            }
        }
    }

protected:
    // -- SMSCBillingHandlerITF methods
    void onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr)
    {
        std::string msg = format("<-- ChargeSmsResult[%u]: CHARGING_%sPOSSIBLE", hdr->dlgId,
                (result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE ) ?
                 "" : "NOT_");
        if (result->getError())
            msg += format(", error %u: %s", result->getError(), result->getMsg());

        Prompt(Logger::LEVEL_DEBUG, msg);

        INDialog * dlg = findDialog(hdr->dlgId);
        if (dlg) {
            if (dlg->getState() == INDialog::dCharged) {
                dlg->setState(INDialog::dApproved);
                if (dlg->isBatchMode()) {
                    if ((result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE)
                        && (dlg->getConfig()->chgPolicy != CDRRecord::ON_DATA_COLLECTED))
                        sendDeliverySmsResult(hdr->dlgId, dlg->getDlvrResult());
                }
            } else {
                msg = format("ERR: Dialog[%u] was not Charged!", hdr->dlgId);
                Prompt(Logger::LEVEL_ERROR, msg);
            }
        } else {
            msg = format("ERR: Dialog[%u] is inknown!", hdr->dlgId);
            Prompt(Logger::LEVEL_ERROR, msg);
        }
    }

};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_BILLING__ */

