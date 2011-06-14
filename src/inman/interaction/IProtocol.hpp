/* ************************************************************************* *
 * INMan generic interaction protocols definition
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INTERACTION_PROTOCOL_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_PROTOCOL_DEFS

#include <map>

#include "core/buffers/FixedStrings.hpp"
#include "inman/interaction/PacketBuffer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

//INMan interaction protocol
class IProtocolAC {
protected:
  IProtocolAC()
  { }

public:
  static const uint16_t _MAX_IDENT_LEN = 32;
  typedef smsc::core::buffers::FixedString_T<_MAX_IDENT_LEN + 1> IdentStr_t;
 
  typedef uint16_t  PduId; //non-zero unique id of packet

  virtual ~IProtocolAC()
  { }

  // -----------------------------------------
  // -- IProtocolAC interafce methods
  // -----------------------------------------
  //returns unique protocol ident
  virtual const IdentStr_t & ident(void)  const = 0;
  //If packet buffer corresponds to one of known PDUs, returns its PDU id,
  //zero otherwise.
  virtual PduId isKnownPacket(const PacketBufferAC & in_buf) const /*throw()*/= 0;
};
typedef IProtocolAC::IdentStr_t IProtocolId_t;

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_PROTOCOL_DEFS */

