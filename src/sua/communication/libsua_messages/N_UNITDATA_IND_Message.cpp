#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include <sua/communication/sua_messages/SCCPAddress.hpp>
#include "N_UNITDATA_IND_Message.hpp"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace libsua_messages {

N_UNITDATA_IND_Message::N_UNITDATA_IND_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _sequenceControl(0),
    _calledAddrLen(0), _callingAddrLen(0), _userDataLen(0)
{}

N_UNITDATA_IND_Message::N_UNITDATA_IND_Message(const sua_messages::CLDTMessage& cldtMessage)
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _sequenceControl(0),
    _calledAddrLen(0), _callingAddrLen(0), _userDataLen(0)
{
  setSequenceControl(cldtMessage.getSequenceControl().getValue());

  const sua_messages::TLV_DestinationAddress& dstAddress = cldtMessage.getDestinationAddress();

  if ( dstAddress.getGlobalTitle().isSetValue() ) {
    if ( dstAddress.getSSN().isSetValue() ) {
      setCalledAddress(sua_messages::SCCPAddress(dstAddress.getGlobalTitle(), dstAddress.getSSN()));
    } else {
      setCalledAddress(sua_messages::SCCPAddress(dstAddress.getGlobalTitle()));
    }
  } else if ( dstAddress.getPointCode().isSetValue() && dstAddress.getSSN().isSetValue() ) {
    setCalledAddress(sua_messages::SCCPAddress(dstAddress.getPointCode(), dstAddress.getSSN()));
  } else {
    throw smsc::util::Exception("N_UNITDATA_IND_Message::N_UNITDATA_IND_Message::: wrong dstAddress in incoming CLDT message");
  }

  const sua_messages::TLV_SourceAddress& srcAddress = cldtMessage.getSourceAddress();

  if ( srcAddress.getGlobalTitle().isSetValue() ) {
    if ( srcAddress.getSSN().isSetValue() ) {
      setCallingAddress(sua_messages::SCCPAddress(srcAddress.getGlobalTitle(), srcAddress.getSSN()));
    } else {
      setCallingAddress(sua_messages::SCCPAddress(srcAddress.getGlobalTitle()));
    }
  } else if ( srcAddress.getPointCode().isSetValue() && srcAddress.getSSN().isSetValue() ) {
    setCallingAddress(sua_messages::SCCPAddress(srcAddress.getPointCode(), srcAddress.getSSN()));
  } else {
    throw smsc::util::Exception("N_UNITDATA_IND_Message::N_UNITDATA_IND_Message::: wrong srcAddress in incoming CLDT message");
  }
  const sua_messages::TLV_Data& userData = cldtMessage.getData();
  setUserData(userData.getValue(), userData.getValueLength());
}

size_t
N_UNITDATA_IND_Message::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  offset = communication::addField(resultBuf, offset, _fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = communication::addField(resultBuf, offset, _sequenceControl);

  offset = communication::addField(resultBuf, offset, _calledAddrLen);
  offset = communication::addField(resultBuf, offset, _calledAddr, _calledAddrLen);

  offset = communication::addField(resultBuf, offset, _callingAddrLen);
  offset = communication::addField(resultBuf, offset, _callingAddr, _callingAddrLen);

  offset = communication::addField(resultBuf, offset, _userDataLen);

  return communication::addField(resultBuf, offset, _userData, _userDataLen);
}

size_t
N_UNITDATA_IND_Message::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  offset = communication::extractField(packetBuf, offset, &_fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = communication::extractField(packetBuf, offset, &_sequenceControl);

  offset = communication::extractField(packetBuf, offset, &_calledAddrLen);
  offset = communication::extractField(packetBuf, offset, _calledAddr, _calledAddrLen);

  offset = communication::extractField(packetBuf, offset, &_callingAddrLen);
  offset = communication::extractField(packetBuf, offset, _callingAddr, _callingAddrLen);

  offset = communication::extractField(packetBuf, offset, &_userDataLen);
  return communication::extractField(packetBuf, offset, _userData, _userDataLen);
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
    result += ",calledAddr=[" + hexdmp(_calledAddr, _calledAddrLen) + "]";

  if ( _callingAddrLen )
    result += ",callingAddr=[" + hexdmp(_callingAddr, _callingAddrLen) + "]";

  if ( _userDataLen )
    result += ",userData=[" + hexdmp(_userData, _userDataLen) + "]";

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
  memcpy(_calledAddr, address, addressLen);
}

void
N_UNITDATA_IND_Message::setCalledAddress(const sua_messages::SCCPAddress& address)
{
  _calledAddrLen = address.serialize(_calledAddr, sizeof(_calledAddr));
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
  memcpy(_callingAddr, address, addressLen);
}

void
N_UNITDATA_IND_Message::setCallingAddress(const sua_messages::SCCPAddress& address)
{
  _callingAddrLen = address.serialize(_callingAddr, sizeof(_callingAddr));
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

}
