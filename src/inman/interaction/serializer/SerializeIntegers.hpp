/* ************************************************************************** *
 * Definition of serializer operators ('<<' and'>>') for native C99 integer 
 * and bool types.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER_INTS_BOOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_INTS_BOOL

#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * unsigned ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned long long & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned long long & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned long long & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned long long > (unsigned long long & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned long & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned long & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned long & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned long > (unsigned long & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned int & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned int & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned int & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned int > (unsigned int & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned short & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned short & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned short & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned short > (unsigned short & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- *
 * signed ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const long long & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, long long & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, long long & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <long long > (long long & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const long & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, long & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, long & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <long > (long & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int > (int & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const short & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, short & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, short & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <short > (short & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- *
 * char serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const unsigned char & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, unsigned char & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, unsigned char & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <unsigned char > (unsigned char & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- *
 * bool serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const bool & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, bool & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, bool & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <bool > (bool & out_val) throw(SerializerException);

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER_INTS_BOOL */

