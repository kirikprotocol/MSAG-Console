#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include "N_UNITDATA_REQ_Message.hpp"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace libsua_messages {

N_UNITDATA_REQ_Message::N_UNITDATA_REQ_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _sequenceControl(0), _returnOption(0), _importance(0), _hopCounter(0),
    _calledAddrLen(0), _callingAddrLen(0), _userDataLen(0)
{}

size_t
N_UNITDATA_REQ_Message::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  offset = communication::addField(resultBuf, offset, _fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = communication::addField(resultBuf, offset, _sequenceControl);

  offset = communication::addField(resultBuf, offset, _returnOption);

  if ( _fieldsMask & SET_IMPORTANCE )
    offset = communication::addField(resultBuf, offset, _importance);

  if ( _fieldsMask & SET_HOP_COUNTER )
    offset = communication::addField(resultBuf, offset, _hopCounter);

  offset = communication::addField(resultBuf, offset, _calledAddrLen);
  offset = communication::addField(resultBuf, offset, _calledAddr, _calledAddrLen);

  offset = communication::addField(resultBuf, offset, _callingAddrLen);
  offset = communication::addField(resultBuf, offset, _callingAddr, _callingAddrLen);

  offset = communication::addField(resultBuf, offset, _userDataLen);

  return communication::addField(resultBuf, offset, _userData, _userDataLen);
}

size_t
N_UNITDATA_REQ_Message::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  offset = communication::extractField(packetBuf, offset, &_fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = communication::extractField(packetBuf, offset, &_sequenceControl);

  offset = communication::extractField(packetBuf, offset, &_returnOption);
  if ( _returnOption > 1 )
    throw smsc::util::Exception("N_UNITDATA_REQ_Message::deserialize::: wrong value=[%d] for returnOption mesage's parameter", _returnOption);

  if ( _fieldsMask & SET_IMPORTANCE )
    offset = communication::extractField(packetBuf, offset, &_importance);

  if ( _fieldsMask & SET_HOP_COUNTER )
    offset = communication::extractField(packetBuf, offset, &_hopCounter);

  offset = communication::extractField(packetBuf, offset, &_calledAddrLen);
  offset = communication::extractField(packetBuf, offset, _calledAddr, _calledAddrLen);

  offset = communication::extractField(packetBuf, offset, &_callingAddrLen);
  offset = communication::extractField(packetBuf, offset, _callingAddr, _callingAddrLen);

  offset = communication::extractField(packetBuf, offset, &_userDataLen);
  return communication::extractField(packetBuf, offset, _userData, _userDataLen);
}

std::string
N_UNITDATA_REQ_Message::toString() const
{
  std::string result(LibsuaMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  if ( _fieldsMask & SET_SEQUENCE_CONTROL ) {
    snprintf(strBuf, sizeof(strBuf), ",sequenceControl=[%d]", _sequenceControl);
    result += strBuf;
  }

  snprintf(strBuf, sizeof(strBuf), ",returnOption=[%d]", _returnOption);
  result += strBuf;

  if ( _fieldsMask & SET_IMPORTANCE ) {
    snprintf(strBuf, sizeof(strBuf), ",importance=[%d]", _importance);
    result += strBuf;
  }

  if ( _fieldsMask & SET_HOP_COUNTER ) {
    snprintf(strBuf, sizeof(strBuf), ",hopCounter=[%d]", _hopCounter);
    result += strBuf;
  }

  if ( _calledAddrLen )
    result += ",calledAddr=[" + hexdmp(_calledAddr, _calledAddrLen) + "]";

  if ( _callingAddrLen )
    result += ",callingAddr=[" + hexdmp(_callingAddr, _callingAddrLen) + "]";

  if ( _userDataLen )
    result += ",userData=[" + hexdmp(_userData, _userDataLen) + "]";

  return result;
}

const char*
N_UNITDATA_REQ_Message::getMsgCodeTextDescription() const
{
  return "N_UNITDATA_REQ_Message";
}

void
N_UNITDATA_REQ_Message::setSequenceControl(uint32_t sequenceControl)
{
  _sequenceControl = sequenceControl;
  _fieldsMask |= SET_SEQUENCE_CONTROL;
}

uint32_t
N_UNITDATA_REQ_Message::getSequenceControl() const
{
  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    return _sequenceControl;
  else
    throw utilx::FieldNotSetException("N_UNITDATA_REQ_Message::getSequenceControl::: sequenceControl wasn't set");
}

void
N_UNITDATA_REQ_Message::setReturnOption(bool on_off)
{
  _returnOption = on_off;
}

bool
N_UNITDATA_REQ_Message::getReturnOption() const
{
  return _returnOption;
}

void
N_UNITDATA_REQ_Message::setImportance(uint8_t importance)
{
  _importance = importance;
  _fieldsMask |= SET_IMPORTANCE;
}

uint8_t
N_UNITDATA_REQ_Message::getImportance() const
{
  if ( _fieldsMask & SET_IMPORTANCE )
    return _importance;
  else
    throw utilx::FieldNotSetException("N_UNITDATA_REQ_Message::getImportance::: importance wasn't set");
}

void
N_UNITDATA_REQ_Message::setHopCounter(uint8_t hopCounter)
{
  _hopCounter = hopCounter;
  _fieldsMask |= SET_HOP_COUNTER;
}

uint8_t
N_UNITDATA_REQ_Message::getHopCounter() const
{
  if ( _fieldsMask & SET_HOP_COUNTER )
    return _hopCounter;
  else
    throw utilx::FieldNotSetException("N_UNITDATA_REQ_Message::getHopCounter::: hopCounter wasn't set");
}

void
N_UNITDATA_REQ_Message::setCalledAddress(const uint8_t* address, uint8_t addressLen)
{
  _calledAddrLen = addressLen;
  memcpy(_calledAddr, address, addressLen);
}

void
N_UNITDATA_REQ_Message::setCalledAddress(const sua_messages::SCCPAddress& address)
{
  _calledAddrLen = address.serialize(_calledAddr, sizeof(_calledAddr));
}

variable_data_t
N_UNITDATA_REQ_Message::getCalledAddress() const
{
  if ( !_calledAddrLen )
    throw utilx::FieldNotSetException("N_UNITDATA_REQ_Message::getCalledAddress::: calledAddress wasn't set or has length is equal to 0");
  return variable_data_t(_calledAddr, _calledAddrLen);
}

void
N_UNITDATA_REQ_Message::setCallingAddress(const uint8_t* address, uint8_t addressLen)
{
  _callingAddrLen = addressLen;
  memcpy(_callingAddr, address, addressLen);
}

void
N_UNITDATA_REQ_Message::setCallingAddress(const sua_messages::SCCPAddress& address)
{
  _callingAddrLen = address.serialize(_callingAddr, sizeof(_callingAddr));
}

variable_data_t
N_UNITDATA_REQ_Message::getCallingAddress() const
{
  if ( !_callingAddrLen )
    throw utilx::FieldNotSetException("N_UNITDATA_REQ_Message::getCallingAddress::: callingAddress wasn't set or has length is equal to 0");
  return variable_data_t(_callingAddr, _callingAddrLen);
}

void
N_UNITDATA_REQ_Message::setUserData(const uint8_t* data, uint16_t dataLen)
{
  _userDataLen = dataLen;
  memcpy(_userData, data, dataLen);
}

variable_data_t
N_UNITDATA_REQ_Message::getUserData() const
{
  if ( !_userDataLen )
    throw utilx::FieldNotSetException("N_UNITDATA_REQ_Message::getUserData::: userData wasn't set or has length is equal to 0");
  return variable_data_t(_userData, _userDataLen);
}

uint32_t
N_UNITDATA_REQ_Message::getLength() const
{
  return
    LibsuaMessage::getLength() + sizeof(_fieldsMask) + 
    ( (_fieldsMask & SET_SEQUENCE_CONTROL) ? sizeof(_sequenceControl) : 0 ) +
    sizeof(_returnOption) +
    ( (_fieldsMask & SET_IMPORTANCE ) ? sizeof(_importance) : 0 ) + 
    ( (_fieldsMask & SET_HOP_COUNTER ) ? sizeof(_hopCounter) : 0 ) + 
    sizeof(_calledAddrLen) + _calledAddrLen + sizeof(_callingAddrLen) + _callingAddrLen + 
    sizeof(_userDataLen) + _userDataLen;
}

}
