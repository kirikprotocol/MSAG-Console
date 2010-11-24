#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/tests/TstSmBill.hpp"

using smsc::core::network::Socket;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::INPSerializer;
using smsc::inman::interaction::INPCSBilling;

using smsc::inman::interaction::SPckChargeSms;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::SPckDeliverySmsResult;
using smsc::inman::interaction::SPckDeliveredSmsData;

using smsc::util::format;

namespace smsc  {
namespace inman {
namespace test {

/* ************************************************************************** *
 * class BillFacade implementation:
 * ************************************************************************** */
BillFacade::BillFacade(AbonentsDB & use_db, ConnectSrv * conn_srv, Logger * use_log/* = NULL*/)
  : TSTFacadeAC(conn_srv, use_log)
  , _abDB(use_db), _msg_ref(0x0100), _msg_id(0x010203040000ULL), _maxDlgId(0)
  , _dlgCfg(use_db)
{ 
  strcpy(_logId, "TFBill");
  INPSerializer::getInstance()->registerCmdSet(INPCSBilling::getInstance());
}

BillFacade::~BillFacade()
{ 
  MutexGuard grd(_sync);
  do_disconnect();
  if (!_dlgReg.empty()) {
    for (INDialogsMap::iterator
         it = _dlgReg.begin(); it != _dlgReg.end(); ++it) {
      delete it->second;
    }
    _dlgReg.clear();
  }
}

unsigned BillFacade::getNextDialogId(void)
{
  MutexGuard grd(_sync);
  return allcNextDialogId();
}

unsigned BillFacade::initDialog(unsigned int dlg_id/* = 0*/, bool batch_mode/* = false*/,
                        uint32_t delivery/* = 1016*/, const CapSmDialogCfg * use_cfg/* = NULL*/)
{
  MutexGuard grd(_sync);
  if (!dlg_id)
    dlg_id = allcNextDialogId();
  if (!use_cfg)
    use_cfg = &_dlgCfg;

  CapSmDialog * pDlg = new CapSmDialog(*use_cfg, dlg_id, batch_mode, delivery);
  _dlgReg.insert(INDialogsMap::value_type(dlg_id, pDlg));
  return dlg_id;
}

CapSmDialog * BillFacade::findDialog(unsigned int dlg_id) const
{
  MutexGuard grd(_sync);
  INDialogsMap::const_iterator it = _dlgReg.find(dlg_id);
  return (it != _dlgReg.end()) ? it->second : NULL;
}

// -- INMan commands composition and sending methods -- //
void BillFacade::composeChargeSms(ChargeSms & op, const CapSmDialogCfg & dlg_cfg)
{
  op.setDestinationSubscriberNumber(dlg_cfg.getDstAbnt()->msIsdn.toString().c_str());
  op.setCallingPartyNumber(dlg_cfg.getOrgAbnt()->msIsdn.toString().c_str());

  if (dlg_cfg.getOrgIMSI())
    op.setCallingIMSI(dlg_cfg.getOrgIMSI());
  if (dlg_cfg.isForcedCDR())
    op.setForcedCDR();
  if (dlg_cfg.getOrgMSC())
    op.setLocationInformationMSC(dlg_cfg.getOrgMSC()->toString().c_str());

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
  op.setServiceOp(dlg_cfg.isUssdOp() ? 0 : -1);
  op.setMsgLength(160);
  op.setSmsXSrvs(dlg_cfg.getSmsXIds());

  if (dlg_cfg.isChargePolicy(CDRRecord::ON_SUBMIT))
    op.setChargeOnSubmit();
  if (dlg_cfg.isChargeType(CDRRecord::MT_Charge))
    op.setMTcharge();
}

void BillFacade::composeDeliveredSmsData(DeliveredSmsData & op, const CapSmDialogCfg & dlg_cfg)
{
  const AbonentInfo * dstAbnt = dlg_cfg.getDstAbnt();
 
  op.setDestinationSubscriberNumber(dstAbnt->msIsdn.toString().c_str());
  op.setCallingPartyNumber(dlg_cfg.getOrgAbnt()->msIsdn.toString().c_str());

  if (dlg_cfg.getOrgIMSI())
    op.setCallingIMSI(dlg_cfg.getOrgIMSI());
  if (dlg_cfg.isForcedCDR())
    op.setForcedCDR();
  if (dlg_cfg.getOrgMSC())
    op.setLocationInformationMSC(dlg_cfg.getOrgMSC()->toString().c_str());

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
  op.setServiceOp(dlg_cfg.isUssdOp() ? 0 : -1);
  op.setMsgLength(160);
  op.setSmsXSrvs(dlg_cfg.getSmsXIds());

  if (dlg_cfg.isChargePolicy(CDRRecord::ON_SUBMIT_COLLECTED))
    op.setChargeOnSubmit();
  if (dlg_cfg.isChargeType(CDRRecord::MT_Charge))
    op.setMTcharge();

  //fill delivery fields for CDR creation
  if (dlg_cfg.getDstIMSI())
    op.setDestIMSI(dlg_cfg.getDstIMSI());
  if (dlg_cfg.getDstMSC()) //".1.1.79139860001"
    op.setDestMSC(dlg_cfg.getDstMSC()->toString().c_str());

  op.setDestSMEid("DST_MAP_PROXY");
  op.setDivertedAdr(dstAbnt->msIsdn.toString().c_str());
  op.setDeliveryTime(time(NULL));
}

void BillFacade::composeDeliverySmsResult(DeliverySmsResult & op, const CapSmDialogCfg & dlg_cfg)
{
  const AbonentInfo * dstAbnt = dlg_cfg.getDstAbnt();

  //fill fields for CDR creation
  if (dlg_cfg.getDstIMSI())
    op.setDestIMSI(dlg_cfg.getDstIMSI());
  if (dlg_cfg.getDstMSC()) //".1.1.79139860001"
    op.setDestMSC(dlg_cfg.getDstMSC()->toString().c_str());
  
  op.setDestSMEid("DST_MAP_PROXY");
  op.setDivertedAdr(dstAbnt->msIsdn.toString().c_str());
  op.setDeliveryTime(time(NULL));
}

void BillFacade::sendChargeSms(unsigned int dlg_id, uint32_t num_bytes/* = 0*/)
{   
  std::string       msg;
  CapSmDialogCfg &  dlgCfg = _dlgCfg;
  CapSmDialog *     dlg = findDialog(dlg_id);

  if (!dlg) {
    format(msg, "WRN: Dialog[%u] is unknown!", dlg_id);
    Prompt(Logger::LEVEL_DEBUG, msg);
  } else {
    dlgCfg = dlg->getConfig();
    //fix charging mode
    if (dlgCfg.isChargePolicy(CDRRecord::ON_DATA_COLLECTED))
        dlgCfg.setChargePolicy(CDRRecord::ON_DELIVERY);
  }
  //compose ChargeSms
  CDRRecord       cdr;
  SPckChargeSms   pck;

  pck.Hdr().dlgId = dlg_id;
  composeChargeSms(pck.Cmd(), dlgCfg);
  pck.Cmd().export2CDR(cdr);

  char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
  int n = 0;
  if (num_bytes)
    n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
  tbuf[n] = 0;

  msg.clear();
  format(msg, "--> %sChargeSms[%u] %s: %s -> %s .., %s", tbuf, dlg_id,
               cdr.dpType().c_str(), cdr._srcAdr.c_str(), cdr._dstAdr.c_str(),
               cdr.nmPolicy());
  Prompt(Logger::LEVEL_DEBUG, msg);
  if (sendPckPart(&pck, num_bytes) && dlg) { // 0 - forces sending whole packet
    if (dlg->isState(CapSmDialog::dIdle))
      dlg->setState(CapSmDialog::dCharged);
    else {
      msg.clear();
      format(msg, "WRN: Dialog[%u] state is %u!", dlg->getState());
      Prompt(Logger::LEVEL_DEBUG, msg);
    }
  }
}

void BillFacade::sendDeliveredSmsData(unsigned int dlg_id, uint32_t num_bytes/* = 0*/)
{
  std::string       msg;
  CapSmDialogCfg &  dlgCfg = _dlgCfg;
  CapSmDialog *     dlg = findDialog(dlg_id);
  if (!dlg) {
    format(msg, "WRN: Dialog[%u] is unknown!", dlg_id);
    Prompt(Logger::LEVEL_DEBUG, msg);
  } else {
    dlgCfg = dlg->getConfig();
  }
  //compose DeliveredSmsData
  CDRRecord       cdr;
  SPckDeliveredSmsData   pck;

  pck.Hdr().dlgId = dlg_id;
  composeDeliveredSmsData(pck.Cmd(), dlgCfg);
  pck.Cmd().setResultValue(dlg->getDlvrResult());
  pck.Cmd().export2CDR(cdr);

  char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
  int n = 0;
  if (num_bytes)
    n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
  tbuf[n] = 0;

  msg.clear();
  format(msg, "--> %sDeliveredSmsData[%u] %s: %s -> %s ..., %s", tbuf, dlg_id,
               cdr.dpType().c_str(), cdr._srcAdr.c_str(), cdr._dstAdr.c_str(),
               cdr.nmPolicy());
  Prompt(Logger::LEVEL_DEBUG, msg);
  if (sendPckPart(&pck, num_bytes) && dlg) { // 0 - forces sending whole packet
    if (dlg->isState(CapSmDialog::dIdle))
      dlg->setState(CapSmDialog::dReported);
    else {
      msg.clear();
      format(msg, "WRN: Dialog[%u] state is %u!", dlg->getState());
      Prompt(Logger::LEVEL_DEBUG, msg);
    }
  }
}

void BillFacade::sendDeliverySmsResult(unsigned int dlg_id, uint32_t delivery_status,
                                       uint32_t num_bytes/* = 0*/)
{
  std::string       msg;
  CapSmDialogCfg &  dlgCfg = _dlgCfg;
  CapSmDialog *     dlg = findDialog(dlg_id);

  if (!dlg) {
    format(msg, "WRN: Dialog[%u] is unknown!", dlg_id);
    Prompt(Logger::LEVEL_DEBUG, msg);
  } else {
    dlgCfg = dlg->getConfig();
  }
  //compose DeliverySmsResult
  SPckDeliverySmsResult   pck;
  pck.Hdr().dlgId = dlg_id;
  pck.Cmd().setResultValue(delivery_status);
  composeDeliverySmsResult(pck.Cmd(), dlgCfg);

  char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
  int n = 0;
  if (num_bytes)
    n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
  tbuf[n] = 0;

  msg.clear();
  format(msg, "--> %sDeliverySmsResult[%u]: DELIVERY_%s", tbuf,
               dlg_id, !delivery_status ? "SUCCEEDED" : "FAILED");
  Prompt(Logger::LEVEL_DEBUG, msg);
  if (sendPckPart(&pck, num_bytes) && dlg) { // 0 - forces sending whole packet
    if (dlg->isState(CapSmDialog::dApproved))
      dlg->setState(CapSmDialog::dReported);
    else {
      msg.clear();
      format(msg, "WRN: Dialog[%u] state is %u!", dlg->getState());
      Prompt(Logger::LEVEL_DEBUG, msg);
    }
  }
}

// ---------------------------------------------------
// -- ConnectListenerITF interface implementation
// ---------------------------------------------------
void BillFacade::onPacketReceived(Connect * conn,
                                  std::auto_ptr<SerializablePacketAC>& recv_cmd)
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
      catch (const SerializerException & exc) {
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

// ---------------------------------------------------
// -- SMSCBillingHandlerITF interface implementation
// ---------------------------------------------------
void BillFacade::onChargeSmsResult(ChargeSmsResult * result, CsBillingHdr_dlg * hdr)
{
  std::string msg;

  format(msg, "<-- ChargeSmsResult[%u]: CHARGING_%sPOSSIBLE", hdr->dlgId,
        (result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE ) ? "" : "NOT_");
  if (result->getError())
      msg += format(", error %u: %s", result->getError(), result->getMsg());

  Prompt(Logger::LEVEL_DEBUG, msg);

  CapSmDialog * dlg = findDialog(hdr->dlgId);
  if (dlg) {
    if (dlg->isState(CapSmDialog::dCharged)) {
      dlg->setState(CapSmDialog::dApproved);
      if (dlg->isBatchMode()) {
        if ((result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE)
            && !dlg->getConfig().isChargePolicy(CDRRecord::ON_DATA_COLLECTED))
            sendDeliverySmsResult(hdr->dlgId, dlg->getDlvrResult());
      }
    } else {
      msg.clear();
      format(msg, "ERR: Dialog[%u] was not Charged!", hdr->dlgId);
      Prompt(Logger::LEVEL_ERROR, msg);
    }
  } else {
    msg.clear();
    format(msg, "ERR: Dialog[%u] is inknown!", hdr->dlgId);
    Prompt(Logger::LEVEL_ERROR, msg);
  }
}


} // test
} // namespace inman
} // namespace smsc

