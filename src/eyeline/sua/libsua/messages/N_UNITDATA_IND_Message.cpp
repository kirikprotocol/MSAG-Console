#include <stdio.h>
#include <string.h>
#include <eyeline/sua/utilx/hexdmp.hpp>
#include <eyeline/sua/utilx/Exception.hpp>
#include <eyeline/sua/communication/TP.hpp>
#include "N_UNITDATA_IND_Message.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

N_UNITDATA_IND_Message::N_UNITDATA_IND_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _sequenceControl(0),
    _calledAddrLen(0), _calledAddr(NULL),
    _callingAddrLen(0), _callingAddr(NULL),
    _userDataLen(0), _userData(NULL)
{}

N_UNITDATA_IND_Message::~N_UNITDATA_IND_Message()
{
  delete [] _calledAddr;
  delete [] _callingAddr;
  delete [] _userData;
}

size_t
N_UNITDATA_IND_Message::serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf, resultBufMaxSz);

  communication::TP tp(0, 0, resultBuf, resultBufMaxSz);

  offset = communication::addField(&tp, offset, _fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = communication::addField(&tp, offset, _sequenceControl);

  offset = communication::addField(&tp, offset, _calledAddrLen);
  offset = communication::addField(&tp, offset, _calledAddr, _calledAddrLen);

  offset = communication::addField(&tp, offset, _callingAddrLen);
  offset = communication::addField(&tp, offset, _callingAddr, _callingAddrLen);

  offset = communication::addField(&tp, offset, _userDataLen);

  return communication::addField(&tp, offset, _userData, _userDataLen);
}

size_t
N_UNITDATA_IND_Message::deserialize(const uint8_t* packetBuf, size_t packetBufSz)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf, packetBufSz);

  communication::TP tp(0, packetBufSz, const_cast<uint8_t*>(packetBuf), packetBufSz);

  offset = communication::extractField(tp, offset, &_fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = communication::extractField(tp, offset, &_sequenceControl);

  offset = communication::extractField(tp, offset, &_calledAddrLen);
  if ( _calledAddr) delete [] _calledAddr;
  _calledAddr = new uint8_t[_calledAddrLen];
  offset = communication::extractField(tp, offset, _calledAddr, _calledAddrLen);

  offset = communication::extractField(tp, offset, &_callingAddrLen);
  if ( _callingAddr ) delete [] _callingAddr;
  _callingAddr = new uint8_t[_callingAddrLen];
  offset = communication::extractField(tp, offset, _callingAddr, _callingAddrLen);

  offset = communication::extractField(tp, offset, &_userDataLen);
  if ( _userData ) delete [] _userData;
  _userData = new uint8_t[_userDataLen];
  return communication::extractField(tp, offset, _userData, _userDataLen);
}

std::string
N_UNITDATA_IND_Message::toString() const
{
  std::string result(LibsuaMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  if ( _fieldsMask & SET_SEQUENCE_CONTROL ) {
    snprintf(strBuf, sizeof(strBuf), ",sequenceControl=[%d]", _sequenceControl);
    result += strBuf;
  }

  if ( _calledAddrLen )
    result += ",calledAddr=[" + utilx::hexdmp(_calledAddr, _calledAddrLen) + "]";

  if ( _callingAddrLen )
    result += ",callingAddr=[" + utilx::hexdmp(_callingAddr, _callingAddrLen) + "]";

  if ( _userDataLen )
    result += ",userData=[" + utilx::hexdmp(_userData, _userDataLen) + "]";

  return result;
}

const char*
N_UNITDATA_IND_Message::getMsgCodeTextDescription() const
{
  return "N_UNITDATA_IND_Message";
}

void
N_UNITDATA_IND_Message::setSequenceControl(uint32_t sequenceControl)
{
  _sequenceControl = sequenceControl;
  _fieldsMask |= SET_SEQUENCE_CONTROL;
}

uint32_t
N_UNITDATA_IND_Message::getSequenceControl() const
{
  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    return _sequenceControl;
  else
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getSequenceControl::: sequenceControl wasn't set");
}

void
N_UNITDATA_IND_Message::setCalledAddress(const uint8_t* address, uint8_t addressLen)
{
  _calledAddrLen = addressLen;
  if ( _calledAddr ) delete [] _calledAddr;
  _calledAddr = new uint8_t[addressLen];
  memcpy(_calledAddr, address, addressLen);
}

variable_data_t
N_UNITDATA_IND_Message::getCalledAddress() const
{
  if ( !_calledAddrLen )
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getCalledAddress::: calledAddress wasn't set or has length is equal to 0");
  return variable_data_t(_calledAddr, _calledAddrLen);
}

void
N_UNITDATA_IND_Message::setCallingAddress(const uint8_t* address, uint8_t addressLen)
{
  _callingAddrLen = addressLen;
  if ( _callingAddr) delete [] _callingAddr;
  _callingAddr = new uint8_t[addressLen];
  memcpy(_callingAddr, address, addressLen);
}

variable_data_t
N_UNITDATA_IND_Message::getCallingAddress() const
{
  if ( !_callingAddrLen )
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getCallingAddress::: callingAddress wasn't set or has length is equal to 0");
  return variable_data_t(_callingAddr, _callingAddrLen);
}

void
N_UNITDATA_IND_Message::setUserData(const uint8_t* data, uint16_t dataLen)
{
  _userDataLen = dataLen;
  if ( _userData ) delete [] _userData;
  _userData = new uint8_t[_userDataLen];
  memcpy(_userData, data, dataLen);
}

variable_data_t
N_UNITDATA_IND_Message::getUserData() const
{
  if ( !_userDataLen )
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getUserData::: userData wasn't set or has length is equal to 0");
  return variable_data_t(_userData, _userDataLen);
}

uint32_t
N_UNITDATA_IND_Message::getLength() const
{
  return
    LibsuaMessage::getLength() + sizeof(_fieldsMask) + 
    ( (_fieldsMask & SET_SEQUENCE_CONTROL) ? sizeof(_sequenceControl) : 0 ) +
    sizeof(_calledAddrLen) + _calledAddrLen + sizeof(_callingAddrLen) + _callingAddrLen + 
    sizeof(_userDataLen) + _userDataLen;
}

}}}
