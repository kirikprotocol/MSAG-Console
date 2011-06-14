#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeStdString.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * std::string serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC & operator<<(PacketBufferAC & out_buf, const std::string & in_val)
  throw(SerializerException)
{
  std::string::size_type  len = in_val.size();
  do {
    unsigned char l7b = (unsigned char)(len & 0x7F);
    if (len >>= 7)
      l7b |= 0x80;
    if (out_buf.Append(&l7b, 1) < 1)
      throw BufferOverflowException("string", 1);
  } while (len);

  len = in_val.size();
  if (len && (out_buf.Append((const unsigned char*)in_val.c_str(), (PacketBufferAC::size_type)len) < len))
    throw BufferOverflowException("string", len);
  return out_buf;
}

//Note: deserialized data is appended to given string, instead of overwriting it
PacketBufferAC & operator>>(PacketBufferAC & in_buf, std::string & out_val)
  throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  std::string::size_type len = 0;
  unsigned char l7b;
  do {
    if (in_buf.Read(&l7b, 1) < 1)
      throw BufferDataException(in_buf.getPos(), " >> string: corrupted size");
    len |= ((std::string::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(std::string::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(in_buf.getPos(), ">> string: length prefix is too large");

  while (len) {
    char strBuf[255 + 1];
    unsigned num2r = (len <= 255) ? (unsigned)len : 255;
    
    if (in_buf.Read((unsigned char*)strBuf, num2r) < num2r)
      throw BufferDataException(in_buf.getPos(), " >> string: corrupted data");

    strBuf[num2r] = 0;
    out_val += strBuf;
    len -= num2r;
  }
  return in_buf;
}

//Note: deserialized data is appended to given string, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, std::string & out_val)
    throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}

template <>
PacketBufferCursor &
  PacketBufferCursor::operator>> <std::string > (std::string & out_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  std::string::size_type len = 0;
  unsigned char l7b;
  do {
    if (_pckBuf.Copy(&l7b, _pckBuf.getPos() + _curOfs + i, 1) < 1)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> string: corrupted size");
    len |= ((std::string::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(std::string::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(_pckBuf.getPos() + _curOfs + i, ">> string: length prefix is too large");

  while (len) {
    char strBuf[255 + 1];
    unsigned num2r = (len <= 255) ? (unsigned)len : 255;

    if (_pckBuf.Copy((unsigned char*)strBuf, _pckBuf.getPos() + _curOfs + i, num2r) < num2r)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> string: corrupted data");

    strBuf[num2r] = 0;
    out_val += strBuf;
    len -= num2r;
    i += num2r;
  }
  _curOfs += i;
  return *this;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

