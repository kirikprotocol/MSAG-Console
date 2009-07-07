#include <stdio.h>
#include <utility>

#include "eyeline/utilx/Exception.hpp"

#include "BindRequest.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

BindRequest::BindRequest(uint32_t commandId)
  : SMPPMessage(commandId), _isSetSystemId(false), _isSetPassword(false), _isSetSystemType(false),
    _isSetInterfaceVersion(false), _isSetAddrTon(false), _isSetAddrNpi(false), _isSetAddrRange(false)
{}

size_t
BindRequest::serialize(io_subsystem::Packet* packet) const
{
  SMPPMessage::serialize(packet);
  addCOctetString(packet, getSystemId(), sizeof(_systemId));
  addCOctetString(packet, getPassword(), sizeof(_password));
  addCOctetString(packet, getSystemType(), sizeof(_systemType));
  packet->addValue(getInterfaceVersion());
  packet->addValue(getAddrTon());
  packet->addValue(getAddrNpi());
  return addCOctetString(packet, getAddressRange(), sizeof(_addrRange));
}

size_t
BindRequest::deserialize(const io_subsystem::Packet* packet)
{
  size_t offset = SMPPMessage::deserialize(packet);

  offset = extractVariableCOctetString(packet, offset, _systemId, sizeof(_systemId));
  _isSetSystemId = true;

  offset = extractVariableCOctetString(packet, offset, _password, sizeof(_password));
  _isSetPassword = true;

  offset = extractVariableCOctetString(packet, offset, _systemType, sizeof(_systemType));
  _isSetSystemType = true;

  offset = packet->extractValue(&_interfaceVersion, offset);
  _isSetInterfaceVersion = true;

  offset = packet->extractValue(&_addrTon, offset);
  _isSetAddrTon = true;

  offset = packet->extractValue(&_addrNpi, offset);
  _isSetAddrNpi = true;

  offset = extractVariableCOctetString(packet, offset, _addrRange, sizeof(_addrRange));
  _isSetAddrRange = true;

  return offset;
}

uint32_t
BindRequest::calculateCommandBodyLength() const
{
  return
    static_cast<uint32_t>((_isSetSystemId ? std::min(strlen(_systemId)+1, sizeof(_systemId)) : 0) +
                          (_isSetPassword ? std::min(strlen(_password), sizeof(_password)) : 0 ) +
                          (_isSetSystemType ? std::min(strlen(_systemType), sizeof(_systemType)) : 0) +
                          (_isSetInterfaceVersion ? sizeof(_interfaceVersion) : 0) +
                          (_isSetAddrTon ? sizeof(_addrTon) : 0) +
                          (_isSetAddrNpi ? sizeof(_addrNpi) : 0 ) +
                          (_isSetAddrRange? std::min(strlen(_addrRange), sizeof(_addrRange)) : 0));
}

const char*
BindRequest::getSystemId() const
{
  if ( !_isSetSystemId )
    throw utilx::FieldNotSetException("BindRequest::getSystemId::: field is not set");
  return _systemId;
}

void
BindRequest::setSystemId(const char* systemId)
{
  if ( strlcpy(_systemId, systemId, sizeof(_systemId)) >= sizeof(_systemId) )
    throw smsc::util::Exception("BindRequest::setSystemId::: specified systemId value is too long - max value size is %d bytes", sizeof(_systemId) - 1);

  _isSetSystemId = true;
}

const char*
BindRequest::getPassword() const
{
  if ( !_isSetPassword )
    throw utilx::FieldNotSetException("BindRequest::getPassword::: field is not set");
  return _password;
}

void
BindRequest::setPassword(const char* password)
{
  if ( strlcpy(_password, password, sizeof(_password)) >= sizeof(_password) )
    throw smsc::util::Exception("BindRequest::setPassword::: specified password value is too long - max value size is %d bytes", sizeof(_password) - 1);

  _isSetPassword = true;
}

const char*
BindRequest::getSystemType() const
{
  if ( !_isSetSystemType )
    throw utilx::FieldNotSetException("BindRequest::getSystemType::: field is not set");
  return _systemType;
}

void
BindRequest::setSystemType(const char* systemType)
{
  if ( strlcpy(_systemType, systemType, sizeof(_systemType)) >= sizeof(_systemType) )
    throw smsc::util::Exception("BindRequest::setSystemType::: specified systemType value is too long - max value size is %d bytes", sizeof(_systemType) - 1);

  _isSetSystemType = true;
}

uint8_t
BindRequest::getInterfaceVersion() const
{
  if ( !_isSetInterfaceVersion )
    throw utilx::FieldNotSetException("BindRequest::getInterfaceVersion::: field is not set");
  return _interfaceVersion;
}

void
BindRequest::setInterfaceVersion(uint8_t interfaceVersion)
{
  _interfaceVersion = interfaceVersion;
  _isSetInterfaceVersion = true;
}

uint8_t
BindRequest::getAddrTon() const
{
  if ( !_isSetAddrTon )
    throw utilx::FieldNotSetException("BindRequest::getAddrTon::: field is not set");
  return _addrTon;
}

void
BindRequest::setAddrTon(uint8_t addrTon)
{
  _addrTon = addrTon;
  _isSetAddrTon = true;
}

uint8_t
BindRequest::getAddrNpi() const
{
  if ( !_isSetAddrNpi )
    throw utilx::FieldNotSetException("BindRequest::getAddrNpi::: field is not set");
  return _addrNpi;
}

void
BindRequest::setAddrNpi(uint8_t addrNpi)
{
  _addrNpi = addrNpi;
  _isSetAddrNpi = true;
}

const char*
BindRequest::getAddressRange() const
{
  if ( !_isSetAddrRange )
    throw utilx::FieldNotSetException("BindRequest::getAddressRange::: field is not set");
  return _addrRange;
}

void
BindRequest::setAddressRange(const char* addrRange)
{
  if ( strlcpy(_addrRange, addrRange, sizeof(_addrRange)) >= sizeof(_addrRange) )
    throw smsc::util::Exception("BindRequest::setAddressRange::: specified addrRange value is too long - max value size is %d bytes", sizeof(_addrRange) - 1);

  _isSetAddrRange = true;
}

std::string
BindRequest::toString() const
{
  char message_dump[256];
  int offset = 0;
  if ( _isSetSystemId )
    offset = snprintf(message_dump, sizeof(message_dump), ",systemId='%s'", _systemId);
  if ( _isSetPassword )
    offset = snprintf(message_dump + offset, sizeof(message_dump) - offset, ",password='%s'", _password);
  if ( _isSetSystemType )
    offset = snprintf(message_dump + offset, sizeof(message_dump) - offset, ",systemType='%s'", _systemType);
  if ( _isSetInterfaceVersion )
    offset = snprintf(message_dump + offset, sizeof(message_dump) - offset, ",interfaceVersion=%d", _interfaceVersion);
  if ( _isSetAddrTon )
    offset = snprintf(message_dump + offset, sizeof(message_dump) - offset, ",addrTon=%d", _addrTon);
  if ( _isSetAddrNpi )
    offset = snprintf(message_dump + offset, sizeof(message_dump) - offset, ",addrNpi=%d", _addrNpi);
  if ( _isSetAddrRange )
    snprintf(message_dump + offset, sizeof(message_dump) - offset, ",addrRange='%s'", _addrRange);

  return SMPPMessage::toString() + message_dump;
}

}}}}
