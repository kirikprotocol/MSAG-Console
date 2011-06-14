#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/IMessages.hpp"
#include "inman/interaction/serializer/SerializeIntegers.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

// ------------------------------------------------------
// -- SerializablePacketIface interface implementation
// ------------------------------------------------------
void INPPacketIface::serialize(PacketBufferAC & out_buf) const throw(SerializerException)
{
  out_buf << at(1).Id();
  out_buf << at(0).Id();
  at(0).save(out_buf);
  at(1).save(out_buf);
}
//
SerializablePacketIface::DsrlzMode_e
  INPPacketIface::deserialize(PacketBufferAC & in_buf,
                              DsrlzMode_e use_mode/* = dsmComplete*/) throw(SerializerException)
{
  SerializableObjID tId;
  in_buf >> tId;
  if (tId != at(1).Id())
    throw SerializerException("mismatched cmdId", SerializerException::invObject, NULL);
  in_buf >> tId;
  if (tId != at(0).Id())
    throw SerializerException("mismatched hdrId", SerializerException::invObject, NULL);

  at(0).load(in_buf);
  if (use_mode == SerializablePacketIface::dsmComplete)
    at(1).load(in_buf);

  return use_mode;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

