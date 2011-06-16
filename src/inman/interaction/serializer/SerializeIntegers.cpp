#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "util/NetHostConverter.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * unsigned ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned long long & in_val) throw(SerializerException)
{
  unsigned long long nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(unsigned long long)) < sizeof(unsigned long long))
    throw BufferOverflowException("unsigned long long", (unsigned long)sizeof(unsigned long long));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned long long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned long long> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(unsigned long long)) < sizeof(unsigned long long))
    throw BufferDataException(in_buf.getPos(), " >> unsigned long long: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned long long & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned long long > (unsigned long long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned long long> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(unsigned long long)) < sizeof(unsigned long long))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> unsigned long long: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(unsigned long long);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned long & in_val) throw(SerializerException)
{
  unsigned long nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(unsigned long)) < sizeof(unsigned long))
    throw BufferOverflowException("unsigned long", (unsigned long)sizeof(unsigned long));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned long> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(unsigned long)) < sizeof(unsigned long))
    throw BufferDataException(in_buf.getPos(), " >> unsigned long: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned long & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned long > (unsigned long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned long> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(unsigned long)) < sizeof(unsigned long))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> unsigned long: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(unsigned long);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned int & in_val) throw(SerializerException)
{
  unsigned int nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(unsigned int)) < sizeof(unsigned int))
    throw BufferOverflowException("unsigned int", (unsigned long)sizeof(unsigned int));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned int & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned int> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(unsigned int)) < sizeof(unsigned int))
    throw BufferDataException(in_buf.getPos(), " >> unsigned int: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned int & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned int > (unsigned int & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned int> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(unsigned int)) < sizeof(unsigned int))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> unsigned int: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(unsigned int);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned short & in_val) throw(SerializerException)
{
  unsigned short nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(unsigned short)) < sizeof(unsigned short))
    throw BufferOverflowException("unsigned short", (unsigned long)sizeof(unsigned short));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned short & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned short> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(unsigned short)) < sizeof(unsigned short))
    throw BufferDataException(in_buf.getPos(), " >> unsigned short: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned short & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned short > (unsigned short & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<unsigned short> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(unsigned short)) < sizeof(unsigned short))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> unsigned short: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(unsigned short);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}


/* -------------------------------------------------------------------------- *
 * signed ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const long long & in_val) throw(SerializerException)
{
  long long nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(long long)) < sizeof(long long))
    throw BufferOverflowException("long long", (unsigned long)sizeof(long long));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, long long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<long long> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(long long)) < sizeof(long long))
    throw BufferDataException(in_buf.getPos(), " >> long long: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, long long & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <long long > (long long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<long long> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(long long)) < sizeof(long long))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> long long: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(long long);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const long & in_val) throw(SerializerException)
{
  long nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(long)) < sizeof(long))
    throw BufferOverflowException("long", (unsigned long)sizeof(long));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<long> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(long)) < sizeof(long))
    throw BufferDataException(in_buf.getPos(), " >> long: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, long & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <long > (long & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<long> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(long)) < sizeof(long))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> long: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(long);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int & in_val) throw(SerializerException)
{
  int nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(int)) < sizeof(int))
    throw BufferOverflowException("int", (unsigned long)sizeof(int));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<int> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(int)) < sizeof(int))
    throw BufferDataException(in_buf.getPos(), " >> int: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int > (int & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<int> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(int)) < sizeof(int))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> int: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(int);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return *this;
}

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const short & in_val) throw(SerializerException)
{
  short nval = smsc::util::NetHostConverter::toNetworkOrder(in_val);
  if (out_buf.Append((unsigned char*)&nval, (PacketBufferAC::size_type)sizeof(short)) < sizeof(short))
    throw BufferOverflowException("short", (unsigned long)sizeof(short));
  return out_buf;
}
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, short & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<short> nval;
  if (in_buf.Read(nval._buf, (PacketBufferAC::size_type)sizeof(short)) < sizeof(short))
    throw BufferDataException(in_buf.getPos(), " >> short: corrupted data");

  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
  return in_buf;
}
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, short & out_val) throw(SerializerException)
{
  PacketBufferCursor  curs(in_buf, 0);
  curs >> out_val;
  return curs._curOfs;
}
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <short > (short & out_val) throw(SerializerException)
{
  smsc::util::AlignedInt_T<short> nval;
  if (_pckBuf.Copy(nval._buf, _pckBuf.getPos() + _curOfs, (PacketBufferAC::size_type)sizeof(short)) < sizeof(short))
    throw BufferDataException(_pckBuf.getPos() + _curOfs, " >> short: corrupted data");

  _curOfs += (PacketBufferAC::size_type)sizeof(short);
  out_val = smsc::util::NetHostConverter::toHostOrder(nval._ival);
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

