/* ************************************************************************* *
 * Abonent Contract and gsmSCFs parameters determination protocol.
 * ************************************************************************* */
#ifndef SMSC_INMAN_IPROTO_ABNT_CONTRACT
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IPROTO_ABNT_CONTRACT

#include "inman/interaction/IProtocol.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

// -------------------------------------------------------------------- //
// INMan Interaction Protocol: Abonent Contract detection
// -------------------------------------------------------------------- //
class INPAbntContract : public IProtocolAC {
public:
  enum CommandTag_e {
    ABNT_CONTRACT_REQUEST_TAG  = 6,    // AbntContractRequest  ( USSDC --> INMAN )
    ABNT_CONTRACT_RESULT_TAG   = 7     // AbntContractResult   ( USSDC <-- INMAN )
  };

  enum HeaderFrm_E { HDR_DIALOG = 1 };

  static const IdentStr_t   _protoId; // "ipAbntContract"

  INPAbntContract()  : IProtocolAC()
  { }
  ~INPAbntContract()
  { }

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
    return (cmd_id <= ABNT_CONTRACT_RESULT_TAG) && (cmd_id >= ABNT_CONTRACT_REQUEST_TAG);
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

#endif /* SMSC_INMAN_IPROTO_ABNT_CONTRACT */

