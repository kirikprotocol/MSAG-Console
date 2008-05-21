#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "DUPUMessage.hpp"

namespace sua_messages {

const uint32_t DUPUMessage::_MSG_CODE;

DUPUMessage::DUPUMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
DUPUMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _user_cause.isSetValue() )
    offset = _user_cause.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("DUPUMessage::serialize::: user_cause is a mandatary field and wasn't set");

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(resultBuf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);
  
  return offset;
}

size_t
DUPUMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedMandatoryTlv(&_user_cause);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
DUPUMessage::toString() const
{
  std::string result(SUAMessage::toString());

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

const char*
DUPUMessage::getMsgCodeTextDescription() const
{
  return "DUPU";
}

void
DUPUMessage::setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode)
{
  _affectedPointCodes = affectedPointCode;
}

TLV_AffectedPointCode
DUPUMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DUPUMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

void
DUPUMessage::setUserCause(const TLV_UserCause& user_cause)
{
  _user_cause = user_cause;
}

const TLV_UserCause&
DUPUMessage::getUserCause() const
{
  if ( _user_cause.isSetValue() ) return _user_cause;
  else throw utilx::FieldNotSetException("DUPUMessage::getUserCause::: user_cause wasn't set");
}

void
DUPUMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
DUPUMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DUPUMessage::getInfoString::: infoString wasn't set");
}

}
