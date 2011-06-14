/* ************************************************************************* *
 * Helper container of SMS/USSD billing requester messages.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_BILLING_REQUESTER_MESSAGES
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_BILLING_REQUESTER_MESSAGES

#include "util/MaxSizeof.hpp"
#include "inman/interaction/msgbill/MsgBilling.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

class SmBillRequestMsg {
private:
  union {
    void *  _aligner;
    uint8_t _buf[smsc::util::MaxSizeOf3_T<SPckChargeSms, SPckDeliveredSmsData, SPckDeliverySmsResult>::VALUE];
  } _mem;

  void _construct(void)
  {
    if (_cmdId == INPBilling::CHARGE_SMS_TAG)
      u._chgSms = new (_mem._buf) SPckChargeSms();
    else if (_cmdId == INPBilling::DELIVERED_SMS_DATA_TAG)
      u._chgDSms = new (_mem._buf) SPckDeliveredSmsData();
    else if (_cmdId == INPBilling::DELIVERY_SMS_RESULT_TAG)
      u._dlvrRes = new (_mem._buf) SPckDeliverySmsResult();
    else //_cmdId = INPBilling::NOT_A_COMMAND;
      u._ptr = NULL;
  }

public:
  INPBilling::CommandTag_e  _cmdId;
  union {
    INPPacketIface *        _ptr;
    SPckChargeSms *         _chgSms;
    SPckDeliveredSmsData *  _chgDSms;
    SPckDeliverySmsResult * _dlvrRes;
  } u;

  explicit SmBillRequestMsg(INPBilling::CommandTag_e cmd_id = INPBilling::NOT_A_COMMAND)
    : _cmdId(cmd_id)
  {
    _construct();
  }
  ~SmBillRequestMsg()
  {
    clear();
  }

  void init(INPBilling::CommandTag_e cmd_id)
  {
    _cmdId = cmd_id; _construct();
  }
  //
  void clear(void)
  {
    if (_cmdId == INPBilling::CHARGE_SMS_TAG)
      u._chgSms->~SPckChargeSms();
    else if (_cmdId == INPBilling::DELIVERED_SMS_DATA_TAG)
      u._chgDSms->~SPckDeliveredSmsData();
    else if (_cmdId == INPBilling::DELIVERY_SMS_RESULT_TAG)
      u._dlvrRes->~SPckDeliverySmsResult();

    _cmdId = INPBilling::NOT_A_COMMAND;
    u._ptr = NULL;
  }

  //
  SerializablePacketIface::DsrlzMode_e
    deserialize(PacketBufferAC & in_buf, SerializablePacketIface::DsrlzMode_e use_mode)
      throw(SerializerException)
  {
    if (_cmdId == INPBilling::CHARGE_SMS_TAG)
      return u._chgSms->deserialize(in_buf, use_mode);
    if (_cmdId == INPBilling::DELIVERED_SMS_DATA_TAG)
      return u._chgDSms->deserialize(in_buf, use_mode);
    if (_cmdId == INPBilling::DELIVERY_SMS_RESULT_TAG)
      return u._dlvrRes->deserialize(in_buf, use_mode);
    return use_mode;
  }

  uint32_t getDlgId(void) const
  {
    if (_cmdId == INPBilling::CHARGE_SMS_TAG)
      return u._chgSms->_Hdr.dlgId;
    if (_cmdId == INPBilling::DELIVERED_SMS_DATA_TAG)
      return u._chgDSms->_Hdr.dlgId;
    if (_cmdId == INPBilling::DELIVERY_SMS_RESULT_TAG)
      return u._dlvrRes->_Hdr.dlgId;
    return 0;
  }
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_BILLING_REQUESTER_MESSAGES */

