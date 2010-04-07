#include "eyeline/utilx/Exception.hpp"

#include "DAVAMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

const uint32_t DAVAMessage::_MSG_CODE;

size_t
DAVAMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _networkAppearance.isSetValue() )
    offset = _networkAppearance.serialize(result_buf, offset);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("DAVAMessage::serialize::: affectedPointCode is a mandatory field and wasn't set");

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  
  return offset;
}

size_t
DAVAMessage::deserialize(const common::TP& packet_buf)
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
DAVAMessage::toString() const
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
DAVAMessage::getLength() const
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
DAVAMessage::getNetworkAppearance() const
{
  if ( _networkAppearance.isSetValue() ) return _networkAppearance;
  else throw utilx::FieldNotSetException("DAVAMessage::getNetworkAppearance::: smi wasn't set");
}

const common::TLV_AffectedPointCode&
DAVAMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DAVAMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const common::TLV_InfoString&
DAVAMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DAVAMessage::getInfoString::: infoString wasn't set");
}

}}}}}}
