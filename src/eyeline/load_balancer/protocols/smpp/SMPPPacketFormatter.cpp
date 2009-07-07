#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "eyeline/utilx/Exception.hpp"

#include "types.hpp"
#include "SMPPMessage.hpp"
#include "SMPPPacketFormatter.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

SMPPPacketFormatter::SMPPPacketFormatter()
  : _packet(NULL), _offset(0), _pduSize(0),
    _logger(smsc::logger::Logger::getInstance("smpp"))
{}

SMPPPacketFormatter::~SMPPPacketFormatter()
{
  delete _packet;
}

void
SMPPPacketFormatter::readNextPartOfPacketHeader(corex::io::InputStream* iStream)
{
  if ( !_packet )
    _packet = new io_subsystem::Packet();

  size_t oldOffset = _offset;
  _offset += iStream->read(_packet->packet_data + _offset, SMPPMessage::SMPP_HEADER_SZ - _offset);
  smsc_log_debug(_logger, "SMPPPacketFormatter::readNextPartOfPacketHeader::: %d bytes has been read",
                 _offset - oldOffset);

  if ( hasBeenPacketHeaderReadCompletely() ) {
    union {
      uint8_t tmpBuf[sizeof(uint32_t)];
      uint32_t value;
    } alligned_uint32_buf;
    memcpy(alligned_uint32_buf.tmpBuf, _packet->packet_data, sizeof(uint32_t));
    _pduSize = ntohl(alligned_uint32_buf.value);

    if ( _pduSize < SMPPMessage::SMPP_HEADER_SZ )
      throw utilx::DeserializationException("SMPPPacketFormatter::readNextPartOfPacketHeader::: invalid command_length field value of SMPP PDU: value [%d octets] is less than 16 octets", _pduSize);

    memcpy(reinterpret_cast<uint8_t*>(&_packet->packet_type), _packet->packet_data + sizeof(uint32_t), sizeof(uint32_t));
    _packet->packet_type = ntohl(_packet->packet_type);
    _packet->protocol_family = SMPP_PROTOCOL;
    smsc_log_debug(_logger, "SMPPPacketFormatter::readNextPartOfPacketHeader::: got complete packet header: packet_type=0x%08X, pduSize=%d",
                   _packet->packet_type, _pduSize);
  }

  if ( hasBeenTotalPacketReadCompletely() ) {
    _packet->packet_data_len = _pduSize;

    _offset = 0; _pduSize = 0;
  }
}

void
SMPPPacketFormatter::readNextPartOfPacketBody(corex::io::InputStream* iStream)
{
  size_t oldOffset = _offset;
  _offset += iStream->read(_packet->packet_data + _offset, _pduSize - _offset);
  smsc_log_debug(_logger, "SMPPPacketFormatter::readNextPartOfPacketBody::: %d bytes has been read",
                 _offset - oldOffset);

  if ( hasBeenTotalPacketReadCompletely() ) {
    _packet->packet_data_len = _pduSize;

    _offset = 0; _pduSize = 0;
  }
}

void
SMPPPacketFormatter::readNextPartOfPacketTailer(corex::io::InputStream* iStream)
{}

bool
SMPPPacketFormatter::hasBeenPacketHeaderReadCompletely() const
{
  if ( _offset < SMPPMessage::SMPP_HEADER_SZ ) return false;
  return true;
}

bool
SMPPPacketFormatter::hasBeenPacketBodyReadCompletely() const
{
  smsc_log_debug(_logger, "SMPPPacketFormatter::hasBeenPacketBodyReadCompletely::: offset=%d, pduSize=%d",
                 _offset, _pduSize);

  if ( _offset < _pduSize ) return false;
  return true;
}

bool
SMPPPacketFormatter::hasBeenPacketTailerReadCompletely() const
{
  return true;
}

bool
SMPPPacketFormatter::hasBeenTotalPacketReadCompletely() const
{
  if ( _offset == _pduSize )
    return true;
  else
    return false;
}

io_subsystem::Packet*
SMPPPacketFormatter::getCompletePacket()
{
  io_subsystem::Packet* packet = _packet;

  _packet = NULL;
  smsc_log_debug(_logger, "SMPPPacketFormatter::getCompletePacket::: return new packet: packet_data_len=%d, packet_type=0x%08X",
                 packet->packet_data_len, packet->packet_type);

  return packet;
}

}}}}
