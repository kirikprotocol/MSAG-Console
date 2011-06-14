/* ************************************************************************** *
 * std::string serialization operators.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZIER_OF_STDSTR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_OF_STDSTR

#include <string>

#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * std::string serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const std::string & in_val)
    throw(SerializerException);

//Note: deserialized data is appended to given string, instead of overwriting it
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, std::string & out_val)
    throw(SerializerException);

//Note: deserialized data is appended to given string, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, std::string & out_val)
    throw(SerializerException);

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <std::string > (std::string & out_val)
    throw(SerializerException);

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER_OF_STDSTR */

