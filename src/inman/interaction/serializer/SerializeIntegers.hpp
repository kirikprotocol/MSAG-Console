/* ************************************************************************** *
 * Definition of serializer operators ('<<' and'>>') for Integer and bool types.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER_INTS_BOOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_INTS_BOOL

#include <inttypes.h>

#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * unsigned ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const uint64_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, uint64_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, uint64_t & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <uint64_t > (uint64_t & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const uint32_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, uint32_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, uint32_t & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <uint32_t > (uint32_t & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const uint16_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, uint16_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, uint16_t & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <uint16_t > (uint16_t & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- *
 * signed ints serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int64_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int64_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int64_t & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int64_t > (int64_t & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int32_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int32_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int32_t & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int32_t > (int32_t & out_val) throw(SerializerException);

/* -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const int16_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, int16_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, int16_t & out_val) throw(SerializerException);
//
template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <int16_t > (int16_t & out_val) throw(SerializerException);

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

