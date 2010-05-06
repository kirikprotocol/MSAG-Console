#include <stdio.h>
#include <string.h>

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "N_UNITDATA_IND_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

N_UNITDATA_IND_Message::N_UNITDATA_IND_Message()
  : LibsccpMessage(_MSG_CODE),
    _fieldsMask(0), _sequenceControl(0),
    _calledAddrLen(0), _callingAddrLen(0), _userDataLen(0), _userData(NULL)
{}

size_t
N_UNITDATA_IND_Message::serialize(common::TP* result_buf) const
{
  size_t offset = LibsccpMessage::serialize(result_buf);
  offset = common::addField(result_buf, offset, _fieldsMask);

  if ( isSetSequenceControl() )
    offset = common::addField(result_buf, offset, _sequenceControl);

  offset = common::addField(result_buf, offset, _calledAddrLen);
  offset = common::addField(result_buf, offset, _calledAddr, _calledAddrLen);

  offset = common::addField(result_buf, offset, _callingAddrLen);
  offset = common::addField(result_buf, offset, _callingAddr, _callingAddrLen);

  offset = common::addField(result_buf, offset, _userDataLen);

  return common::addField(result_buf, offset, _userData, _userDataLen);
}

size_t
N_UNITDATA_IND_Message::serialize(uint8_t* result_buf, size_t result_buf_max_sz) const
{
  common::TP tp(0, 0, result_buf, result_buf_max_sz);
  return serialize(&tp);
}

size_t
N_UNITDATA_IND_Message::deserialize(const common::TP& packet_buf)
{
  size_t offset = LibsccpMessage::deserialize(packet_buf);
  offset = common::extractField(packet_buf, offset, &_fieldsMask);

  if ( _fieldsMask & SET_SEQUENCE_CONTROL )
    offset = common::extractField(packet_buf, offset, &_sequenceControl);

  offset = common::extractField(packet_buf, offset, &_calledAddrLen);
  offset = common::extractField(packet_buf, offset, _calledAddr, _calledAddrLen);

  offset = common::extractField(packet_buf, offset, &_callingAddrLen);
  offset = common::extractField(packet_buf, offset, _callingAddr, _callingAddrLen);

  offset = common::extractField(packet_buf, offset, &_userDataLen);
  offset = common::extractField(packet_buf, offset, _userDataBuf, _userDataLen);
  _userData = _userDataBuf;
  return offset;
}

size_t
N_UNITDATA_IND_Message::deserialize(const uint8_t* packet_buf, size_t packet_buf_sz)
{
  common::TP tp(0, packet_buf_sz, const_cast<uint8_t*>(packet_buf), packet_buf_sz);

  return deserialize(tp);
}

std::string
N_UNITDATA_IND_Message::toString() const
{
  std::string result(LibsccpMessage::toString());

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

void
N_UNITDATA_IND_Message::setSequenceControl(uint32_t sequence_control)
{
  _sequenceControl = sequence_control;
  _fieldsMask |= SET_SEQUENCE_CONTROL;
}

bool
N_UNITDATA_IND_Message::isSetSequenceControl() const
{
  return _fieldsMask & SET_SEQUENCE_CONTROL;
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
N_UNITDATA_IND_Message::setCalledAddress(const uint8_t* address, uint8_t address_len)
{
  _calledAddrLen = address_len;
  memcpy(_calledAddr, address, address_len);
}

void
N_UNITDATA_IND_Message::setCalledAddress(const sccp::SCCPAddress& address)
{
  _calledAddrLen = address.pack2Octs(_calledAddr);
}

utilx::variable_data_t
N_UNITDATA_IND_Message::getCalledAddress() const
{
  if ( !_calledAddrLen )
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getCalledAddress::: calledAddress wasn't set or has length is equal to 0");
  return utilx::variable_data_t(_calledAddr, _calledAddrLen);
}

void
N_UNITDATA_IND_Message::setCallingAddress(const uint8_t* address, uint8_t address_len)
{
  _callingAddrLen = address_len;
  memcpy(_callingAddr, address, address_len);
}

void
N_UNITDATA_IND_Message::setCallingAddress(const sccp::SCCPAddress& address)
{
  _callingAddrLen = address.pack2Octs(_callingAddr);
}

utilx::variable_data_t
N_UNITDATA_IND_Message::getCallingAddress() const
{
  if ( !_callingAddrLen )
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getCallingAddress::: callingAddress wasn't set or has length is equal to 0");
  return utilx::variable_data_t(_callingAddr, _callingAddrLen);
}

void
N_UNITDATA_IND_Message::setUserData(const uint8_t* data, uint16_t data_len)
{
  unsigned valriableMsgSize = ((_fieldsMask & SET_SEQUENCE_CONTROL) ? static_cast<unsigned>(sizeof(_sequenceControl)) : 0) +
       _callingAddrLen + _calledAddrLen;
  if ( data_len + FIXED_MSG_PART_SZ + valriableMsgSize > common::TP::MAX_PACKET_SIZE)
    throw smsc::util::Exception("N_UNITDATA_IND_Message::setUserData::: too long userdata=%d", data_len);

  _userDataLen = data_len;
  _userData = data;
}

utilx::variable_data_t
N_UNITDATA_IND_Message::getUserData() const
{
  if ( !_userDataLen )
    throw utilx::FieldNotSetException("N_UNITDATA_IND_Message::getUserData::: userData wasn't set or has length is equal to 0");
  return utilx::variable_data_t(_userData, _userDataLen);
}

uint32_t
N_UNITDATA_IND_Message::getLength() const
{
  return LibsccpMessage::getLength() +
    static_cast<uint32_t>(sizeof(_fieldsMask) + 
                          ( (_fieldsMask & SET_SEQUENCE_CONTROL) ? sizeof(_sequenceControl) : 0 ) +
                          sizeof(_calledAddrLen) + _calledAddrLen + sizeof(_callingAddrLen) + _callingAddrLen + 
                          sizeof(_userDataLen) + _userDataLen);
}

}}}
