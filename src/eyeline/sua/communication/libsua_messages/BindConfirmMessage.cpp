#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <stdio.h>
#include "eyeline/sua/communication/libsua_messages/BindConfirmMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace libsua_messages {

const uint32_t BindConfirmMessage::_MSG_CODE;

BindConfirmMessage::BindConfirmMessage()
  : LibsuaMessage(_MSG_CODE), _status(BIND_OK)
{}

BindConfirmMessage::BindConfirmMessage(status_t status)
  : LibsuaMessage(_MSG_CODE), _status(status)
{}

size_t
BindConfirmMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  return communication::addField(resultBuf, offset, _status);
}

size_t
BindConfirmMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  return offset = extractField(packetBuf, offset, &_status);
}

std::string
BindConfirmMessage::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), ",status=[%d]", _status);
  return LibsuaMessage::toString() + strBuf;
}

const char*
BindConfirmMessage::getMsgCodeTextDescription() const
{
  return "BIND_CONFIRM";
}

uint32_t
BindConfirmMessage::getLength() const
{
  return LibsuaMessage::getLength() + static_cast<uint32_t>(sizeof(_status));
}

}}}}
