/* ************************************************************************** *
 * INMan SM/USSD Billing protocol client
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_BILLING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TEST_BILLING_HPP

#include "inman/tests/TstSmDialog.hpp"
#include "inman/tests/TstFacade.hpp"

#include "inman/interaction/msgbill/MsgBilling.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::inman::interaction::INPBilling;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::DeliveredSmsData;
using smsc::inman::interaction::INPBillingHdr_dlg;

/* ************************************************************************** *
 * class BillFacade: 
 * ************************************************************************** */
class BillFacade : public TSTFacadeAC, smsc::inman::interaction::SMSCBillingHandlerITF {
protected:
  typedef std::map<unsigned int, CapSmDialog*> INDialogsMap;

  AbonentsDB &    _abDB;
  uint32_t        _msg_ref;
  uint64_t        _msg_id;
  unsigned        _maxDlgId;
  INDialogsMap    _dlgReg;
  CapSmDialogCfg  _dlgCfg;  //default INDialog config

  unsigned    allcNextDialogId(void) { return ++_maxDlgId; }

  // -- INMan commands composition and sending methods -- //
  void composeChargeSms(ChargeSms & op, const CapSmDialogCfg & dlg_cfg);
  void composeDeliveredSmsData(DeliveredSmsData & op, const CapSmDialogCfg & dlg_cfg);
  void composeDeliverySmsResult(DeliverySmsResult & op, const CapSmDialogCfg & dlg_cfg);

  // ---------------------------------------------------
  // -- SMSCBillingHandlerITF interface implementation
  // ---------------------------------------------------
  void onChargeSmsResult(ChargeSmsResult * result, INPBillingHdr_dlg * hdr);

public:
  static const INPBilling  _protoDef; //provided protocol definition

  BillFacade(AbonentsDB & use_db, TcpServerIface & conn_srv, Logger * use_log = NULL);
  //
  virtual ~BillFacade();

  const CapSmDialogCfg & getDlgConfig(void) const { return _dlgCfg; }
  void  printDlgConfig(void) const { return _dlgCfg.printConfig(); }

  CapSmDialogCfg & getDlgConfig(void) { return _dlgCfg; }

  bool setAbonentId(unsigned ab_id, bool orig_abn = true)
  {
    return orig_abn ? _dlgCfg.setOrgAbnt(ab_id) : _dlgCfg.setDstAbnt(ab_id);
  }

  // -- INDialog management methods -- //
  CapSmDialog *  findDialog(unsigned int dlg_id) const;

  unsigned  getNextDialogId(void);
  unsigned  initDialog(unsigned int dlg_id = 0, bool batch_mode = false,
                        uint32_t delivery = 1016, const CapSmDialogCfg * use_cfg = NULL);

  void sendChargeSms(unsigned int dlg_id, uint32_t num_bytes = 0);
  void sendDeliveredSmsData(unsigned int dlg_id, uint32_t num_bytes = 0);
  void sendDeliverySmsResult(unsigned int dlg_id, uint32_t delivery_status, uint32_t num_bytes = 0);

  // ------------------------------------------------------------
  // -- PacketListenerIface interface methods:
  // ------------------------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception) */;
};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_BILLING_HPP */

