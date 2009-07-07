#include <stdio.h>

#include "eyeline/utilx/Exception.hpp"
#include "BindResponse.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

BindResponse::BindResponse(uint32_t commandId)
  : SMPPMessage(commandId), _isSetSystemId(false) {}

size_t
BindResponse::serialize(io_subsystem::Packet* packet) const
{
  SMPPMessage::serialize(packet);

  size_t offset = addCOctetString(packet, getSystemId(), sizeof(_systemId));
  if ( _scInterfaceVersion.isSetValue() )
    offset = _scInterfaceVersion.serialize(packet);

  return offset;
}

size_t
BindResponse::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);

  offset = extractVariableCOctetString(packet, offset, _systemId, sizeof(_systemId));
  _isSetSystemId = true;

  return _scInterfaceVersion.deserialize(packet, offset);
}

const char*
BindResponse::getSystemId() const
{
  if ( !_isSetSystemId )
    throw utilx::FieldNotSetException("BindResponse::getSystemId::: field is not set");
  return _systemId;
}

void
BindResponse::setSystemId(const char* systemId)
{
  if ( strlcpy(_systemId, systemId, sizeof(_systemId)) >= sizeof(_systemId) )
    throw smsc::util::Exception("BindResponse::setSystemId::: specified systemId value is too long - max value size is %d bytes", sizeof(_systemId) - 1);

  _isSetSystemId = true;
}

uint8_t
BindResponse::getScInterfaceVersion()
{
  if ( !_scInterfaceVersion.isSetValue() )
    throw utilx::FieldNotSetException("BindResponse::getScInterfaceVersion::: field is not set");
  return _scInterfaceVersion.getValue();
}

void
BindResponse::setScInterfaceVersion(uint8_t scInterfaceVersion)
{
  _scInterfaceVersion.setValue(scInterfaceVersion);
}

std::string
BindResponse::toString() const
{
  char message_dump[64];
  int offset = 0;
  if ( _isSetSystemId )
    offset = snprintf(message_dump, sizeof(message_dump), ",systemId='%s'", _systemId);
  if ( _scInterfaceVersion.isSetValue() )
    snprintf(message_dump + offset, sizeof(message_dump) - offset, ",scInterfaceVersion=0x%x", _scInterfaceVersion.getValue());

  return SMPPMessage::toString() + message_dump;
}

uint32_t
BindResponse::calculateCommandBodyLength() const
{
  return
    static_cast<uint32_t>((_isSetSystemId ? std::min(strlen(_systemId)+1, sizeof(_systemId)) : 0) +
                          (_scInterfaceVersion.isSetValue() ? _scInterfaceVersion.getParameterSize() : 0 ));
}

}}}}
