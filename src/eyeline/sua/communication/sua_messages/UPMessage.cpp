#include <stdio.h>
#include <eyeline/utilx/Exception.hpp>

#include "UPMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t UPMessage::_MSG_CODE;

UPMessage::UPMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
UPMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _aspId.isSetValue() )
    offset = _aspId.serialize(resultBuf, offset);
  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);

  return offset;
}

size_t
UPMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_aspId);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
UPMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _aspId.isSetValue() )
    result += "," + _aspId.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
UPMessage::getLength() const
{
  uint32_t length=0;
  if ( _aspId.isSetValue() ) length = _aspId.getLength();
  if ( _infoString.isSetValue() ) length += _infoString.getLength();
  return HEADER_SIZE + length;
}

const char*
UPMessage::getMsgCodeTextDescription() const
{
  return "ASP UP";
}

void
UPMessage::setAspIdentifier(const TLV_AspIdentifier& aspId)
{
  _aspId = aspId;
}

const TLV_AspIdentifier&
UPMessage::getAspIdentifier() const
{
  if ( _aspId.isSetValue() ) return _aspId;
  else throw utilx::FieldNotSetException("UPMessage::getAspIdentifier::: aspId wasn't set");
}

void
UPMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
UPMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("UPMessage::getInfoString::: infoString wasn't set");
}

}}}}
