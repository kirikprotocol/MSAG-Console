#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeOctArray16.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * OctArray16Iface serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const OctArray16Iface & in_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type len = (PacketBufferAC::size_type)in_val.size();
  do {
    unsigned char l7b = (unsigned char)(len & 0x7F);
    if (len >>= 7)
      l7b |= 0x80;
    if (out_buf.Append(&l7b, 1) < 1)
      throw BufferOverflowException("OctArray16", 1);
  } while (len);
  
  if (!in_val.empty()
      && (out_buf.Append(&in_val[0], (PacketBufferAC::size_type)in_val.size()) < in_val.size()))
    throw BufferOverflowException("OctArray16", in_val.size());

  return out_buf;
}

//Note: deserialized data is appended to given OctArray16, instead of overwriting it
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, OctArray16Iface & out_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  OctArray16Iface::size_type len = 0;
  unsigned char l7b;

  do {
    if (in_buf.Read(&l7b, 1) < 1)
      throw BufferDataException(in_buf.getPos(), " >> OctArray16: corrupted size");
    len |= ((OctArray16Iface::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(OctArray16Iface::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(in_buf.getPos(), ">> OctArray16: length prefix is too large");

  if (len) {
    if ((len + out_val.size()) < len) //check container overflow
      throw DataContainerOverflowException("OctArray16", len);
  
    OctArray16Iface::size_type oldLen = out_val.size();
    out_val.resize(len + oldLen);
    if (in_buf.Read(&out_val[oldLen], (PacketBufferAC::size_type)len) < len)
      throw BufferDataException(in_buf.getPos(), " >> OctArray16: corrupted data");
  }
  return in_buf;
}

//Note: deserialized data is appended to given OctArray16, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, OctArray16Iface & out_val)
    throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}


template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <OctArray16Iface > (OctArray16Iface & out_val)
    throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  OctArray16Iface::size_type len = 0;
  unsigned char l7b;

  do {
    if (_pckBuf.Copy(&l7b, _pckBuf.getPos() + _curOfs + i, 1) < 1)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> OctArray16: corrupted size");
    len |= ((OctArray16Iface::size_type)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(OctArray16Iface::size_type)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(_pckBuf.getPos() + _curOfs + i, ">> OctArray16: length prefix is too large");

  if (len) {
    if ((len + out_val.size()) < len) //check container overflow
      throw DataContainerOverflowException("OctArray16", len);

    OctArray16Iface::size_type oldLen = out_val.size();
    out_val.resize(len + oldLen);
    if (_pckBuf.Copy(&out_val[oldLen], _pckBuf.getPos() + _curOfs + i, (PacketBufferAC::size_type)len) < len)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> OctArray16: corrupted data");
    i += len;
  }
  _curOfs += i;
  return *this;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

