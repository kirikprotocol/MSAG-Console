/* ************************************************************************** *
 * FixedLengthString<_SizeTArg> serialization operators.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_FXDLENSTR_T
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_FXDLENSTR_T

#include "core/buffers/FixedLengthString.hpp"
#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

/* -------------------------------------------------------------------------- *
 * FixedLengthString<_SizeTArg> serialization
 * -------------------------------------------------------------------------- */
template <size_t _SizeTArg>
PacketBufferAC & operator<<(PacketBufferAC & out_buf,
                            const smsc::core::buffers::FixedLengthString<_SizeTArg> & in_val)
  throw(SerializerException)
{
  size_t len = in_val.length();
  do {
    unsigned char l7b = (unsigned char)(len & 0x7F);
    if (len >>= 7)
      l7b |= 0x80;
    if (out_buf.Append(&l7b, 1) < 1)
      throw BufferOverflowException("FixedLengthString<>", 1);
  } while (len);

  if (!in_val.empty() && (out_buf.Append((const unsigned char*)in_val.c_str(),
                                 (PacketBufferAC::size_type)in_val.length()) < in_val.length()))
    throw BufferOverflowException("FixedLengthString<>", in_val.length());
  return out_buf;
}

//Note: deserialized data is appended to given string, instead of overwriting it
template <size_t _SizeTArg>
PacketBufferAC & operator>>(PacketBufferAC & in_buf,
                            smsc::core::buffers::FixedLengthString<_SizeTArg> & out_val)
  throw(SerializerException)
{
  PacketBufferAC::size_type i = 0;
  size_t len = 0;
  unsigned char l7b;
  do {
    if (in_buf.Read(&l7b, 1) < 1)
      throw BufferDataException(in_buf.getPos(), " >> FixedLengthString<>: corrupted length prefix");

    len |= ((size_t)(l7b & 0x7F) << (7*i++));
  } while ((l7b >= 0x80) && (i < (((sizeof(size_t)<<3)+6)/7)));

  if (l7b >= 0x80)
    throw BufferDataException(in_buf.getPos(), ">> FixedLengthString<>: length prefix is too large");

  if (len) {
    size_t iniLen = out_val.length();
    if (len >= (_SizeTArg - iniLen))  //consider ending zero
      throw DataContainerOverflowException("FixedLengthString<>", len);
    
    if (in_buf.Read((unsigned char*)out_val.str + iniLen,(PacketBufferAC::size_type)len) < len)
      throw BufferDataException(in_buf.getPos(), " >> FixedLengthString<>: corrupted data");
    out_val.str[iniLen + len] = 0;
  }
  return in_buf;
}


struct PacketBufferCursorFLS : public PacketBufferCursor {
  PacketBufferCursorFLS(const PacketBufferAC & use_buf, PacketBufferAC::size_type use_ofs)
    : PacketBufferCursor(use_buf, use_ofs)
  { }
  ~PacketBufferCursorFLS()
  { }

  //Note: deserialized data is appended to given string, instead of overwriting it
  template <size_t _SizeTArg>
    PacketBufferCursorFLS & operator>>(smsc::core::buffers::FixedLengthString<_SizeTArg> & out_val)
      throw(SerializerException)
  {
    PacketBufferAC::size_type i = 0;
    size_t len = 0;
    unsigned char l7b;
    do {
      if (_pckBuf.Copy(&l7b, _pckBuf.getPos() + _curOfs + i, 1) < 1)
        throw BufferDataException(_pckBuf.getPos() + _curOfs + i, " >> FixedLengthString<>: corrupted size");

      len |= ((size_t)(l7b & 0x7F) << (7*i++));
    } while ((l7b >= 0x80) && (i < (((sizeof(size_t)<<3)+6)/7)));

    if (l7b >= 0x80)
      throw BufferDataException(_pckBuf.getPos() + _curOfs + i, ">> FixedLengthString<>: length prefix is too large");

    if (len) {
      size_t iniLen = out_val.length();
      if (len >= (_SizeTArg - iniLen))  //consider ending zero
        throw DataContainerOverflowException("FixedLengthString<>", len);

      PacketBufferAC::size_type
        n = _pckBuf.Copy((unsigned char*)out_val.str + iniLen, _pckBuf.getPos() + _curOfs + i, (PacketBufferAC::size_type)len);
      if (n < len)
        throw BufferDataException(_pckBuf.getPos() + _curOfs + i + n, " >> FixedLengthString<>: corrupted data");
      i += len;
      out_val.str[iniLen + len] = 0;
    }
    _curOfs += i;
    return *this;
  }
};

//Note: deserialized data is appended to given string, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
template <size_t _SizeTArg>
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, smsc::core::buffers::FixedLengthString<_SizeTArg> & out_val)
    throw(SerializerException)
{
  PacketBufferCursorFLS  curs(in_buf, 0);
  curs.operator>><_SizeTArg >(out_val);
  return curs._curOfs;
}

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_FXDLENSTR_T */

