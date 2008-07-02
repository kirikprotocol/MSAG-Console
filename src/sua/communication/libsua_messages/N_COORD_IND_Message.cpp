#include <stdio.h>
#include <sua/utilx/Exception.hpp>
#include "N_COORD_IND_Message.hpp"

namespace libsua_messages {

N_COORD_IND_Message::N_COORD_IND_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _pointCode(0), _ssn(0),
    _subsystemMultiplicityIndicator(0)
{}

size_t
N_COORD_IND_Message::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);

  offset = communication::addField(resultBuf, offset, _fieldsMask);

  offset = communication::addField(resultBuf, offset, _pointCode);

  offset = communication::addField(resultBuf, offset, _ssn);

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    offset = communication::addField(resultBuf, offset, _subsystemMultiplicityIndicator);

  return offset;
}

size_t
N_COORD_IND_Message::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);

  offset = communication::extractField(packetBuf, offset, &_fieldsMask);

  offset = communication::extractField(packetBuf, offset, &_pointCode);

  offset = communication::extractField(packetBuf, offset, &_ssn);

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    offset = communication::extractField(packetBuf, offset, &_subsystemMultiplicityIndicator);

  return offset;
}

std::string
N_COORD_IND_Message::toString() const
{
  std::string result(LibsuaMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",pointCode=[%d]", _pointCode);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",ssn=[%d]", _ssn);
  result += strBuf;

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND ) {
    snprintf(strBuf, sizeof(strBuf), ",subsystemMultiplicityIndicator=[%d]", _subsystemMultiplicityIndicator);
    result += strBuf;
  }

  return result;
}

const char*
N_COORD_IND_Message::getMsgCodeTextDescription() const
{
  return "N_COORD_IND_Message";
}

void
N_COORD_IND_Message::setSignalingPointCode(uint16_t pointCode)
{
  _pointCode = pointCode;
}

uint16_t
N_COORD_IND_Message::getSignalingPointCode() const
{
  return _pointCode;
}

void
N_COORD_IND_Message::setSSN(uint8_t ssn)
{
  _ssn = ssn;
}

uint8_t
N_COORD_IND_Message::getSSN() const
{
  return _ssn;
}

void
N_COORD_IND_Message::setSubsystemMultiplicityInd(uint8_t subsystemMultInd)
{
  _subsystemMultiplicityIndicator = subsystemMultInd;
  _fieldsMask |= SET_SUBSYSTEM_MULTIPLICITY_IND;
}

uint8_t
N_COORD_IND_Message::getSubsystemMultiplicityInd() const
{
  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    return _subsystemMultiplicityIndicator;
  else
    throw utilx::FieldNotSetException("N_COORD_IND_Message::getSubsystemMultiplicityInd::: subsystemMultiplicityIndicator wasn't set");
}

uint32_t
N_COORD_IND_Message::getLength() const
{
  return LibsuaMessage::getLength() + sizeof(_fieldsMask) + sizeof(_pointCode) + sizeof(_ssn) +
    ( (_fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND) ? sizeof(_subsystemMultiplicityIndicator) : 0 );
}

}
