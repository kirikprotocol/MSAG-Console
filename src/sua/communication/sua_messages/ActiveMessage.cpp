#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "ActiveMessage.hpp"

namespace sua_messages {

const uint32_t ActiveMessage::_MSG_CODE;

ActiveMessage::ActiveMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
ActiveMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _trafficModeType.isSetValue() )
    offset = _trafficModeType.serialize(resultBuf, offset);

  if ( _tidLabel.isSetValue() )
    offset = _tidLabel.serialize(resultBuf, offset);

  if ( _drnLabel.isSetValue() )
    offset = _drnLabel.serialize(resultBuf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);

  return offset;
}

size_t
ActiveMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_trafficModeType);
  tlvFactory.registerExpectedOptionalTlv(&_tidLabel);
  tlvFactory.registerExpectedOptionalTlv(&_drnLabel);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
ActiveMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _trafficModeType.isSetValue() )
    result += "," + _trafficModeType.toString();

  if ( _tidLabel.isSetValue() )
    result += "," + _tidLabel.toString();

  if ( _drnLabel.isSetValue() )
    result += "," + _drnLabel.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
ActiveMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _trafficModeType.isSetValue() )
    length += _trafficModeType.getLength();

  if ( _tidLabel.isSetValue() )
    length += _tidLabel.getLength();

  if ( _drnLabel.isSetValue() )
    length += _drnLabel.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
ActiveMessage::getMsgCodeTextDescription() const
{
  return "ASP ACTIVE";
}

void
ActiveMessage::setTrafficModyType(const TLV_TrafficModeType& trafficModeType)
{
  _trafficModeType = trafficModeType;
}

const TLV_TrafficModeType&
ActiveMessage::getTrafficModeType() const
{
  if ( _trafficModeType.isSetValue() ) return _trafficModeType;
  else throw utilx::FieldNotSetException("ActiveMessage::getTrafficModeType::: trafficModeType wasn't set");
}

void
ActiveMessage::setTIDLabel(const TLV_TIDLabel& tidLabel)
{
  _tidLabel = tidLabel;
}

const TLV_TIDLabel&
ActiveMessage::getTIDLabel() const
{
  if ( _tidLabel.isSetValue() ) return _tidLabel;
  else throw utilx::FieldNotSetException("ActiveMessage::getTIDLabel::: tidLabel wasn't set");
}

void
ActiveMessage::setDRNLabel(const TLV_DRNLabel& drnLabel)
{
  _drnLabel = drnLabel;
}

const TLV_DRNLabel&
ActiveMessage::getDRNLabel() const
{
  if ( _drnLabel.isSetValue() ) return _drnLabel;
  else throw utilx::FieldNotSetException("ActiveMessage::getDRNLabel::: drnLabel wasn't set");
}

void
ActiveMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
ActiveMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("ActiveMessage::getInfoString::: infoString wasn't set");
}

}
