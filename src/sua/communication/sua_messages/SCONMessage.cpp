#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "SCONMessage.hpp"

namespace sua_messages {

const uint32_t SCONMessage::_MSG_CODE;

SCONMessage::SCONMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
SCONMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(resultBuf, offset);

  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(resultBuf, offset);

  if ( _congestionLevel.isSetValue() )
    offset = _congestionLevel.serialize(resultBuf, offset);
  else
    throw smsc::util::Exception("SCONMessage::getLength::: congestionLevel is a mandatory field and wasn't set");

  if ( _smi.isSetValue() )
    offset = _smi.serialize(resultBuf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);
  
  return offset;
}

size_t
SCONMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedMandatoryTlv(&_congestionLevel);
  tlvFactory.registerExpectedOptionalTlv(&_smi);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
SCONMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _ssn.isSetValue() )
    result += "," + _ssn.toString();

  if ( _congestionLevel.isSetValue() )
    result += "," + _congestionLevel.toString();

  if ( _smi.isSetValue() )
    result += "," + _smi.toString();

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

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _ssn.isSetValue() )
    length += _ssn.getLength();

  if ( _congestionLevel.isSetValue() )
    length += _congestionLevel.getLength();
  else
    throw smsc::util::Exception("SCONMessage::getLength::: congestionLevel is a mandatory field and wasn't set");

  if ( _smi.isSetValue() )
    length += _smi.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
SCONMessage::getMsgCodeTextDescription() const
{
  return "SCON";
}

void
SCONMessage::setRoutingContext(const TLV_RoutingContext& routingContext)
{
  _routingContext = routingContext;
}

const TLV_RoutingContext&
SCONMessage::getRoutingContext() const
{
  if ( _routingContext.isSetValue() ) return _routingContext;
  else throw utilx::FieldNotSetException("SCONMessage::getRoutingContext::: routingContext wasn't set");
}

void
SCONMessage::setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode)
{
  _affectedPointCodes = affectedPointCode;
}

const TLV_AffectedPointCode&
SCONMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("SCONMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

void
SCONMessage::setSSN(const TLV_SSN& ssn)
{
  _ssn = ssn;
}

const TLV_SSN&
SCONMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("SCONMessage::getSSN::: ssn wasn't set");
}

void
SCONMessage::setCongestionLevel(const TLV_CongestionLevel& congestionLevel)
{
  _congestionLevel = congestionLevel;
}

const TLV_CongestionLevel&
SCONMessage::getCongestionLevel() const
{
  if ( _congestionLevel.isSetValue() ) return _congestionLevel;
  else throw utilx::FieldNotSetException("SCONMessage::getCongestionLevelI::: congestionLevel wasn't set");
}

void
SCONMessage::setSMI(const TLV_SMI& smi)
{
  _smi = smi;
}

const TLV_SMI&
SCONMessage::getSMI() const
{
  if ( _smi.isSetValue() ) return _smi;
  else throw utilx::FieldNotSetException("SCONMessage::getSMI::: smi wasn't set");
}

void
SCONMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
SCONMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("SCONMessage::getInfoString::: infoString wasn't set");
}

}
