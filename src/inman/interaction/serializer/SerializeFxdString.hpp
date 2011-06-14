/* ************************************************************************** *
 * smsc::core::buffers::FixedStringIface serialization operators.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_FXDSTR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_FXDSTR

#include "core/buffers/FixedStrings.hpp"
#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * smsc::core::buffers::FixedStringIface serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const smsc::core::buffers::FixedStringIface & in_val)
    throw(SerializerException);

//Note: deserialized data is appended to given string, instead of overwriting it
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, smsc::core::buffers::FixedStringIface & out_val)
    throw(SerializerException);

//Note: deserialized data is appended to given string, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, smsc::core::buffers::FixedStringIface & out_val)
    throw(SerializerException);

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <smsc::core::buffers::FixedStringIface > (smsc::core::buffers::FixedStringIface & out_val)
    throw(SerializerException);

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_FXDSTR */

