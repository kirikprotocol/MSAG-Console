/* ************************************************************************* *
 * SMS/USSD messages charging protocol definition.
 * ************************************************************************* */
#ifndef SMSC_INMAN_IPROTO_SMBILLING
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IPROTO_SMBILLING

#include "inman/interaction/IProtocol.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
// -------------------------------------------------------------------- //
// INMan Interaction Protocol: SMS/USSD Billing
// -------------------------------------------------------------------- //
class INPBilling : public IProtocolAC {
public:
  enum CommandTag_e {
    NOT_A_COMMAND           = 0,
    CHARGE_SMS_TAG          = 1,    // 1. ChargeSms         ( SMSC --> INMAN )
    CHARGE_SMS_RESULT_TAG   = 2,    // 2. ChargeSmsResult   ( SMSC <-- INMAN )
    DELIVERY_SMS_RESULT_TAG = 3,    // 3. DeliverySmsResult ( SMSC --> INMAN )
    DELIVERED_SMS_DATA_TAG  = 4     // 4. DeliveredSmsData  ( SMSC --> INMAN )
  };
  enum HeaderFrm_e {
    HDR_DIALOG = 1 /*, HDR_SESSIONED_DLG = 2*/
  };

  static const IdentStr_t   _protoId; // "ipBilling"

  INPBilling() : IProtocolAC()
  { }
  ~INPBilling()
  { }

  static const char * nameOfCmd(CommandTag_e cmd_id);

  static PduId mkPduId(uint16_t cmd_id, uint16_t hdr_id)
  {
    return (cmd_id << 8) | (hdr_id & 0x0F);
  }
  static uint16_t getCmdId(PduId pdu_id)
  {
    return (uint16_t)(pdu_id >> 8);
  }
  static bool isKnownCmd(uint16_t cmd_id)
  {
    return (cmd_id <= DELIVERED_SMS_DATA_TAG) && (cmd_id >= CHARGE_SMS_TAG);
  }

  static bool isKnownHdr(uint16_t obj_id)
  {
    return (obj_id == HDR_DIALOG);
  }

  // -----------------------------------------
  // -- IProtocolAC interafce methods
  // -----------------------------------------
  //returns unique protocol ident
  virtual const IdentStr_t & ident(void)  const { return _protoId; }
  //
  virtual PduId isKnownPacket(const PacketBufferAC & in_buf) const;
};

} //interaction
} //inman
} //smsc

#endif /* SMSC_INMAN_IPROTO_SMBILLING */

