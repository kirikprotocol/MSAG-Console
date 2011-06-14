#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "inman/interaction/serializer/SerializableObj.hpp"

#include "ussman/interaction/IProtoUSSG.hpp"

namespace smsc  {
namespace ussman {
namespace interaction {

using smsc::inman::interaction::IProtocolAC;
using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::SerializableObjIface;
using smsc::inman::interaction::operator >>;

/* ************************************************************************** *
 * INPUSSGateway protocol implementation:
 * ************************************************************************** */
const IProtocolAC::IdentStr_t INPUSSGateway::_protoId("ipUssGateway");

static const char * const _nm_cmd[] = { //matches INPUSSGateway::CommanTag_e
  "NOT_A_COMMAND", "PROCESS_USS_REQUEST", "PROCESS_USS_RESULT"
};

const char * INPUSSGateway::nameOfCmd(INPUSSGateway::CommandTag_e cmd_id)
{
  return ((uint16_t)cmd_id <= INPUSSGateway::PROCESS_USS_RESULT_TAG) ? _nm_cmd[(uint16_t)cmd_id] : _nm_cmd[0];
}

// -----------------------------------------
// -- IProtocolAC interface methods
// -----------------------------------------
IProtocolAC::PduId INPUSSGateway::isKnownPacket(const PacketBufferAC & in_buf) const
{
  smsc::inman::interaction::PacketBufferCursor  curBuf(in_buf, 0);
  SerializableObjIface::ident_type  cmdId = 0;

  try {
    curBuf >> cmdId;
  } catch (const SerializerException & exc) {
    return 0;
  }
  return isKnownCmd(cmdId) ? (PduId)cmdId : 0;
}

} //interaction
} //ussman
} //smsc
