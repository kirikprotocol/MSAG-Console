#include "PacketFormattingStrategy.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

Packet*
PacketFormattingStrategy::receive(corex::io::InputStream* i_stream)
{
  if ( !hasBeenPacketHeaderReadCompletely() ) {
    readNextPartOfPacketHeader(i_stream);
    if ( hasBeenTotalPacketReadCompletely() )
      return getCompletePacket();
    return NULL;
  }

  if ( !hasBeenPacketBodyReadCompletely() ) {
    readNextPartOfPacketBody(i_stream);
    if ( hasBeenTotalPacketReadCompletely() )
      return getCompletePacket();
    return NULL;
  }

  if ( !hasBeenPacketTailerReadCompletely() ) {
    readNextPartOfPacketTailer(i_stream);
    if ( !hasBeenTotalPacketReadCompletely() )
      return NULL;
  }

  return getCompletePacket();
}

Packet*
PacketFormattingStrategy::prepareToSend(const Message& msg)
{
  Packet* packet = new Packet();
  msg.serialize(packet);
  packet->packet_type = msg.getMsgCode();
  return packet;
}

}}}
