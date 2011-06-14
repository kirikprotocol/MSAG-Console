#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "ussman/interaction/USSGMessages.hpp"

namespace smsc  {
namespace ussman {
namespace interaction {

using smsc::inman::interaction::operator <<;
using smsc::inman::interaction::operator >>;

/* ************************************************************************** *
 * class USSGPacketAC implementation:
 * ************************************************************************** */
void USSGPacketAC::serialize(PacketBufferAC & out_buf) const
  throw(SerializerException)
{
  out_buf << at(0).Id();
  out_buf << dlgId;
  at(0).save(out_buf);
}

USSGPacketAC::DsrlzMode_e
  USSGPacketAC::deserialize(PacketBufferAC & in_buf,
                           USSGPacketAC::DsrlzMode_e use_mode/* = dsmComplete*/)
  throw(SerializerException)
{
  smsc::inman::interaction::SerializableObjID tId;
  in_buf >> tId;
  if (tId != at(0).Id())
    throw SerializerException("mismatched cmdId", SerializerException::invObject, NULL);

  in_buf >> dlgId;
  if (use_mode == SerializablePacketIface::dsmComplete)
    at(0).load(in_buf);

  return use_mode;
}

} //interaction
} //ussman
} //smsc

