#include "XUDT.hpp"
#include "OptionalParameters.hpp"
#include "eyeline/utilx/hexdmp.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

size_t
XUDT::serialize(common::TP* result_buf) const
{
  size_t offset = SCCPMessage::serialize(result_buf);
  if ( !_isSetProtocolClass )
    throw utilx::SerializationException("XUDT::serialize::: mandatory field protocolClass isn't set");
  offset = addMFixedField(result_buf, offset, _protocolClass.getValue());

  if ( !_isSetHopCounter )
    throw utilx::SerializationException("XUDT::serialize::: mandatory field hopCounter isn't set");
  offset = addMFixedField(result_buf, offset, _hopCounter);

  if ( !_isSetCalledAddress )
    throw utilx::SerializationException("XUDT::serialize::: mandatory field calledAddress isn't set");
  size_t calledAddrPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  if ( !_isSetCallingAddress )
    throw utilx::SerializationException("XUDT::serialize::: mandatory field callingAddress isn't set");
  size_t callingAddrPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  if ( !_isSetData )
    throw utilx::SerializationException("XUDT::serialize::: mandatory field data isn't set");
  size_t dataPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  size_t optionalPartPtr = offset;
  offset = addOneOctetPointer(result_buf, offset);

  uint8_t tmpBuf[eyeline::sccp::SCCPAddress::_maxOctsLen];
  unsigned len = _calledAddress.pack2Octs(tmpBuf);
  if ( !len )
    throw utilx::SerializationException("XUDT::serialize::: can't serialize calledAddress");
  offset = addMVariablePartValue(result_buf, offset, calledAddrPtr, len, tmpBuf);

  len = _callingAddress.pack2Octs(tmpBuf);
  if ( !len )
    throw utilx::SerializationException("XUDT::serialize::: can't serialize callingAddress");
  offset = addMVariablePartValue(result_buf, offset, callingAddrPtr, len, tmpBuf);

  offset = addMVariablePartValue(result_buf, offset, dataPtr, _dataLen, _data);

  size_t beginOptionalPart = offset;
  if ( _isSetSegmentation )
    offset = addOptionalParameter(result_buf, offset, SEGMENTATION_PARAM_TAG,
                                  SEGMENTATION_VALUE_SIZE, _segmentation.getValue());

  if ( _isSetImportance )
    offset = addOptionalParameter(result_buf, offset, IMPORTANCE_PARAM_TAG,
                                  IMPORTANCE_VALUE_SIZE, &_importance);

  if ( _isSetSegmentation || _isSetImportance ) {
    updateOptinalPointer(result_buf, optionalPartPtr, beginOptionalPart);
    offset = addEndOfOptionalParams(result_buf, offset);
  }
  return offset;
}

size_t
XUDT::deserialize(const common::TP& packet_buf)
{
  size_t offset = SCCPMessage::deserialize(packet_buf);

  uint8_t protocolClassValue;
  offset = extractMFixedField(packet_buf, offset, &protocolClassValue);
  _protocolClass = ProtocolClass(protocolClassValue); _isSetProtocolClass = true;
  offset = extractMFixedField(packet_buf, offset, &_hopCounter); _isSetHopCounter = true;

  size_t optionalPartPtr = 0;
  offset = parseMandatoryVariablePart(packet_buf, offset, &optionalPartPtr);
  if ( optionalPartPtr )
    return parseOptionalPart(packet_buf, optionalPartPtr);
  else
    return offset;
}

size_t
XUDT::parseMandatoryVariablePart(const common::TP& packet_buf, size_t offset,
                                 size_t* optional_part_ptr)
{
  uint8_t sccpFieldPtrValue;
  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("XUDT::deserialize::: zero value of pointer to called party address parameter");
  size_t calledAddrPtr = sccpFieldPtrValue - 1 + offset;

  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("XUDT::deserialize::: zero value of pointer to calling party address parameter");
  size_t callingAddrPtr = sccpFieldPtrValue - 1 + offset;

  offset = extractOneOctetPointer(packet_buf, offset, &sccpFieldPtrValue);
  if ( !sccpFieldPtrValue )
    throw utilx::DeserializationException("XUDT::deserialize::: zero value of pointer to data parameter");
  size_t dataPtr = sccpFieldPtrValue - 1 + offset;

  uint8_t optionalPartPtr=0;
  *optional_part_ptr = 0;
  offset = extractOneOctetPointer(packet_buf, offset, &optionalPartPtr);
  if ( optionalPartPtr )
    *optional_part_ptr = offset - 1 + optionalPartPtr;

  while ( !_isSetCalledAddress || !_isSetCallingAddress || !_isSetData ) {
    if ( offset == calledAddrPtr || offset == callingAddrPtr ) {
      size_t beginOfAddrParam = offset;
      uint8_t fieldLen;
      offset = common::extractField(packet_buf, offset, &fieldLen);

      if ( fieldLen > eyeline::sccp::SCCPAddress::_maxOctsLen )
        throw utilx::DeserializationException("XUDT::parseMandatoryVariablePart::: %s address field is too long [%d octets]",
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

size_t
XUDT::parseOptionalPart(const common::TP& packet_buf, size_t offset)
{
  bool hasOptionalParam = true;
  while(hasOptionalParam) {
    uint8_t tag, valueLen;
    offset = extractParamName(packet_buf, offset, &tag);
    switch(tag) {
    case SEGMENTATION_PARAM_TAG:
      offset = extractLengthIndicator(packet_buf, offset, &valueLen);
      if ( valueLen != SEGMENTATION_VALUE_SIZE )
        throw utilx::DeserializationException("XUDT::parseOptionalPart::: wrong length value=%d for Segmentation parameter",
                                              valueLen);
      offset = _segmentation.deserialize(packet_buf, offset);
      _isSetSegmentation = true;
      break;
    case IMPORTANCE_PARAM_TAG:
      offset = extractLengthIndicator(packet_buf, offset, &valueLen);
      if ( valueLen != IMPORTANCE_VALUE_SIZE )
        throw utilx::DeserializationException("XUDT::parseOptionalPart::: wrong length value=%d for Segmentation parameter",
                                              valueLen);
      offset = common::extractField(packet_buf, offset, &_importance);
      _isSetImportance = true;
      break;
    case END_OPTIONAL_PARAM_TAG:
      hasOptionalParam = false;
      break;
    default:
      throw utilx::DeserializationException("XUDT::parseOptionalPart::: invalid parameter name=0x%02X", tag);
    }
  }
  return offset;
}

std::string
XUDT::toString() const
{
  char tmpBuf[64];
  sprintf(tmpBuf, "msgCode=%u", getMsgCode());

  std::string strBuf;
  if ( _isSetProtocolClass )
    strBuf += "," + _protocolClass.toString();
  if ( _isSetHopCounter ) {
    char hopCounterStr[32];
    sprintf(hopCounterStr, ",hopCounter=%u", _hopCounter);
    strBuf += hopCounterStr;
  }
  if ( _isSetCalledAddress )
    strBuf += ",calledAddress=" + std::string(_calledAddress.toString());
  if ( _isSetCallingAddress )
    strBuf += ",callingAddress=" + std::string(_callingAddress.toString());
  if ( _isSetData )
    strBuf += ",data=" + utilx::hexdmp(_data, _dataLen);
  if ( _isSetSegmentation )
    strBuf += ",segmentation=" + _segmentation.toString();
  if ( _isSetImportance ) {
    char importanceStr[32];
    sprintf(importanceStr, ",importance=%u", _importance);
    strBuf += importanceStr;
  }

  return tmpBuf + strBuf;
}

}}}}}
