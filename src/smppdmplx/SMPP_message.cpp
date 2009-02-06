#include "SMPP_message.hpp"

namespace smpp_dmplx {

uint32_t
SMPP_message::getCommandLength() const
{
  return _commandLength;
}

void
SMPP_message::setCommandLength(uint32_t arg)
{
  _commandLength = arg;
}

uint32_t
SMPP_message::getCommandId() const
{
  return _commandId;
}

void
SMPP_message::setCommandId(uint32_t arg)
{
  _commandId = arg;
}

uint32_t
SMPP_message::getCommandStatus() const
{
  return _commandStatus;
}

void
SMPP_message::setCommandStatus(uint32_t arg)
{
  _commandStatus=arg;
}

uint32_t
SMPP_message::getSequenceNumber() const
{
  return _sequenceNumber;
}

void
SMPP_message::setSequenceNumber(uint32_t arg)
{
  _sequenceNumber = arg;
}

std::auto_ptr<BufferedOutputStream>
SMPP_message::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(new BufferedOutputStream(SMPP_HEADER_SZ));

  buf->writeUInt32(getCommandLength());
  buf->writeUInt32(SMPP_message::getCommandId());
  buf->writeUInt32(getCommandStatus());
  buf->writeUInt32(getSequenceNumber());

  return buf;
}

void
SMPP_message::unmarshal(BufferedInputStream& buf)
{
  setCommandLength(buf.size() + sizeof(uint32_t));
  setCommandStatus(buf.readUInt32());
  setSequenceNumber(buf.readUInt32());
}

std::string
SMPP_message::toString() const
{
  char message_dump[128];

  snprintf(message_dump, sizeof(message_dump), "commandLength=0x%08X,commandId=0x%08X,commandStatus=0x%08X,sequenceNumber=0x%08X",_commandLength, _commandId, _commandStatus, _sequenceNumber);

  return std::string(message_dump);
}

}
