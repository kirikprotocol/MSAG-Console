#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "NotifyMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t NotifyMessage::_MSG_CODE;

size_t
NotifyMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _status.isSetValue() )
    offset = _status.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("NotifyMessage::serialize::: status is a mandatory field and wasn't set");

  if ( _aspIdentifier.isSetValue() )
    offset = _aspIdentifier.serialize(result_buf, offset);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  
  return offset;
}

size_t
NotifyMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_status);
  tlvFactory.registerExpectedOptionalTlv(&_aspIdentifier);
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
NotifyMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _status.isSetValue() )
    result += "," + _status.toString();

  if ( _aspIdentifier.isSetValue() )
    result += "," + _aspIdentifier.toString();

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
NotifyMessage::getLength() const
{
  uint32_t length=0;

  if ( _status.isSetValue() )
    length=_status.getLength();
  else
    throw smsc::util::Exception("NotifyMessage::getLength::: status is a mandatory field and wasn't set");

  if ( _aspIdentifier.isSetValue() )
    length += _aspIdentifier.getLength();

  if ( _routingContext.isSetValue() )
    length += _routingContext.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_ApplicationStatus&
NotifyMessage::getStatus() const
{
  if ( _status.isSetValue() ) return _status;
  else throw utilx::FieldNotSetException("NotifyMessage::getStatus::: status wasn't set");
}

const common::TLV_AspIdentifier&
NotifyMessage::getAspIdentifier() const
{
  if ( _aspIdentifier.isSetValue() ) return _aspIdentifier;
  else throw utilx::FieldNotSetException("NotifyMessage::getAspIdentifier::: aspIdentifier wasn't set");
}

const common::TLV_InfoString&
NotifyMessage::getInfoString() const
{
  if ( _infoString.isSetValue()) return _infoString;
  else throw utilx::FieldNotSetException("NotifyMessage::getInfoString::: infoString wasn't set");
}

}}}}}
