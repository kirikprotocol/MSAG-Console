#include <stdio.h>
#include <eyeline/utilx/Exception.hpp>

#include "DRSTMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t DRSTMessage::_MSG_CODE;

DRSTMessage::DRSTMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
DRSTMessage::serialize(communication::TP* resultBuf) const
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
DRSTMessage::deserialize(const communication::TP& packetBuf)
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
DRSTMessage::toString() const
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
DRSTMessage::getLength() const
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
DRSTMessage::getMsgCodeTextDescription() const
{
  return "DRST";
}

void
DRSTMessage::setAffectedPointCode(const TLV_AffectedPointCode& affectedPointCode)
{
  _affectedPointCodes = affectedPointCode;
}

TLV_AffectedPointCode
DRSTMessage::getAffectedPointCode() const
{
  if ( _affectedPointCodes.isSetValue() ) return _affectedPointCodes;
  else throw utilx::FieldNotSetException("DRSTMessage::getAffectedPointCode::: affectedPointCode wasn't set");
}

void
DRSTMessage::setSSN(const TLV_SSN& ssn)
{
  _ssn = ssn;
}

const TLV_SSN&
DRSTMessage::getSSN() const
{
  if ( _ssn.isSetValue() ) return _ssn;
  else throw utilx::FieldNotSetException("DRSTMessage::getSSN::: ssn wasn't set");
}

void
DRSTMessage::setSMI(const TLV_SMI& smi)
{
  _smi = smi;
}

const TLV_SMI&
DRSTMessage::getSMI() const
{
  if ( _smi.isSetValue() ) return _smi;
  else throw utilx::FieldNotSetException("DRSTMessage::getSMI::: smi wasn't set");
}

void
DRSTMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
DRSTMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DRSTMessage::getInfoString::: infoString wasn't set");
}

}}}}
