#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "InactiveMessage.hpp"

namespace sua_messages {

const uint32_t InactiveMessage::_MSG_CODE;

InactiveMessage::InactiveMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
InactiveMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  return offset;
}

size_t
InactiveMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
InactiveMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
InactiveMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
InactiveMessage::getMsgCodeTextDescription() const
{
  return "ASP INACTIVE";
}

void
InactiveMessage::setRoutingContext(const TLV_RoutingContext& routingContext)
{
  _routingContext = routingContext;
}

const TLV_RoutingContext&
InactiveMessage::getRoutingContext() const
{
  if ( _routingContext.isSetValue() ) return _routingContext;
  else throw utilx::FieldNotSetException("InactiveMessage::getRoutingContext::: routingContext wasn't set");
}

void
InactiveMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
InactiveMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("InactiveMessage::getInfoString::: infoString wasn't set");
}

}
