#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <arpa/inet.h>

#include "util/Uint64Converter.h"

#include "inman/interaction/serializer/SerializeIntegers.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * unsigned ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const uint64_t & in_val) throw(SerializerException)
{
  uint64_t nval = smsc::util::Uint64Converter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, 8) < 8)
    throw BufferOverflowException("uint64_t", 8);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, uint64_t & out_val) throw(SerializerException)
{
  uint64_t nval;
  if (in_buf.Read((unsigned char*)&nval, 8) < 8)
    throw BufferDataException(in_buf.getPos(), " >> uint64_t: corrupted data");

  out_val = smsc::util::Uint64Converter::toHostOrder(nval);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, uint64_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <uint64_t > (uint64_t & out_val) throw(SerializerException)
{
  uint64_t nval;
  if (_pckBuf.Copy((unsigned char*)&nval, _pckBuf.getPos() + _curOfs, 8) < 8)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> uint64_t: corrupted data");

  _curOfs += 8;
  out_val = smsc::util::Uint64Converter::toHostOrder(nval);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const uint32_t & in_val) throw(SerializerException)
{
  uint32_t nval = htonl(in_val);
  if (out_buf.Append((unsigned char*)&nval, 4) < 4)
    throw BufferOverflowException("uint32_t", 4);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, uint32_t & out_val) throw(SerializerException)
{
  if (in_buf.Read((unsigned char*)&out_val, 4) < 4)
    throw BufferDataException(in_buf.getPos(), " >> uint32_t: corrupted data");
  out_val = ntohl(out_val);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, uint32_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <uint32_t > (uint32_t & out_val)
    throw(SerializerException)
{
  if (_pckBuf.Copy((unsigned char*)&out_val, _pckBuf.getPos() + _curOfs, 4) < 4)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> uint32_t: corrupted data");

  _curOfs += 4;
  out_val = ntohl(out_val);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const uint16_t & in_val) throw(SerializerException)
{
  uint16_t nval = htons(in_val);
  if (out_buf.Append((unsigned char*)&nval, 2) < 2)
    throw BufferOverflowException("uint16_t", 2);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, uint16_t & out_val) throw(SerializerException)
{
  if (in_buf.Read((unsigned char*)&out_val, 2) < 2)
    throw BufferDataException(in_buf.getPos(), " >> uint16_t: corrupted data");
  out_val = ntohs(out_val);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, uint16_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <uint16_t > (uint16_t & out_val)
    throw(SerializerException)
{
  if (_pckBuf.Copy((unsigned char*)&out_val, _pckBuf.getPos() + _curOfs, 2) < 2)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> uint16_t: corrupted data");
  _curOfs += 2;
  out_val = ntohs(out_val);
  return *this;
}

/* -------------------------------------------------------------------------- *
 * signed ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int64_t & in_val) throw(SerializerException)
{
  uint64_t nval = smsc::util::Uint64Converter::toNetworkOrder((uint64_t)in_val);
  if (out_buf.Append((unsigned char*)&nval, 8) < 8)
    throw BufferOverflowException("int64_t", 8);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int64_t & out_val) throw(SerializerException)
{
  uint64_t nval;
  if (in_buf.Read((unsigned char*)&nval, 8) < 8)
    throw BufferDataException(in_buf.getPos(), " >> int64_t: corrupted data");

  out_val = (int64_t)smsc::util::Uint64Converter::toHostOrder(nval);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int64_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int64_t > (int64_t & out_val) throw(SerializerException)
{
  uint64_t nval;
  if (_pckBuf.Copy((unsigned char*)&nval, _pckBuf.getPos() + _curOfs, 8) < 8)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> int64_t: corrupted data");

  _curOfs += 8;
  out_val = (int64_t)smsc::util::Uint64Converter::toHostOrder(nval);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int32_t & in_val) throw(SerializerException)
{
  uint32_t nval = htonl((uint32_t)in_val);
  if (out_buf.Append((unsigned char*)&nval, 4) < 4)
    throw BufferOverflowException("int32_t", 4);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int32_t & out_val) throw(SerializerException)
{
  if (in_buf.Read((unsigned char*)&out_val, 4) < 4)
    throw BufferDataException(in_buf.getPos(), " >> int32_t: corrupted data");
  out_val = (int32_t)ntohl((uint32_t)out_val);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int32_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int32_t > (int32_t & out_val) throw(SerializerException)
{
  if (_pckBuf.Copy((unsigned char*)&out_val, _pckBuf.getPos() + _curOfs, 4) < 4)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> int32_t: corrupted data");

  _curOfs += 4;
  out_val = (int32_t)ntohl((uint32_t)out_val);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int16_t & in_val) throw(SerializerException)
{
  uint16_t nval = htons((uint16_t)in_val);
  if (out_buf.Append((unsigned char*)&nval, 2) < 2)
    throw BufferOverflowException("int16_t", 2);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int16_t & out_val) throw(SerializerException)
{
  if (in_buf.Read((unsigned char*)&out_val, 2) < 2)
    throw BufferDataException(in_buf.getPos(), " >> int16_t: corrupted data");
  out_val = (int16_t)ntohs((uint16_t)out_val);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int16_t & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int16_t > (int16_t & out_val) throw(SerializerException)
{
  if (_pckBuf.Copy((unsigned char*)&out_val, _pckBuf.getPos() + _curOfs, 2) < 2)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> int16_t: corrupted data");

  _curOfs += 2;
  out_val = (int16_t)ntohs((uint16_t)out_val);
  return *this;
}

/* -------------------------------------------------------------------------- *
 * char serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned char & in_val) throw(SerializerException)
{
  if (out_buf.Append(&in_val, 1) < 1)
    throw BufferOverflowException("uchar_t", 1);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned char & out_val) throw(SerializerException)
{
  if (in_buf.Read(&out_val, 1) < 1)
    throw BufferDataException(in_buf.getPos(), " >> uchar_t: corrupted data");
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned char & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned char > (unsigned char & out_val) throw(SerializerException)
{
  if (_pckBuf.Copy(&out_val, _pckBuf.getPos() + _curOfs, 1) < 1)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> uchar_t: corrupted data");

  ++_curOfs;
  return *this;
}

/* -------------------------------------------------------------------------- *
 * bool serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const bool & in_val) throw(SerializerException)
{
  unsigned char nval = in_val ? 0xFF : 0;
  if (out_buf.Append(&nval, 1) < 1)
    throw BufferOverflowException("bool", 1);
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, bool & out_val) throw(SerializerException)
{
  unsigned char nval;
  if (in_buf.Read(&nval, 1) < 1)
    throw BufferDataException(in_buf.getPos(), " >> bool: corrupted data");
  out_val = nval ? true : false;
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, bool & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <bool > (bool & out_val) throw(SerializerException)
{
  unsigned char nval;
  if (_pckBuf.Copy(&nval, _pckBuf.getPos() + _curOfs, 1) < 1)
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> bool: corrupted data");

  out_val = nval ? true : false;
  ++_curOfs;
  return *this;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

