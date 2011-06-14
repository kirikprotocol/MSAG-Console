/* ************************************************************************* *
 * Unstructured Supplementary Service Gateway protocol definition.
 * ************************************************************************* */
#ifndef SMSC_USSMAN_IPROTO_DEF
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_USSMAN_IPROTO_DEF

#include "inman/interaction/IProtocol.hpp"

namespace smsc  {
namespace ussman {
namespace interaction {

using smsc::inman::interaction::PacketBufferAC;

// -------------------------------------------------------------------- //
// Interaction Protocol: USS Gateway
// -------------------------------------------------------------------- //
class INPUSSGateway : public smsc::inman::interaction::IProtocolAC {
public:
  //Note: PduId == CommandId
  enum CommandTag_e {
    NOT_A_COMMAND           = 0
  , PROCESS_USS_REQUEST_TAG = 1 // 1. USSRequest        ( client --> USSMan )
  , PROCESS_USS_RESULT_TAG  = 2 // 2. USSRequestResult  ( client <-- USSMan )
  };

  static const IdentStr_t   _protoId; // "ipUssGateway"

  INPUSSGateway() : IProtocolAC()
  { }
  ~INPUSSGateway()
  { }

  static const char * nameOfCmd(CommandTag_e cmd_id);

  static bool isKnownCmd(uint16_t cmd_id)
  {
    return (cmd_id == PROCESS_USS_REQUEST_TAG) || (cmd_id == PROCESS_USS_RESULT_TAG);
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
} //ussman
} //smsc

#endif /* SMSC_USSMAN_IPROTO_DEF */

