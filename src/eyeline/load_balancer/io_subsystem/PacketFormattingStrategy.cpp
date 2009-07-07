#include "PacketFormattingStrategy.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

Packet*
PacketFormattingStrategy::receive(corex::io::InputStream* iStream)
{
  if ( !hasBeenPacketHeaderReadCompletely() ) {
    readNextPartOfPacketHeader(iStream);
    if ( hasBeenTotalPacketReadCompletely() )
      return getCompletePacket();
    return NULL;
  }

  if ( !hasBeenPacketBodyReadCompletely() ) {
    readNextPartOfPacketBody(iStream);
    if ( hasBeenTotalPacketReadCompletely() )
      return getCompletePacket();
    return NULL;
  }

  if ( !hasBeenPacketTailerReadCompletely() ) {
    readNextPartOfPacketTailer(iStream);
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
