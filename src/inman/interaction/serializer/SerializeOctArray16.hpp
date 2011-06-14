/* ************************************************************************** *
 * Definition of serializer operators ('<<' and'>>') for 
 * LWArrayExtension_T<uint8_t, uint16_t> type.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_SERIALIZER_STDVECTOR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_SERIALIZER_STDVECTOR

#include "core/buffers/OctArrayT.hpp"
#include "inman/interaction/serializer/SerializerDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::core::buffers::OctArray16Iface;

/* -------------------------------------------------------------------------- *
 * std::vector serialization
 * -------------------------------------------------------------------------- */
PacketBufferAC &
  operator<<(PacketBufferAC & out_buf, const OctArray16Iface & in_val)
    throw(SerializerException);

//Note: deserialized data is appended to given vector, instead of overwriting it
PacketBufferAC &
  operator>>(PacketBufferAC & in_buf, OctArray16Iface & out_val)
    throw(SerializerException);

//Note: deserialized data is appended to given vector, instead of overwriting it
//Returns number of bytes processed in PacketBufferAC
PacketBufferAC::size_type
  operator>>(const PacketBufferAC & in_buf, OctArray16Iface & out_val)
    throw(SerializerException);

template <> PacketBufferCursor &
  PacketBufferCursor::operator>> <OctArray16Iface > (OctArray16Iface & out_val)
    throw(SerializerException);

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_SERIALIZER_STDVECTOR */

