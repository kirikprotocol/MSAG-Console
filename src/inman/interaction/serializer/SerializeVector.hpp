/* ************************************************************************** *
 * Definition of serializer operators ('<<' and'>>') for std::vector type.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER_STDVECTOR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_STDVECTOR

#include <vector>

#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {
/* -------------------------------------------------------------------------- *
 * std::vector serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const std::vector<unsigned char> & in_val)
    throw(SerializerException);

//Note: deserialized data is appended to given vector, instead of overwriting it
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, std::vector<unsigned char> & out_val)
    throw(SerializerException);

//Note: deserialized data is appended to given vector, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, std::vector<unsigned char> & out_val)
    throw(SerializerException);

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <std::vector<unsigned char> > (std::vector<unsigned char> & out_val)
    throw(SerializerException);

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER_STDVECTOR */

