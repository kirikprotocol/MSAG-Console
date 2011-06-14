#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/msgbill/IProtoBilling.hpp"
#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "inman/interaction/serializer/SerializableObj.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * INPBilling protocol implementation:
 * ************************************************************************** */
const IProtocolAC::IdentStr_t INPBilling::_protoId("ipBilling");

static const char * const _nm_cmd[] = { //matches INPBilling::CommanTag_e
  "NOT_A_COMMAND", "CHARGE_SMS", "CHARGE_SMS_RESULT", "DELIVERY_SMS_RESULT", "DELIVERED_SMS_DATA"
};

const char * INPBilling::nameOfCmd(INPBilling::CommandTag_e cmd_id)
{
  return ((uint16_t)cmd_id <= INPBilling::DELIVERED_SMS_DATA_TAG) ? _nm_cmd[(uint16_t)cmd_id] : _nm_cmd[0];
}

// -----------------------------------------
// -- IProtocolAC interface methods
// -----------------------------------------
IProtocolAC::PduId INPBilling::isKnownPacket(const PacketBufferAC & in_buf) const
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
