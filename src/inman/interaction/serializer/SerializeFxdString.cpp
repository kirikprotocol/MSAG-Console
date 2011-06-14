#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeFxdString.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::core::buffers::FixedStringIface;

/* -------------------------------------------------------------------------- *
 * FixedStringIface serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC & operator<<(PacketBufferAC & out_buf, const FixedStringIface & in_val)
  throw(SerializerException)
{
  FixedStringIface::size_type  len = in_val.size();
  do {
    uint8_t l7b = (uint8_t)(len & 0x7F);
    if (len >>= 7)
      l7b |= 0x80;
    if (out_buf.Append(&l7b, 1) < 1)
      throw BufferOverflowException("FixedStringIface", 1);
  } while (len);

  len = in_val.size();
  if (len && (out_buf.Append((const uint8_t*)in_val.c_str(),
                         (PacketBufferAC::size_type)len) < len))
    throw BufferOverflowException("FixedStringIface", len);
  return out_buf;
}

//Note: deserialized data is appended to given string, instead of overwriting it
PacketBufferAC & operator>>(PacketBufferAC & in_buf, FixedStringIface & out_val)
  throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  FixedStringIface::size_type len = 0;
  uint8_t l7b;
  do {
    if (in_buf.Read(&l7b, 1) < 1)
      throw BufferDataException(in_buf.getPos(), " >> FixedStringIface: corrupted length prefix");
    len |= ((FixedStringIface::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(FixedStringIface::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(in_buf.getPos(), ">> FixedStringIface: length prefix is too large");

  if (len) {
    FixedStringIface::size_type   iniLen = out_val.length();
    if (len > (out_val.capacity() - iniLen))
      throw DataContainerOverflowException("FixedStringIface", len);
    if (in_buf.Read((uint8_t*)out_val.v_str() + iniLen, (PacketBufferAC::size_type)len) < len)
      throw BufferDataException(in_buf.getPos(), " >> FixedStringIface: corrupted data");
    out_val.v_str()[iniLen + len] = 0;
  }
  return in_buf;
}

//Note: deserialized data is appended to given string, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, FixedStringIface & out_val)
    throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <FixedStringIface > (FixedStringIface & out_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  FixedStringIface::size_type len = 0;
  uint8_t l7b;
  do {
    if (_pckBuf.Copy(&l7b, _pckBuf.getPos() + _curOfs + i, 1) < 1)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> FixedStringIface: corrupted size");
    len |= ((FixedStringIface::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(FixedStringIface::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(_pckBuf.getPos() + _curOfs + i, ">> FixedStringIface: length prefix is too large");

  if (len) {
    FixedStringIface::size_type   iniLen = out_val.length();
    if (len > (out_val.capacity() - iniLen))
      throw DataContainerOverflowException("FixedStringIface", len);

    PacketBufferAC::size_type
      n = _pckBuf.Copy((uint8_t*)out_val.v_str() + iniLen,
                        _pckBuf.getPos() + _curOfs + i, (PacketBufferAC::size_type)len);
    if (n < len)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i + n, " >> FixedStringIface: corrupted data");
    
    out_val.v_str()[iniLen + len] = 0;
    i += len;
  }
  _curOfs += i;
  return *this;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

