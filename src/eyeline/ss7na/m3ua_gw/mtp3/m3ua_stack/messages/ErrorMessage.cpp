#include <stdio.h>
#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "ErrorMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

const uint32_t ErrorMessage::_MSG_CODE;

size_t
ErrorMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _errorCode.isSetValue() )
    offset = _errorCode.serialize(result_buf, offset);
  else
    throw smsc::util::Exception("ErrorMessage::serialize::: errorCode is a mandatory field and wasn't set");

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(result_buf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(result_buf, offset);

  if ( _networkAppearance.isSetValue() )
    offset = _networkAppearance.serialize(result_buf, offset);

  if ( _diagnosticInfo.isSetValue() )
    offset = _diagnosticInfo.serialize(result_buf, offset);
  
  return offset;
}

size_t
ErrorMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  M3uaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_errorCode);

  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_networkAppearance);
  tlvFactory.registerExpectedOptionalTlv(&_diagnosticInfo);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
ErrorMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _errorCode.isSetValue() )
    result += "," + _errorCode.toString();

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _networkAppearance.isSetValue() )
    result += "," + _networkAppearance.toString();

  if ( _diagnosticInfo.isSetValue() )
    result += "," + _diagnosticInfo.toString();

  return result;
}

uint32_t
ErrorMessage::getLength() const
{
  uint32_t length=0;

  if ( _errorCode.isSetValue() )
    length = _errorCode.getLength();
  else
    throw smsc::util::Exception("ErrorMessage::getLength::: errorCode is a mandatory field and wasn't set");

  if ( _routingContext.isSetValue() )
    length += _routingContext.getLength();

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _networkAppearance.isSetValue() )
    length += _networkAppearance.getLength();

  if ( _diagnosticInfo.isSetValue() )
    length += _diagnosticInfo.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_ErrorCode&
ErrorMessage::getErrorCode() const
{
  if ( _errorCode.isSetValue() ) return _errorCode;
  else throw utilx::FieldNotSetException("ErrorMessage::getErrorCode::: errorCode wasn't set");
}

const common::TLV_AffectedPointCode&
ErrorMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("ErrorMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

const TLV_NetworkAppearance&
ErrorMessage::getNetworkAppearance() const
{
  if ( _networkAppearance.isSetValue() ) return _networkAppearance;
  else throw utilx::FieldNotSetException("ErrorMessage::getNetworkAppearance::: networkAppearance wasn't set");
}

const common::TLV_DiagnosticInformation&
ErrorMessage::getDiagnosticInformation() const
{
  if ( _diagnosticInfo.isSetValue() ) return _diagnosticInfo;
  else throw utilx::FieldNotSetException("ErrorMessage::getDiagnosticInformation::: diagnosticInformation wasn't set");
}

}}}}}}
