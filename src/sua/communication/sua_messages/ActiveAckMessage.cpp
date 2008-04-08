#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "ActiveAckMessage.hpp"

namespace sua_messages {

const uint32_t ActiveAckMessage::_MSG_CODE;

ActiveAckMessage::ActiveAckMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
ActiveAckMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  return offset;
}

size_t
ActiveAckMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_trafficModeType);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);

  return offset;
}

std::string
ActiveAckMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _trafficModeType.isSetValue() )
    result += "," + _trafficModeType.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
ActiveAckMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _trafficModeType.isSetValue() )
    length += _trafficModeType.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
ActiveAckMessage::getMsgCodeTextDescription() const
{
  return "ASP ACTIVE ACK";
}

void
ActiveAckMessage::setRoutingContext(const TLV_RoutingContext& routingContext)
{
  _routingContext = routingContext;
}

const TLV_RoutingContext&
ActiveAckMessage::getRoutingContext() const
{
  if ( _routingContext.isSetValue() ) return _routingContext;
  else throw utilx::FieldNotSetException("ActiveAckMessage::getRoutingContext::: routingContext wasn't set");
}

void
ActiveAckMessage::setTrafficModyType(const TLV_TrafficModeType& trafficModeType)
{
  _trafficModeType = trafficModeType;
}

const TLV_TrafficModeType&
ActiveAckMessage::getTrafficModeType() const
{
  if ( _trafficModeType.isSetValue() ) return _trafficModeType;
  else throw utilx::FieldNotSetException("ActiveAckMessage::getTrafficModeType::: trafficModeType wasn't set");
}

void
ActiveAckMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
ActiveAckMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("ActiveAckMessage::getInfoString::: infoString wasn't set");
}

}
