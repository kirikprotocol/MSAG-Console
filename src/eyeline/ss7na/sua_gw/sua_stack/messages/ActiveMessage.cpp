#include <stdio.h>
#include "eyeline/utilx/Exception.hpp"

#include "ActiveMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t ActiveMessage::_MSG_CODE;

size_t
ActiveMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _trafficModeType.isSetValue() )
    offset = _trafficModeType.serialize(result_buf, offset);

  if ( _tidLabel.isSetValue() )
    offset = _tidLabel.serialize(result_buf, offset);

  if ( _drnLabel.isSetValue() )
    offset = _drnLabel.serialize(result_buf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);

  return offset;
}

size_t
ActiveMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_trafficModeType);
  tlvFactory.registerExpectedOptionalTlv(&_tidLabel);
  tlvFactory.registerExpectedOptionalTlv(&_drnLabel);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
ActiveMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

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

const common::TLV_TrafficModeType&
ActiveMessage::getTrafficModeType() const
{
  if ( _trafficModeType.isSetValue() ) return _trafficModeType;
  else throw utilx::FieldNotSetException("ActiveMessage::getTrafficModeType::: trafficModeType wasn't set");
}

const TLV_TIDLabel&
ActiveMessage::getTIDLabel() const
{
  if ( _tidLabel.isSetValue() ) return _tidLabel;
  else throw utilx::FieldNotSetException("ActiveMessage::getTIDLabel::: tidLabel wasn't set");
}

const TLV_DRNLabel&
ActiveMessage::getDRNLabel() const
{
  if ( _drnLabel.isSetValue() ) return _drnLabel;
  else throw utilx::FieldNotSetException("ActiveMessage::getDRNLabel::: drnLabel wasn't set");
}

const common::TLV_InfoString&
ActiveMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("ActiveMessage::getInfoString::: infoString wasn't set");
}

}}}}}
