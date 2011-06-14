#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeVector.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * std::vector serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const std::vector<unsigned char> & in_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type len = (PacketBufferAC::size_type)in_val.size();
  do {
    unsigned char l7b = (unsigned char)(len & 0x7F);
    if (len >>= 7)
      l7b |= 0x80;
    if (out_buf.Append(&l7b, 1) < 1)
      throw BufferOverflowException("vector", 1);
  } while (len);
  
  len = (PacketBufferAC::size_type)in_val.size();
  if (len && (out_buf.Append(&in_val[0], len) < len))
    throw BufferOverflowException("vector", len);

  return out_buf;
}

//Note: deserialized data is appended to given vector, instead of overwriting it
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, std::vector<unsigned char> & out_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  std::vector<unsigned char>::size_type len = 0;
  unsigned char l7b;

  do {
    if (in_buf.Read(&l7b, 1) < 1)
      throw BufferDataException(in_buf.getPos(), " >> vector: corrupted size");
    len |= ((std::vector<unsigned char>::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(std::vector<unsigned char>::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(in_buf.getPos(), ">> vector: length prefix is too large");

  if (len) {
    if ((len + out_val.size()) < len) //check container overflow
      throw DataContainerOverflowException("vector", len);
  
    std::vector<unsigned char>::size_type oldLen = out_val.size();
    out_val.resize(len + oldLen);
    if (in_buf.Read(&out_val[oldLen], (PacketBufferAC::size_type)len) < len)
      throw BufferDataException(in_buf.getPos(), " >> vector: corrupted data");
  }
  return in_buf;
}

//Note: deserialized data is appended to given vector, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, std::vector<unsigned char> & out_val)
    throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}


template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <std::vector<unsigned char> > (std::vector<unsigned char> & out_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  std::vector<unsigned char>::size_type len = 0;
  unsigned char l7b;

  do {
    if (_pckBuf.Copy(&l7b, _pckBuf.getPos() + _curOfs + i, 1) < 1)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> vector: corrupted size");
    len |= ((std::vector<unsigned char>::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(std::vector<unsigned char>::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(_pckBuf.getPos() + _curOfs + i, ">> vector: length prefix is too large");

  if (len) {
    if ((len + out_val.size()) < len) //check container overflow
      throw DataContainerOverflowException("vector", len);

    std::vector<unsigned char>::size_type oldLen = out_val.size();
    out_val.resize(len + oldLen);
    if (_pckBuf.Copy(&out_val[oldLen], _pckBuf.getPos() + _curOfs + i, (PacketBufferAC::size_type)len) < len)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> vector: corrupted data");
    i += (PacketBufferAC::size_type)len;
  }
  _curOfs += i;
  return *this;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

