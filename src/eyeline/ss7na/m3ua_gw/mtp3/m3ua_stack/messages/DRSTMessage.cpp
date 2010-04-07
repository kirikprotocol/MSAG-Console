#include <stdio.h>
#include "eyeline/utilx/Exception.hpp"

#include "DRSTMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

const uint32_t DRSTMessage::_MSG_CODE;

size_t
DRSTMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _networkAppearance.isSetValue() )
    offset = _networkAppearance.serialize(result_buf, offset);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCode.isSetValue() )
    offset = _affectedPointCode.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("DRSTMessage::serialize::: affectedPointCode is a mandatory field and wasn't set");

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);

  return offset;
}

size_t
DRSTMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  M3uaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_affectedPointCode);

  tlvFactory.registerExpectedOptionalTlv(&_networkAppearance);
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
DRSTMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _networkAppearance.isSetValue() )
    result += "," + _networkAppearance.toString();

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCode.isSetValue() )
    result += "," + _affectedPointCode.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
DRSTMessage::getLength() const
{
  uint32_t length=0;

  if ( _networkAppearance.isSetValue() )
    length = _networkAppearance.getLength();

  if ( _routingContext.isSetValue() )
    length += _routingContext.getLength();

  if ( _affectedPointCode.isSetValue() )
    length += _affectedPointCode.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const TLV_NetworkAppearance&
DRSTMessage::getNetworkAppearance() const
{
  if ( _networkAppearance.isSetValue() ) return _networkAppearance;
  else throw utilx::FieldNotSetException("DRSTMessage::getNetworkAppearance::: networkAppearance wasn't set");
}

common::TLV_AffectedPointCode
DRSTMessage::getAffectedPointCode() const
{
  if ( _affectedPointCode.isSetValue() ) return _affectedPointCode;
  else throw utilx::FieldNotSetException("DRSTMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const common::TLV_InfoString&
DRSTMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DRSTMessage::getInfoString::: infoString wasn't set");
}

}}}}}}
