#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "DAUDMessage.hpp"

namespace sua_messages {

const uint32_t DAUDMessage::_MSG_CODE;

DAUDMessage::DAUDMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
DAUDMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(resultBuf, offset);

  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(resultBuf, offset);

  if ( _user_cause.isSetValue() )
    offset = _user_cause.serialize(resultBuf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);
  
  return offset;
}

size_t
DAUDMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedOptionalTlv(&_user_cause);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
DAUDMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _ssn.isSetValue() )
    result += "," + _ssn.toString();

  if ( _user_cause.isSetValue() )
    result += "," + _user_cause.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
DAUDMessage::getLength() const
{
  uint32_t length=0;

  if ( _routingContext.isSetValue() )
    length = _routingContext.getLength();

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _ssn.isSetValue() )
    length += _ssn.getLength();

  if ( _user_cause.isSetValue() )
    length += _user_cause.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
DAUDMessage::getMsgCodeTextDescription() const
{
  return "DAUD";
}

void
DAUDMessage::setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode)
{
  _affectedPointCodes = affectedPointCode;
}

const TLV_AffectedPointCode&
DAUDMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DAUDMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

void
DAUDMessage::setSSN(const TLV_SSN& ssn)
{
  _ssn = ssn;
}

const TLV_SSN&
DAUDMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("DAUDMessage::getSSN::: ssn wasn't set");
}

void
DAUDMessage::setUserCause(const TLV_UserCause& user_cause)
{
  _user_cause = user_cause;
}

const TLV_UserCause&
DAUDMessage::getUserCause() const
{
  if ( _user_cause.isSetValue() ) return _user_cause;
  else throw utilx::FieldNotSetException("DAUDMessage::getUserCause::: user_cause wasn't set");
}

void
DAUDMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
DAUDMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DAUDMessage::getInfoString::: infoString wasn't set");
}

}
