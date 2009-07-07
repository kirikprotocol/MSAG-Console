#include <stdio.h>
#include "eyeline/utilx/Exception.hpp"
#include "SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

SMPPMessage::SMPPMessage(uint32_t commandId)
  : _commandId(commandId), _isSetCommandLength(false),
    _isSetCommandStatus(false), _isSetSequenceNumber(false)
{}

io_subsystem::Message::message_code_t
SMPPMessage::getMsgCode() const
{
  return _commandId;
}

size_t
SMPPMessage::serialize(io_subsystem::Packet* packet) const
{
  packet->addValue(getCommandLength());
  packet->addValue(getMsgCode());
  packet->addValue(getCommandStatus());
  return packet->addValue(getSequenceNumber());
}

size_t
SMPPMessage::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = packet->extractValue(&_commandLength, 0);
  if ( _commandLength < SMPP_HEADER_SZ )
    throw utilx::DeserializationException("SMPPMessage::deserialize::: message is too short - message length = %d", _commandLength);
  _isSetCommandLength = true;

  uint32_t commandId;
  offset = packet->extractValue(&commandId, offset);
  if ( commandId != _commandId )
    throw utilx::DeserializationException("SMPPMessage::deserialize::: got unexpected commandId value=0x%08X, expected commandId value=0x%08X", commandId, _commandId);

  offset = packet->extractValue(&_commandStatus, offset);
  _isSetCommandStatus = true;

  offset = packet->extractValue(&_sequenceNumber, offset);
  _isSetSequenceNumber = true;

  return offset;
}

std::string
SMPPMessage::toString() const
{
  char message_dump[128];

  int offset = snprintf(message_dump, sizeof(message_dump), "commandLength=%d,commandId=0x%08X", getCommandLength(), _commandId);

  if ( _isSetCommandStatus )
    offset += snprintf(message_dump + offset, sizeof(message_dump) - offset, ",commandStatus=0x%08X", _commandStatus);
  if ( _isSetSequenceNumber )
    offset += snprintf(message_dump + offset, sizeof(message_dump) - offset, ",sequenceNumber=%d", _sequenceNumber);
  return std::string(message_dump);
}

uint32_t
SMPPMessage::getCommandLength() const
{
  if ( _isSetCommandLength )
    return _commandLength;
  return
    static_cast<uint32_t>(sizeof(_commandLength) + sizeof(_commandId)+
                          sizeof(_commandStatus) + sizeof(_sequenceNumber) + calculateCommandBodyLength());
}

uint32_t
SMPPMessage::getCommandStatus() const
{
  if ( !_isSetCommandStatus )
    throw utilx::FieldNotSetException("SMPPMessage::getCommandStatus::: field is not set");

  return _commandStatus;
}

void
SMPPMessage::setCommandStatus(uint32_t commandStatus)
{
  _commandStatus = commandStatus;
  _isSetCommandStatus = true;
}

uint32_t
SMPPMessage::getSequenceNumber() const
{
  if ( !_isSetSequenceNumber )
    throw utilx::FieldNotSetException("SMPPMessage::getSequenceNumber::: field is not set");
  return _sequenceNumber;
}

void
SMPPMessage::setSequenceNumber(uint32_t sequenceNumber)
{
  _sequenceNumber = sequenceNumber;
  _isSetSequenceNumber = true;
}

}}}}
