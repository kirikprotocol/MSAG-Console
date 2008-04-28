#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "DAVAMessage.hpp"

namespace sua_messages {

const uint32_t DAVAMessage::_MSG_CODE;

DAVAMessage::DAVAMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
DAVAMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _routingContext.isSetValue() )
    offset = _routingContext.serialize(resultBuf, offset);

  if ( _affectedPointCodes.isSetValue() )
    offset = _affectedPointCodes.serialize(resultBuf, offset);

  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(resultBuf, offset);

  if ( _smi.isSetValue() )
    offset = _smi.serialize(resultBuf, offset);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);
  
  return offset;
}

size_t
DAVAMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_routingContext);
  tlvFactory.registerExpectedOptionalTlv(&_affectedPointCodes);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedOptionalTlv(&_smi);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
DAVAMessage::toString() const
{
  std::string result(SUAMessage::toString());

  if ( _routingContext.isSetValue() )
    result += "," + _routingContext.toString();

  if ( _affectedPointCodes.isSetValue() )
    result += "," + _affectedPointCodes.toString();

  if ( _ssn.isSetValue() )
    result += "," + _ssn.toString();

  if ( _smi.isSetValue() )
    result += "," + _smi.toString();

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

  if ( _affectedPointCodes.isSetValue() )
    length += _affectedPointCodes.getLength();

  if ( _ssn.isSetValue() )
    length += _ssn.getLength();

  if ( _smi.isSetValue() )
    length += _smi.getLength();

  if ( _infoString.isSetValue() )
    length += _infoString.getLength();

  return HEADER_SIZE + length;
}

const char*
DAVAMessage::getMsgCodeTextDescription() const
{
  return "DAVA";
}

void
DAVAMessage::setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode)
{
  _affectedPointCodes = affectedPointCode;
}

const TLV_AffectedPointCode&
DAVAMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DAVAMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

void
DAVAMessage::setSSN(const TLV_SSN& ssn)
{
  _ssn = ssn;
}

const TLV_SSN&
DAVAMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("DAVAMessage::getSSN::: ssn wasn't set");
}

void
DAVAMessage::setSMI(const TLV_SMI& smi)
{
  _smi = smi;
}

const TLV_SMI&
DAVAMessage::getSMI() const
{
  if ( _smi.isSetValue() ) return _smi;
  else throw utilx::FieldNotSetException("DAVAMessage::getSMI::: smi wasn't set");
}

void
DAVAMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
DAVAMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DAVAMessage::getInfoString::: infoString wasn't set");
}

}
