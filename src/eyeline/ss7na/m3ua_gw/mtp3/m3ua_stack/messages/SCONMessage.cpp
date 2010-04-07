#include "eyeline/utilx/Exception.hpp"

#include "SCONMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

const uint32_t SCONMessage::_MSG_CODE;

size_t
SCONMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _networkAppearance.isSetValue() )
    offset = _networkAppearance.serialize(result_buf, offset);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("SCONMessage::serialize::: affectedPointCode is a mandatory field and wasn't set");

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  
  return offset;
}

size_t
SCONMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  M3uaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_affectedPointCodes);

  tlvFactory.registerExpectedOptionalTlv(&_networkAppearance);
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
SCONMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _networkAppearance.isSetValue() )
    result += "," + _networkAppearance.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
SCONMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _networkAppearance.isSetValue() )
    length += _networkAppearance.getLength();

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const TLV_NetworkAppearance&
SCONMessage::getNetworkAppearance() const
{
  if ( _networkAppearance.isSetValue() ) return _networkAppearance;
  else throw utilx::FieldNotSetException("SCONMessage::getNetworkAppearance::: smi wasn't set");
}

const common::TLV_AffectedPointCode&
SCONMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("SCONMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const TLV_ConcernedDestination&
SCONMessage::getConcernedDestination() const
{
  if ( _concernedDestination.isSetValue() ) return _concernedDestination;
  else throw utilx::FieldNotSetException("SCONMessage::getConcernedDestination::: concernedDestination wasn't set");
}

const TLV_CongestionIndication&
SCONMessage::getCongestionIndication() const
{
  if ( _congestionIndication.isSetValue() ) return _congestionIndication;
  else throw utilx::FieldNotSetException("SCONMessage::getCongestionIndication::: congestionIndication  wasn't set");
}

const common::TLV_InfoString&
SCONMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("SCONMessage::getInfoString::: infoString wasn't set");
}

}}}}}}
