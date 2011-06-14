/* ************************************************************************** *
 * Definition of serializer operators ('<<' and'>>') for time types.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER_TIME
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_TIME

#include <time.h>

#include "inman/interaction/serializer/SerializerDefs.hpp"
#include "inman/interaction/PacketBuffer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * time_t serialization
 * NOTE: time_t is a signed integer type consisting of at least 4 bytes!
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const time_t & in_val) throw(SerializerException);
//
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, time_t & out_val) throw(SerializerException);
//
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, time_t & out_val) throw(SerializerException);

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <time_t> (time_t & out_val) throw(SerializerException);

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER_TIME */

