/* ************************************************************************** *
 * Packet buffers definition.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INTERACTION_PACKET_BUFFER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INTERACTION_PACKET_BUFFER

#include <inttypes.h>

#include "core/buffers/ExtendingBuf.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

//Basic class of all packet buffers Serializer operates with.
typedef smsc::core::buffers::BufferExtension_T<uint8_t, uint32_t> PacketBufferAC;

//Buffer that stores data of grater than sizeof(void*) bytes on heap.
typedef smsc::core::buffers::ExtendingBuffer_T<uint8_t, uint32_t, sizeof(void *)> PacketHeapBuffer;

//Buffer that stores data of less than _PACKET_SZ bytes as data member.
template <uint32_t _PACKET_SZ>
class PacketBuffer_T : public smsc::core::buffers::ExtendingBuffer_T<uint8_t, uint32_t, _PACKET_SZ> {
public:
  PacketBuffer_T() : smsc::core::buffers::ExtendingBuffer_T<uint8_t, uint32_t, _PACKET_SZ>()
  { }
  ~PacketBuffer_T()
  { }
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_INTERACTION_PACKET_BUFFER */

