#include "UDTS.hpp"
#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

UDTS::UDTS(const UDT& udt, common::return_cause_value_t cause)
: SCCPMessage(_MSG_CODE)
{
  setReturnCause(cause);
  setCalledAddress(udt.getCallingAddress());
  setCallingAddress(udt.getCalledAddress());
  const utilx::variable_data_t& msgData = udt.getData();
  setData(msgData.data, msgData.dataLen);
}

size_t
UDTS::serialize(common::TP* result_buf) const
{
  size_t offset = SCCPMessage::serialize(result_buf);
  if ( !_isSetReturnCause )
    throw utilx::SerializationException("UDTS::serialize::: mandatory field returnCause isn't set");
  offset = addMFixedField(result_buf, offset, _returnCause);

  if ( !_isSetCalledAddress )
    throw utilx::SerializationException("UDTS::serialize::: mandatory field calledAddress isn't set");
  size_t calledAddrPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  if ( !_isSetCallingAddress )
    throw utilx::SerializationException("UDTS::serialize::: mandatory field callingAddress isn't set");
  size_t callingAddrPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  if ( !_isSetData )
    throw utilx::SerializationException("UDTS::serialize::: mandatory field data isn't set");
  size_t dataPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  uint8_t tmpBuf[eyeline::sccp::SCCPAddress::_maxOctsLen];
  unsigned len = _calledAddress.pack2Octs(tmpBuf);
  if ( !len )
    throw utilx::SerializationException("UDTS::serialize::: can't serialize calledAddress");
  offset = addMVariablePartValue(result_buf, offset, calledAddrPtr, len, tmpBuf);

  len = _callingAddress.pack2Octs(tmpBuf);
  if ( !len )
    throw utilx::SerializationException("UDTS::serialize::: can't serialize callingAddress");
  offset = addMVariablePartValue(result_buf, offset, callingAddrPtr, len, tmpBuf);

  offset = addMVariablePartValue(result_buf, offset, dataPtr, _dataLen, _data);

  return offset;
}

size_t
UDTS::deserialize(const common::TP& packet_buf)
{
  size_t offset = SCCPMessage::deserialize(packet_buf);

  offset = extractMFixedField(packet_buf, offset, &_returnCause); _isSetReturnCause = true;
  uint8_t sccpFieldPtrValue;
  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("UDTS::deserialize::: zero value of pointer to called party address parameter");
  size_t calledAddrPtr = sccpFieldPtrValue - 1 + offset;

  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("UDTS::deserialize::: zero value of pointer to calling party address parameter");
  size_t callingAddrPtr = sccpFieldPtrValue - 1 + offset;

  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("UDTS::deserialize::: zero value of pointer to data parameter");
  size_t dataPtr = sccpFieldPtrValue - 1 + offset;

  while ( !_isSetCalledAddress || !_isSetCallingAddress || !_isSetData ) {
    if ( offset == calledAddrPtr || offset == callingAddrPtr ) {
      size_t beginOfAddrParam = offset;
      uint8_t fieldLen;
      offset = common::extractField(packet_buf, offset, &fieldLen);

      if ( fieldLen > eyeline::sccp::SCCPAddress::_maxOctsLen )
        throw utilx::DeserializationException("UDTS::deserialize::: %s address field is too long [%d octets]",
                                              (beginOfAddrParam == callingAddrPtr) ? "calling" : "called",
                                              fieldLen);

      uint8_t tmpBuf[eyeline::sccp::SCCPAddress::_maxOctsLen];
      offset = common::extractField(packet_buf, offset, tmpBuf, fieldLen);
      if ( beginOfAddrParam == calledAddrPtr ) {
        _calledAddress.unpackOcts(tmpBuf, fieldLen);
        _isSetCalledAddress = true;
      } else {
        _callingAddress.unpackOcts(tmpBuf, fieldLen);
        _isSetCallingAddress = true;
      }
    } else if ( offset == dataPtr  ) {
      offset = common::extractField(packet_buf, offset, &_dataLen);
      offset = common::extractField(packet_buf, offset, _data, _dataLen);
      _isSetData = true;
    }
  }
  return offset;
}

std::string
UDTS::toString() const
{
  char tmpBuf[64];
  sprintf(tmpBuf, "msgCode=%u", getMsgCode());

  std::string strBuf;
  if ( _isSetReturnCause ) {
    char retCauseStr[512];
    sprintf(retCauseStr, ",returnCause=%d (%s)", _returnCause,
           (_returnCause > common::MAX_CAUSE_VALUE ? "Reserved" :
                                                    common::returnCauseValueDesc[_returnCause]));
    strBuf += retCauseStr;
  }
  if ( _isSetCalledAddress )
    strBuf += ",calledAddress=" + std::string(_calledAddress.toString());
  if ( _isSetCallingAddress )
    strBuf += ",callingAddress=" + std::string(_callingAddress.toString());
  if ( _isSetData )
    strBuf += ",data=" + utilx::hexdmp(_data, _dataLen);

  return tmpBuf + strBuf;
}

}}}}}

