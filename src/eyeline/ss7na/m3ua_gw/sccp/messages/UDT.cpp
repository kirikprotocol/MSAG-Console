#include "UDT.hpp"
#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

size_t
UDT::serialize(common::TP* result_buf) const
{
  size_t offset = SCCPMessage::serialize(result_buf);
  if ( !_isSetProtocolClass )
    throw utilx::SerializationException("UDT::serialize::: mandatory field protocolClass isn't set");
  offset = addMFixedField(result_buf, offset, _protocolClass.getValue());

  if ( !_isSetCalledAddress )
    throw utilx::SerializationException("UDT::serialize::: mandatory field calledAddress isn't set");
  size_t calledAddrPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  if ( !_isSetCallingAddress )
    throw utilx::SerializationException("UDT::serialize::: mandatory field callingAddress isn't set");
  size_t callingAddrPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  if ( !_isSetData )
    throw utilx::SerializationException("UDT::serialize::: mandatory field data isn't set");
  size_t dataPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  uint8_t tmpBuf[eyeline::sccp::SCCPAddress::_maxOctsLen];
  unsigned len = _calledAddress.pack2Octs(tmpBuf);
  if ( !len )
    throw utilx::SerializationException("UDT::serialize::: can't serialize calledAddress");
  offset = addMVariablePartValue(result_buf, offset, calledAddrPtr, len, tmpBuf);

  len = _callingAddress.pack2Octs(tmpBuf);
  if ( !len )
    throw utilx::SerializationException("UDT::serialize::: can't serialize callingAddress");
  offset = addMVariablePartValue(result_buf, offset, callingAddrPtr, len, tmpBuf);

  offset = addMVariablePartValue(result_buf, offset, dataPtr, _dataLen, _data);

  return offset;
}

size_t
UDT::deserialize(const common::TP& packet_buf)
{
  size_t offset = SCCPMessage::deserialize(packet_buf);

  uint8_t protocolClassValue;
  offset = extractMFixedField(packet_buf, offset, &protocolClassValue);
  _protocolClass = ProtocolClass(protocolClassValue); _isSetProtocolClass = true;
  uint8_t sccpFieldPtrValue;
  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("UDT::deserialize::: zero value of pointer to called party address parameter");
  size_t calledAddrPtr = sccpFieldPtrValue - 1 + offset;

  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("UDT::deserialize::: zero value of pointer to calling party address parameter");
  size_t callingAddrPtr = sccpFieldPtrValue - 1 + offset;

  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("UDT::deserialize::: zero value of pointer to data parameter");
  size_t dataPtr = sccpFieldPtrValue - 1 + offset;

  while ( !_isSetCalledAddress || !_isSetCallingAddress || !_isSetData ) {
    if ( offset == calledAddrPtr || offset == callingAddrPtr ) {
      size_t beginOfAddrParam = offset;
      uint8_t fieldLen;
      offset = common::extractField(packet_buf, offset, &fieldLen);

      if ( fieldLen > eyeline::sccp::SCCPAddress::_maxOctsLen )
        throw utilx::DeserializationException("UDT::deserialize::: %s address field is too long [%d octets]",
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
UDT::toString() const
{
  char tmpBuf[64];
  sprintf(tmpBuf, "msgCode=%u", getMsgCode());

  std::string strBuf;
  if ( _isSetProtocolClass )
    strBuf += "," + _protocolClass.toString();
  if ( _isSetCalledAddress )
    strBuf += ",calledAddress=" + std::string(_calledAddress.toString());
  if ( _isSetCallingAddress )
    strBuf += ",callingAddress=" + std::string(_callingAddress.toString());
  if ( _isSetData )
    strBuf += ",data=" + utilx::hexdmp(_data, _dataLen);

  return tmpBuf + strBuf;
}

}}}}}
