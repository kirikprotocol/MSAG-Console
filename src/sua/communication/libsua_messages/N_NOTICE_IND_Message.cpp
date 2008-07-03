#include "N_NOTICE_IND_Message.hpp"

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace libsua_messages {

N_NOTICE_IND_Message::N_NOTICE_IND_Message()
  : LibsuaMessage(_MSG_CODE),
    _fieldsMask(0), _calledAddrLen(0), _callingAddrLen(0), _reasonForReturn(0),
    _userDataLen(0), _importance(0)
{}

N_NOTICE_IND_Message::N_NOTICE_IND_Message(const N_UNITDATA_REQ_Message& originalMessage, uint8_t reasonForReturn)
  : LibsuaMessage(_MSG_CODE), _fieldsMask(0), _importance(0)
{
  variable_data_t dataParam = originalMessage.getCalledAddress();
  setCalledAddress(dataParam.data, dataParam.dataLen);

  dataParam = originalMessage.getCallingAddress();
  setCallingAddress(dataParam.data, dataParam.dataLen);

  setReasonForReturn(reasonForReturn);

  dataParam = originalMessage.getUserData();
  setUserData(dataParam.data, dataParam.dataLen);

  try {
    setImportance(originalMessage.getImportance());
  } catch (utilx::FieldNotSetException& ex) {}
}

N_NOTICE_IND_Message::N_NOTICE_IND_Message(const sua_messages::CLDRMessage& cldrMessage)
  : LibsuaMessage(_MSG_CODE), _fieldsMask(0), _importance(0)
{
  const sua_messages::TLV_DestinationAddress& dstAddress = cldrMessage.getDestinationAddress();

  if ( dstAddress.getGlobalTitle().isSetValue() ) {
    if ( dstAddress.getSSN().isSetValue() ) {
      setCalledAddress(sua_messages::SCCPAddress(dstAddress.getGlobalTitle(), dstAddress.getSSN()));
    } else {
      setCalledAddress(sua_messages::SCCPAddress(dstAddress.getGlobalTitle()));
    }
  } else if ( dstAddress.getPointCode().isSetValue() && dstAddress.getSSN().isSetValue() ) {
    setCalledAddress(sua_messages::SCCPAddress(dstAddress.getPointCode(), dstAddress.getSSN()));
  } else {
    throw smsc::util::Exception("N_NOTICE_IND_Message::N_NOTICE_IND_Message::: wrong dstAddress in incoming CLDR message");
  }

  const sua_messages::TLV_SourceAddress& srcAddress = cldrMessage.getSourceAddress();

  if ( srcAddress.getGlobalTitle().isSetValue() ) {
    if ( srcAddress.getSSN().isSetValue() ) {
      setCallingAddress(sua_messages::SCCPAddress(srcAddress.getGlobalTitle(), srcAddress.getSSN()));
    } else {
      setCallingAddress(sua_messages::SCCPAddress(srcAddress.getGlobalTitle()));
    }
  } else if ( srcAddress.getPointCode().isSetValue() && srcAddress.getSSN().isSetValue() ) {
    setCallingAddress(sua_messages::SCCPAddress(srcAddress.getPointCode(), srcAddress.getSSN()));
  } else {
    throw smsc::util::Exception("N_NOTICE_IND_Message::N_NOTICE_IND_Message::: wrong srcAddress in incoming CLDR message");
  }

  const sua_messages::TLV_SCCP_Cause& sccpCause = cldrMessage.getSCCPCause();
  communication::return_cause_type_t sccpCauseType = sccpCause.getCauseType();

  if ( sccpCauseType != communication::RETURN_CAUSE )
    throw smsc::util::Exception("N_NOTICE_IND_Message::N_NOTICE_IND_Message::: invalid sccp cause type value = [%d] in incoming CLDR message", sccpCauseType);

  setReasonForReturn(sccpCause.getCauseValue());

  const sua_messages::TLV_Data& userData = cldrMessage.getData();
  setUserData(userData.getValue(), userData.getValueLength());
}

size_t
N_NOTICE_IND_Message::serialize(communication::TP* resultBuf) const
{
  size_t offset = LibsuaMessage::serialize(resultBuf);
  offset = communication::addField(resultBuf, offset, _fieldsMask);

  offset = communication::addField(resultBuf, offset, _calledAddrLen);
  offset = communication::addField(resultBuf, offset, _calledAddr, _calledAddrLen);

  offset = communication::addField(resultBuf, offset, _callingAddrLen);
  offset = communication::addField(resultBuf, offset, _callingAddr, _callingAddrLen);

  offset = communication::addField(resultBuf, offset, _reasonForReturn);

  offset = communication::addField(resultBuf, offset, _userDataLen);
  offset = communication::addField(resultBuf, offset, _userData, _userDataLen);

  if ( _fieldsMask & SET_IMPORTANCE )
    offset = communication::addField(resultBuf, offset, _importance);

  return offset;
}

size_t
N_NOTICE_IND_Message::deserialize(const communication::TP& packetBuf)
{
  size_t offset = LibsuaMessage::deserialize(packetBuf);
  offset = communication::extractField(packetBuf, offset, &_fieldsMask);

  offset = communication::extractField(packetBuf, offset, &_calledAddrLen);
  offset = communication::extractField(packetBuf, offset, _calledAddr, _calledAddrLen);

  offset = communication::extractField(packetBuf, offset, &_callingAddrLen);
  offset = communication::extractField(packetBuf, offset, _callingAddr, _callingAddrLen);

  offset = communication::extractField(packetBuf, offset, &_reasonForReturn);

  offset = communication::extractField(packetBuf, offset, &_userDataLen);
  offset = communication::extractField(packetBuf, offset, _userData, _userDataLen);

  if ( _fieldsMask & SET_IMPORTANCE )
    offset = communication::extractField(packetBuf, offset, &_importance);

  return offset;
}

std::string
N_NOTICE_IND_Message::toString() const
{
  std::string result(LibsuaMessage::toString());

  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), ",fieldsMask=[%02X]", _fieldsMask);
  result += strBuf;

  if ( _calledAddrLen )
    result += ",calledAddr=[" + hexdmp(_calledAddr, _calledAddrLen) + "]";

  if ( _callingAddrLen )
    result += ",callingAddr=[" + hexdmp(_callingAddr, _callingAddrLen) + "]";

  snprintf(strBuf, sizeof(strBuf), ",reasonForReturn=[%d]", _reasonForReturn);
  result += strBuf;

  if ( _userDataLen )
    result += ",userData=[" + hexdmp(_userData, _userDataLen) + "]";

  if ( _fieldsMask & SET_IMPORTANCE ) {
    snprintf(strBuf, sizeof(strBuf), ",importance=[%d]", _importance);
    result += strBuf;
  }

  return result;
}

const char*
N_NOTICE_IND_Message::getMsgCodeTextDescription() const
{
  return "N_NOTICE_IND_Message";
}

void
N_NOTICE_IND_Message::setCalledAddress(const uint8_t* address, uint8_t addressLen)
{
  _calledAddrLen = addressLen;
  memcpy(_calledAddr, address, addressLen);
}

void
N_NOTICE_IND_Message::setCalledAddress(const sua_messages::SCCPAddress& address)
{
  _calledAddrLen = address.serialize(_calledAddr, sizeof(_calledAddr));
}

variable_data_t
N_NOTICE_IND_Message::getCalledAddress() const
{
  if ( !_calledAddrLen )
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getCalledAddress::: calledAddress wasn't set or has length is equal to 0");
  return variable_data_t(_calledAddr, _calledAddrLen);
}

void
N_NOTICE_IND_Message::setCallingAddress(const uint8_t* address, uint8_t addressLen)
{
  _callingAddrLen = addressLen;
  memcpy(_callingAddr, address, addressLen);
}

void
N_NOTICE_IND_Message::setCallingAddress(const sua_messages::SCCPAddress& address)
{
  _callingAddrLen = address.serialize(_callingAddr, sizeof(_callingAddr));
}

variable_data_t
N_NOTICE_IND_Message::getCallingAddress() const
{
  if ( !_callingAddrLen )
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getCallingAddress::: callingAddress wasn't set or has length is equal to 0");
  return variable_data_t(_callingAddr, _callingAddrLen);
}

void
N_NOTICE_IND_Message::setReasonForReturn(uint8_t reasonForReturn)
{
  _reasonForReturn = reasonForReturn;
}

uint8_t
N_NOTICE_IND_Message::getReasonForReturn() const
{
  return _reasonForReturn;
}

void
N_NOTICE_IND_Message::setUserData(const uint8_t* data, uint8_t dataLen)
{
  _userDataLen = dataLen;
  memcpy(_userData, data, dataLen);
}

variable_data_t
N_NOTICE_IND_Message::getUserData() const
{
  if ( !_userDataLen )
    throw utilx::FieldNotSetException("N_NOTICE_IND_Message::getUserData::: userData wasn't set or has length is equal to 0");
  return variable_data_t(_userData, _userDataLen);
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
  return
    LibsuaMessage::getLength() + sizeof(_fieldsMask) + sizeof(_calledAddrLen) + _calledAddrLen +
    sizeof(_callingAddrLen) + _callingAddrLen + sizeof(_reasonForReturn) +
    sizeof(_userDataLen) + _userDataLen + 
    ( (_fieldsMask & SET_IMPORTANCE) ? sizeof(_importance) : 0 );
}

}
