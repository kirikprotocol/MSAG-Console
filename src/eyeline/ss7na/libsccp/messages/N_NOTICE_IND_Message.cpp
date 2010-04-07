#include <stdio.h>
#include <string.h>

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "N_NOTICE_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

N_NOTICE_IND_Message::N_NOTICE_IND_Message()
  : LibsccpMessage(_MSG_CODE),
    _fieldsMask(0), _calledAddrLen(0), _callingAddrLen(0),
    _reasonForReturn(0), _userDataLen(0), _importance(0)
{}

N_NOTICE_IND_Message::N_NOTICE_IND_Message(const N_UNITDATA_REQ_Message& message,
                                           common::return_cause_value_t reason_for_return)
  : LibsccpMessage(_MSG_CODE),
    _fieldsMask(0), _calledAddrLen(0), _callingAddrLen(0),
    _reasonForReturn(reason_for_return), _userDataLen(0), _importance(0)
{
  const utilx::variable_data_t& calledAddr = message.getCalledAddress();
  setCalledAddress(calledAddr.data, calledAddr.dataLen);

  const utilx::variable_data_t& callingAddr = message.getCallingAddress();
  setCallingAddress(callingAddr.data, callingAddr.dataLen);

  const utilx::variable_data_t& userData = message.getUserData();
  setUserData(userData.data, userData.dataLen);

  if ( message.isSetImportance() )
    setImportance(message.getImportance());
}

size_t
N_NOTICE_IND_Message::serialize(common::TP* result_buf) const
{
  size_t offset = LibsccpMessage::serialize(result_buf);
  offset = common::addField(result_buf, offset, _fieldsMask);

  offset = common::addField(result_buf, offset, _calledAddrLen);
  offset = common::addField(result_buf, offset, _calledAddr, _calledAddrLen);

  offset = common::addField(result_buf, offset, _callingAddrLen);
  offset = common::addField(result_buf, offset, _callingAddr, _callingAddrLen);

  offset = common::addField(result_buf, offset, _reasonForReturn);

  offset = common::addField(result_buf, offset, _userDataLen);
  offset = common::addField(result_buf, offset, _userData, _userDataLen);

  if ( _fieldsMask & SET_IMPORTANCE )
    offset = common::addField(result_buf, offset, _importance);

  return offset;
}

size_t
N_NOTICE_IND_Message::serialize(uint8_t* result_buf, size_t result_buf_max_sz) const
{
  common::TP tp(0, 0, result_buf, result_buf_max_sz);

  return serialize(&tp);
}

size_t
N_NOTICE_IND_Message::deserialize(const common::TP& packet_buf)
{
  size_t offset = LibsccpMessage::deserialize(packet_buf);
  offset = common::extractField(packet_buf, offset, &_fieldsMask);

  offset = common::extractField(packet_buf, offset, &_calledAddrLen);
  offset = common::extractField(packet_buf, offset, _calledAddr, _calledAddrLen);

  offset = common::extractField(packet_buf, offset, &_callingAddrLen);
  offset = common::extractField(packet_buf, offset, _callingAddr, _callingAddrLen);

  offset = common::extractField(packet_buf, offset, &_reasonForReturn);

  offset = common::extractField(packet_buf, offset, &_userDataLen);
  offset = common::extractField(packet_buf, offset, _userData, _userDataLen);

  if ( _fieldsMask & SET_IMPORTANCE )
    offset = common::extractField(packet_buf, offset, &_importance);

  return offset;
}

size_t
N_NOTICE_IND_Message::deserialize(const uint8_t* packet_buf, size_t packet_buf_sz)
{
  common::TP tp(0, packet_buf_sz, const_cast<uint8_t*>(packet_buf), packet_buf_sz);

  return deserialize(tp);
}

std::string
N_NOTICE_IND_Message::toString() const
{
  std::string result(LibsccpMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  if ( _calledAddrLen )
    result += ",calledAddr=[" + utilx::hexdmp(_calledAddr, _calledAddrLen) + "]";

  if ( _callingAddrLen )
    result += ",callingAddr=[" + utilx::hexdmp(_callingAddr, _callingAddrLen) + "]";

  snprintf(strBuf, sizeof(strBuf), ",reasonForReturn=[%d]", _reasonForReturn);
  result += strBuf;

  if ( _userDataLen )
    result += ",userData=[" + utilx::hexdmp(_userData, _userDataLen) + "]";

  if ( _fieldsMask & SET_IMPORTANCE ) {
    snprintf(strBuf, sizeof(strBuf), ",importance=[%d]", _importance);
    result += strBuf;
  }

  return result;
}

void
N_NOTICE_IND_Message::setCalledAddress(const uint8_t* address, uint8_t address_len)
{
  _calledAddrLen = address_len;
  memcpy(_calledAddr, address, address_len);
}

void
N_NOTICE_IND_Message::setCalledAddress(const sccp::SCCPAddress& address)
{
  _calledAddrLen = address.pack2Octs(_calledAddr);
}

utilx::variable_data_t
N_NOTICE_IND_Message::getCalledAddress() const
{
  if ( !_calledAddrLen )
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getCalledAddress::: calledAddress wasn't set or has length is equal to 0");
  return utilx::variable_data_t(_calledAddr, _calledAddrLen);
}

void
N_NOTICE_IND_Message::setCallingAddress(const uint8_t* address, uint8_t addressLen)
{
  _callingAddrLen = addressLen;
  memcpy(_callingAddr, address, addressLen);
}

void
N_NOTICE_IND_Message::setCallingAddress(const sccp::SCCPAddress& address)
{
  _callingAddrLen = address.pack2Octs(_callingAddr);
}

utilx::variable_data_t
N_NOTICE_IND_Message::getCallingAddress() const
{
  if ( !_callingAddrLen )
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getCallingAddress::: callingAddress wasn't set or has length is equal to 0");
  return utilx::variable_data_t(_callingAddr, _callingAddrLen);
}

void
N_NOTICE_IND_Message::setUserData(const uint8_t* data, uint16_t data_len)
{
  unsigned valriableMsgSize = ((_fieldsMask & SET_IMPORTANCE) ? sizeof(_importance) : 0) +
      _callingAddrLen + _calledAddrLen;
  if ( data_len + FIXED_MSG_PART_SZ + valriableMsgSize > common::TP::MAX_PACKET_SIZE)
    throw smsc::util::Exception("N_NOTICE_IND_Message::setUserData::: too long userdata=%d", data_len);

  _userDataLen = data_len;
  memcpy(_userData, data, data_len);
}

utilx::variable_data_t
N_NOTICE_IND_Message::getUserData() const
{
  if ( !_userDataLen )
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getUserData::: userData wasn't set or has length is equal to 0");
  return utilx::variable_data_t(_userData, _userDataLen);
}

void
N_NOTICE_IND_Message::setImportance(uint8_t importance)
{
  _importance = importance;
  _fieldsMask |= SET_IMPORTANCE;
}

uint8_t
N_NOTICE_IND_Message::getImportance() const
{
  if ( _fieldsMask & SET_IMPORTANCE )
    return _importance;
  else
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getImportance::: importance wasn't set");
}

uint32_t
N_NOTICE_IND_Message::getLength() const
{
  return LibsccpMessage::getLength() +
    static_cast<uint32_t>(sizeof(_fieldsMask) + sizeof(_calledAddrLen) + _calledAddrLen +
                  sizeof(_callingAddrLen) + _callingAddrLen + sizeof(_reasonForReturn) +
                  sizeof(_userDataLen) + _userDataLen + 
                  ( (_fieldsMask & SET_IMPORTANCE) ? sizeof(_importance) : 0 ));
}

}}}
