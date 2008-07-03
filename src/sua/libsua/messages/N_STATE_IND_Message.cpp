#include <stdio.h>
#include <sua/utilx/Exception.hpp>
#include <sua/communication/TP.hpp>
#include "N_STATE_IND_Message.hpp"

namespace libsua {

N_STATE_IND_Message::N_STATE_IND_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _pointCode(0), _ssn(0),
    _userStatus(0), _subsystemMultiplicityIndicator(0)
{}

size_t
N_STATE_IND_Message::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf, resultBufMaxSz);

  communication::TP tp(0, 0, resultBuf, resultBufMaxSz);

  offset = communication::addField(&tp, offset, _fieldsMask);

  offset = communication::addField(&tp, offset, _pointCode);

  offset = communication::addField(&tp, offset, _ssn);

  offset = communication::addField(&tp, offset, _userStatus);

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    offset = communication::addField(&tp, offset, _subsystemMultiplicityIndicator);

  return offset;
}

size_t
N_STATE_IND_Message::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf, packetBufSz);

  communication::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  offset = communication::extractField(tp, offset, &_fieldsMask);

  offset = communication::extractField(tp, offset, &_pointCode);

  offset = communication::extractField(tp, offset, &_ssn);

  offset = communication::extractField(tp, offset, &_userStatus);

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    offset = communication::extractField(tp, offset, &_subsystemMultiplicityIndicator);

  return offset;
}

std::string
N_STATE_IND_Message::toString() const
{
  std::string result(LibsuaMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",pointCode=[%d]", _pointCode);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",ssn=[%d]", _ssn);
  result += strBuf;

  snprintf(strBuf, sizeof(strBuf), ",userStatus=[%d]", _userStatus);
  result += strBuf;

  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND ) {
    snprintf(strBuf, sizeof(strBuf), ",subsystemMultiplicityIndicator=[%d]", _subsystemMultiplicityIndicator);
    result += strBuf;
  }

  return result;
}

const char*
N_STATE_IND_Message::getMsgCodeTextDescription() const
{
  return "N_STATE_IND_Message";
}

void
N_STATE_IND_Message::setPointCode(uint16_t pointCode)
{
  _pointCode = pointCode;
}

uint16_t
N_STATE_IND_Message::getPointCode() const
{
  return _pointCode;
}

void
N_STATE_IND_Message::setSSN(uint8_t ssn)
{
  _ssn = ssn;
}

uint8_t
N_STATE_IND_Message::getSSN() const
{
  return _ssn;
}

void
N_STATE_IND_Message::setUserStatus(uint8_t userStatus)
{
  _userStatus = userStatus;
}

uint8_t
N_STATE_IND_Message::getUserStatus() const
{
  return _userStatus;
}

void
N_STATE_IND_Message::setSubsystemMultiplicityInd(uint8_t subsystemMultInd)
{
  _subsystemMultiplicityIndicator = subsystemMultInd;
  _fieldsMask |= SET_SUBSYSTEM_MULTIPLICITY_IND;
}

uint8_t
N_STATE_IND_Message::getSubsystemMultiplicityInd() const
{
  if ( _fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND )
    return _subsystemMultiplicityIndicator;
  else
    throw utilx::FieldNotSetException("N_STATE_IND_Message::getSubsystemMultiplicityInd::: subsystemMultiplicityIndicator wasn't set");
}

uint32_t
N_STATE_IND_Message::getLength() const
{
  return
    LibsuaMessage::getLength() + sizeof(_fieldsMask) + sizeof(_pointCode) +
    sizeof(_ssn) + sizeof(_userStatus) + 
    ( (_fieldsMask & SET_SUBSYSTEM_MULTIPLICITY_IND) ? sizeof(_subsystemMultiplicityIndicator) : 0 );
}

}
