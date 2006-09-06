#include "SMPP_message.hpp"
#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

const uint32_t
smpp_dmplx::SMPP_message::BIND_RECEIVER = 0x00000001;

const uint32_t
smpp_dmplx::SMPP_message::BIND_RECEIVER_RESP= 0x80000001;

const uint32_t
smpp_dmplx::SMPP_message::BIND_TRANSMITTER = 0x00000002;

const uint32_t
smpp_dmplx::SMPP_message::BIND_TRANSMITTER_RESP = 0x80000002;

const uint32_t
smpp_dmplx::SMPP_message::BIND_TRANSCEIVER = 0x00000009;

const uint32_t
smpp_dmplx::SMPP_message::BIND_TRANSCEIVER_RESP = 0x80000009;

const uint32_t
smpp_dmplx::SMPP_message::UNBIND = 0x00000006;

const uint32_t
smpp_dmplx::SMPP_message::UNBIND_RESP = 0x80000006;

const uint32_t
smpp_dmplx::SMPP_message::ENQUIRE_LINK = 0x00000015;

const uint32_t
smpp_dmplx::SMPP_message::ENQUIRE_LINK_RESP = 0x80000015;

const uint32_t
smpp_dmplx::SMPP_message::SMPP_HEADER_SZ = 16;

const uint32_t
smpp_dmplx::SMPP_message::MAX_SMPP_MESSAGE_SIZE = 70*1024;

smpp_dmplx::SMPP_message::~SMPP_message()
{}

uint32_t
smpp_dmplx::SMPP_message::getCommandLength() const
{
  return _commandLength;
}

void
smpp_dmplx::SMPP_message::setCommandLength(uint32_t arg)
{
  _commandLength = arg;
}

uint32_t
smpp_dmplx::SMPP_message::getCommandId() const
{
  return _commandId;
}

void
smpp_dmplx::SMPP_message::setCommandId(uint32_t arg)
{
  _commandId = arg;
}

uint32_t
smpp_dmplx::SMPP_message::getCommandStatus() const
{
  return _commandStatus;
}

void
smpp_dmplx::SMPP_message::setCommandStatus(uint32_t arg)
{
  _commandStatus=arg;
}

uint32_t
smpp_dmplx::SMPP_message::getSequenceNumber() const
{
  return _sequenceNumber;
}

void
smpp_dmplx::SMPP_message::setSequenceNumber(uint32_t arg)
{
  _sequenceNumber = arg;
}

std::auto_ptr<smpp_dmplx::BufferedOutputStream>
smpp_dmplx::SMPP_message::marshal() const
{
  std::auto_ptr<BufferedOutputStream> buf(new BufferedOutputStream(SMPP_HEADER_SZ));

  buf->writeUInt32(getCommandLength());
  buf->writeUInt32(SMPP_message::getCommandId());
  buf->writeUInt32(getCommandStatus());
  buf->writeUInt32(getSequenceNumber());

  return buf;
}

void
smpp_dmplx::SMPP_message::unmarshal(smpp_dmplx::BufferedInputStream& buf)
{
  setCommandLength(buf.size() + sizeof(uint32_t));
  setCommandStatus(buf.readUInt32());
  setSequenceNumber(buf.readUInt32());
}

std::string
smpp_dmplx::SMPP_message::toString() const
{
  char message_dump[128];

  snprintf(message_dump, sizeof(message_dump), "commandLength=0x%08X,commandId=0x%08X,commandStatus=0x%08X,sequenceNumber=0x%08X",_commandLength, _commandId, _commandStatus, _sequenceNumber);

  return std::string(message_dump);
}
