#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "DUPUMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t DUPUMessage::_MSG_CODE;

size_t
DUPUMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _user_cause.isSetValue() )
    offset = _user_cause.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("DUPUMessage::serialize::: user_cause is a mandatary field and wasn't set");

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(result_buf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  
  return offset;
}

size_t
DUPUMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedMandatoryTlv(&_user_cause);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
DUPUMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _user_cause.isSetValue() )
    result += "," + _user_cause.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
DUPUMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _user_cause.isSetValue() )
    length += _user_cause.getLength();
  else
    throw smsc::util::Exception("DUPUMessage::getLength::: user_cause is a mandatary field and wasn't set");

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_AffectedPointCode&
DUPUMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DUPUMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const TLV_UserCause&
DUPUMessage::getUserCause() const
{
  if ( _user_cause.isSetValue() ) return _user_cause;
  else throw utilx::FieldNotSetException("DUPUMessage::getUserCause::: user_cause wasn't set");
}

const common::TLV_InfoString&
DUPUMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DUPUMessage::getInfoString::: infoString wasn't set");
}

}}}}}
