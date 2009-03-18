#include <stdio.h>
#include <util/Exception.hpp>
#include <eyeline/utilx/Exception.hpp>

#include "ErrorMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t ErrorMessage::_MSG_CODE;

ErrorMessage::ErrorMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
ErrorMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _errorCode.isSetValue() )
    offset = _errorCode.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("ErrorMessage::serialize::: errorCode is a mandatory field and wasn't set");

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(resultBuf, offset);

  if ( _networkAppearance.isSetValue() )
    offset = _networkAppearance.serialize(resultBuf, offset);

  if ( _diagnosticInfo.isSetValue() )
    offset = _diagnosticInfo.serialize(resultBuf, offset);
  
  return offset;
}

size_t
ErrorMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_errorCode);
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_networkAppearance);
  tlvFactory.registerExpectedOptionalTlv(&_diagnosticInfo);

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
ErrorMessage::toString() const
{
  std::string result(SUAMessage::toString());

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

const char*
ErrorMessage::getMsgCodeTextDescription() const
{
  return "ERROR";
}

void
ErrorMessage::setErrorCode(const TLV_ErrorCode& errorCode)
{
  _errorCode = errorCode;
}

const TLV_ErrorCode&
ErrorMessage::getErrorCode() const
{
  if ( _errorCode.isSetValue() ) return _errorCode;
  else throw utilx::FieldNotSetException("ErrorMessage::getErrorCode::: errorCode wasn't set");
}

void
ErrorMessage::setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode)
{
  _affectedPointCodes = affectedPointCode;
}

const TLV_AffectedPointCode&
ErrorMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("ErrorMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

void
ErrorMessage::setNetworkAppearance(const TLV_NetworkAppearance& networkAppearance)
{
  _networkAppearance = networkAppearance;
}

const TLV_NetworkAppearance&
ErrorMessage::getNetworkAppearance() const
{
  if ( _networkAppearance.isSetValue() ) return _networkAppearance;
  else throw utilx::FieldNotSetException("ErrorMessage::getNetworkAppearance::: networkAppearance wasn't set");
}

void
ErrorMessage::setDiagnosticInformation(const TLV_DiagnosticInformation& diagnosticInformation)
{
  _diagnosticInfo = diagnosticInformation;
}

const TLV_DiagnosticInformation&
ErrorMessage::getDiagnosticInformation() const
{
  if ( _diagnosticInfo.isSetValue() ) return _diagnosticInfo;
  else throw utilx::FieldNotSetException("ErrorMessage::getDiagnosticInformation::: diagnosticInformation wasn't set");
}

}}}}
