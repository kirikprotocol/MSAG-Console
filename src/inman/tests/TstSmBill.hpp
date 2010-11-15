/* ************************************************************************** *
 * INMan SM/USSD Billing protocol client
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_BILLING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TEST_BILLING_HPP

#include "inman/tests/AbonentsDB.hpp"
#include "inman/tests/TstFacade.hpp"

#include "inman/interaction/msgbill/MsgBilling.hpp"

using smsc::inman::cdr::CDRRecord;

using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::DeliveredSmsData;
using smsc::inman::interaction::SMSCBillingHandlerITF;
using smsc::inman::interaction::CsBillingHdr_dlg;

namespace smsc  {
namespace inman {
namespace test {

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
      : did(dlg_id), cfg(*use_cfg), state(dIdle), batchMode(batch_mode)
      , dlvrRes(dlvr_res)
  { }

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
  INDialogCfg         cfg;
  DlgState            state;
  bool                batchMode;
  uint32_t            dlvrRes; //failure by default 
};

/* ************************************************************************** *
 * class BillFacade: 
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

  // ---------------------------------------------------
  // -- SMSCBillingHandlerITF interface implementation
  // ---------------------------------------------------
  void onChargeSmsResult(ChargeSmsResult * result, CsBillingHdr_dlg * hdr);

public:
  explicit BillFacade(ConnectSrv * conn_srv, Logger * use_log = NULL);
  //
  virtual ~BillFacade();

  // -- INDialog template params -- //
  const INDialogCfg * getDlgConfig(void) const { return &_dlgCfg; }
  void  printDlgConfig(void) const;

  void setUssdOp(bool op) { _dlgCfg.ussdOp = op; }
  void setSmsXIds(uint32_t srv_ids) { _dlgCfg.xsmsIds = srv_ids; }
  void setChargePolicy(CDRRecord::ChargingPolicy chg_pol) { _dlgCfg.chgPolicy = chg_pol; }
  void setChargeType(CDRRecord::ChargingType chg_typ) { _dlgCfg.chgType = chg_typ; }
  void setForcedCDR(bool forced_cdr = true) { _dlgCfg.forcedCDR = forced_cdr; }
  void setLocMSC(const std::string & use_adr) { _dlgCfg.locMSC = use_adr; }
  bool setAbonentId(unsigned ab_id, bool orig_abn = true);

  // -- INDialog management methods -- //
  INDialog *  findDialog(unsigned int did) const;
  unsigned    getNextDialogId(void) { return ++_maxDlgId; }
  unsigned    initDialog(unsigned int did = 0, bool batch_mode = false,
                          uint32_t delivery = 1016, INDialogCfg * use_cfg = NULL);

  // -- INMan commands composition and sending methods -- //
  void composeChargeSms(ChargeSms & op, const INDialogCfg * dlg_cfg);
  void composeDeliveredSmsData(DeliveredSmsData & op, const INDialogCfg * dlg_cfg);
  void composeDeliverySmsResult(DeliverySmsResult & op, const INDialogCfg * dlg_cfg);

  void sendChargeSms(unsigned int dlgId, uint32_t num_bytes = 0);
  void sendDeliveredSmsData(unsigned int dlgId, uint32_t num_bytes = 0);
  void sendDeliverySmsResult(unsigned int dlgId, uint32_t deliveryStatus, uint32_t num_bytes = 0);

  // ---------------------------------------------------
  // -- ConnectListenerITF interface implementation
  // ---------------------------------------------------
  virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
  /*throw(std::exception) */;

};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_BILLING_HPP */

