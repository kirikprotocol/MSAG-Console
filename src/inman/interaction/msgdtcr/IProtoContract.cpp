#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/msgdtcr/IProtoContract.hpp"
#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "inman/interaction/serializer/SerializableObj.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * INPAbntContract protocol implementation:
 * ************************************************************************** */
const IProtocolAC::IdentStr_t INPAbntContract::_protoId("ipAbntContract");

// -----------------------------------------
// -- IProtocolAC interface methods
// -----------------------------------------
IProtocolAC::PduId INPAbntContract::isKnownPacket(const PacketBufferAC & in_buf) const
{
  PacketBufferCursor                curBuf(in_buf, 0);
  SerializableObjIface::ident_type  cmdId = 0, hdrFrm = 0;

  try {
    curBuf >> cmdId;
    curBuf >> hdrFrm;
  } catch (const SerializerException & exc) {
    return 0;
  }
  return (isKnownCmd(cmdId) && isKnownHdr(hdrFrm)) ? mkPduId(cmdId, hdrFrm) : 0;
}

} //interaction
} //inman
} //smsc
