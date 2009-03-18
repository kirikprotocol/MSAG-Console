#include <util/Exception.hpp>
#include <eyeline/utilx/Exception.hpp>

#include "NotifyMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t NotifyMessage::_MSG_CODE;

NotifyMessage::NotifyMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
NotifyMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _status.isSetValue() )
    offset = _status.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("NotifyMessage::serialize::: status is a mandatory field and wasn't set");

  if ( _aspIdentifier.isSetValue() )
    offset = _aspIdentifier.serialize(resultBuf, offset);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _diagnosticInfo.isSetValue() )
    offset = _diagnosticInfo.serialize(resultBuf, offset);
  
  return offset;
}

size_t
NotifyMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedMandatoryTlv(&_status);
  tlvFactory.registerExpectedOptionalTlv(&_aspIdentifier);
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_diagnosticInfo);

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
NotifyMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _status.isSetValue() )
    result += "," + _status.toString();

  if ( _aspIdentifier.isSetValue() )
    result += "," + _aspIdentifier.toString();

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _diagnosticInfo.isSetValue() )
    result += "," + _diagnosticInfo.toString();

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

  if ( _diagnosticInfo.isSetValue() )
    length += _diagnosticInfo.getLength();

  return HEADER_SIZE + length;
}

const char*
NotifyMessage::getMsgCodeTextDescription() const
{
  return "NOTIFY";
}

void
NotifyMessage::setStatus(const TLV_ApplicationStatus& status)
{
  _status = status;
}

const TLV_ApplicationStatus&
NotifyMessage::getStatus() const
{
  if ( _status.isSetValue() ) return _status;
  else throw utilx::FieldNotSetException("NotifyMessage::getStatus::: status wasn't set");
}

void
NotifyMessage::setAspIdentifier(TLV_AspIdentifier aspIdentifier)
{
  _aspIdentifier = aspIdentifier;
}

const TLV_AspIdentifier&
NotifyMessage::getAspIdentifier() const
{
  if ( _aspIdentifier.isSetValue() ) return _aspIdentifier;
  else throw utilx::FieldNotSetException("NotifyMessage::getAspIdentifier::: aspIdentifier wasn't set");
}

void
NotifyMessage::setDiagnosticInfo(const TLV_DiagnosticInformation& diagnosticInfo)
{
  _diagnosticInfo = diagnosticInfo;
}

const TLV_DiagnosticInformation&
NotifyMessage::getDiagnosticInfo() const
{
  if ( _diagnosticInfo.isSetValue()) return _diagnosticInfo;
  else throw utilx::FieldNotSetException("NotifyMessage::getDiagnosticInfo::: diagnosticInfo wasn't set");
}

}}}}
