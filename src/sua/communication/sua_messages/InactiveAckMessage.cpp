#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>
#include <sua/sua_layer/io_dispatcher/Connection.hpp>

#include "InactiveAckMessage.hpp"

namespace sua_messages {

const uint32_t InactiveAckMessage::_MSG_CODE;

InactiveAckMessage::InactiveAckMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
InactiveAckMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);

  return offset;
}

size_t
InactiveAckMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
InactiveAckMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
InactiveAckMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
InactiveAckMessage::getMsgCodeTextDescription() const
{
  return "ASP INACTIVE ACK";
}

void
InactiveAckMessage::setRoutingContext(const TLV_RoutingContext& routingContext)
{
  _routingContext = routingContext;
}

const TLV_RoutingContext&
InactiveAckMessage::getRoutingContext() const
{
  if ( _routingContext.isSetValue() ) return _routingContext;
  else throw utilx::FieldNotSetException("InactiveAckMessage::getRoutingContext::: routingContext wasn't set");
}

void
InactiveAckMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
InactiveAckMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("InactiveAckMessage::getInfoString::: infoString wasn't set");
}

}
