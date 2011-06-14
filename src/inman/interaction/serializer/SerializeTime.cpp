#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeTime.hpp"

#include <arpa/inet.h>

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * time_t serialization
 * NOTE: time_t is a signed integer type consisting of at least 4 bytes!
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const time_t & in_val) throw(SerializerException)
{
  uint32_t nval = htonl((uint32_t)in_val);
  if (out_buf.Append((uint8_t*)&nval, 4) < 4)
    throw BufferOverflowException("time_t", 4);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, time_t & out_val) throw(SerializerException)
{   
  uint32_t nval;
  if (in_buf.Read((uint8_t*)&nval, 4) < 4)
    throw BufferDataException(in_buf.getPos(), " >> time_t: corrupted data");

  out_val = (time_t)ntohl(nval);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, time_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <time_t>(time_t & out_val)
    throw(SerializerException)
{
  uint32_t nval;
  if (_pckBuf.Copy((uint8_t*)&nval, _pckBuf.getPos() + _curOfs, 4) < 4)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> time_t: corrupted data");

  _curOfs += 4;
  out_val = (time_t)ntohl(nval);
  return *this;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

