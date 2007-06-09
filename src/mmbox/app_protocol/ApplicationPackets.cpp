#include "ApplicationPackets.hpp"
#include <logger/Logger.h>
#include <sstream>
//#include <util/BinDump.hpp>

namespace mmbox {
namespace app_protocol {

RequestApplicationPacket::packet_code_t
RequestApplicationPacket::extractPacketCode(const std::vector<uint8_t>& objectBuffer)
{
  smsc::util::SerializationBuffer serializerBuf(objectBuffer.size());
  serializerBuf.assign(&objectBuffer[0], objectBuffer.size());
  return serializerBuf.ReadNetInt16();
}

void
RequestApplicationPacket::deserialize(const std::vector<uint8_t>& objectBuffer)
{
  _deserializerBuf.assign(&objectBuffer[0], objectBuffer.size());
  packet_code_t skipped_packet_code  = _deserializerBuf.ReadNetInt16();
  if ( skipped_packet_code != getPacketCode() )
    throw smsc::util::CustomException("RequestApplicationPacket::deserialize::: can't parse input buffer - got packet with code=%d, waited packet with code=%d",skipped_packet_code, getPacketCode());
}

ResponseApplicationPacket::ResponseApplicationPacket(packet_code_t req_pck_code, result_status_t status)
  : _packet_code(req_pck_code|RES_APP_PACKET_CODE_MASK), _status(status) {}

void
ResponseApplicationPacket::serialize(std::vector<uint8_t>& objectBuffer) const
{
  smsc::util::SerializationBuffer outputBuf;
  outputBuf.WriteNetInt16(_packet_code);
  outputBuf.WriteNetInt32(_status);
  objectBuffer.assign((uint8_t*)outputBuf.getBuffer(), (uint8_t*)outputBuf.getBuffer() + outputBuf.getBufferSize());
}

std::string
ResponseApplicationPacket::toString() const
{
  std::ostringstream obuf;
  obuf << "status=[" << _status
       << "]";
  return obuf.str();
}

}}
