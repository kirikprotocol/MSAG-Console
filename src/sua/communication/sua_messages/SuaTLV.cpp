#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <sua/sua_layer/io_dispatcher/Exceptions.hpp>

#include <stdio.h>
//#include <logger/Logger.h>
#include "SuaTLV.hpp"

extern std::string
hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace sua_messages {

SuaTLVFactory::SuaTLVFactory()
  : _numsOfMandatoryTlvObjects(0)
{
  memset(reinterpret_cast<uint8_t*>(_optionalTlvTypeToTLVObject), 0, sizeof(_optionalTlvTypeToTLVObject));
  memset(reinterpret_cast<uint8_t*>(_mandatoryTlvTypeToTLVObject), 0, sizeof(_mandatoryTlvTypeToTLVObject));
}

void
SuaTLVFactory::registerExpectedOptionalTlv(SuaTLV* expectedTlv)
{
  _optionalTlvTypeToTLVObject[getTagIdx(expectedTlv->getTag())] = expectedTlv;
}

void
SuaTLVFactory::registerExpectedMandatoryTlv(SuaTLV* expectedTlv)
{
  _mandatoryTlvTypeToTLVObject[getTagIdx(expectedTlv->getTag())] = expectedTlv;
  ++_numsOfMandatoryTlvObjects;
}

void
SuaTLVFactory::setPositionTo4BytesBoundary(size_t* offset)
{
  int paddingTo4bytes = *offset & 0x03;
  if ( paddingTo4bytes ) // if tag length is not a multiple of 4 bytes
    *offset += 0x04 - paddingTo4bytes;
}

void
SuaTLVFactory::generateProtocolException()
{
  SuaTLV* tlvObj;
  int i=0, offset=0;
  char bufForTagsList[NUM_OF_TAGS*7+1]; // 7 bytes (including space for comma) for each tag presentation
  while (_numsOfMandatoryTlvObjects-- > 0) {
    while (i<NUM_OF_TAGS) {
      if ( (tlvObj = _mandatoryTlvTypeToTLVObject[i++] ) ) {
        offset=sprintf(bufForTagsList+offset, "0x%04X,",tlvObj->getTag()); break;
      }
    }
  }
  bufForTagsList[strlen(bufForTagsList)-1]=0;
  throw io_dispatcher::ProtocolException("SuaTLVFactory::generateProtocolException::: missing mandatory tlv(s) with next tag values [%s] when parsing input buffer", bufForTagsList);
}

size_t
SuaTLVFactory::parseInputBuffer(const communication::TP& packetBuf, size_t offset)
{
  uint16_t tag, valLen;

  while ( offset < packetBuf.packetLen ) {
    setPositionTo4BytesBoundary(&offset);

    if ( offset >= packetBuf.packetLen ) break;

    offset = communication::extractField(packetBuf, offset, &tag);
    offset = communication::extractField(packetBuf, offset, &valLen);

    valLen -= sizeof(tag) + sizeof(valLen);

    SuaTLV* suaTLVObject = NULL;
    if ( _numsOfMandatoryTlvObjects ) {
      suaTLVObject = _mandatoryTlvTypeToTLVObject[getTagIdx(tag)];
      if ( suaTLVObject ) {
        offset = suaTLVObject->deserialize(packetBuf, offset, valLen);
        _mandatoryTlvTypeToTLVObject[getTagIdx(tag)] = NULL;
        --_numsOfMandatoryTlvObjects; continue;
      }
    }
    suaTLVObject = _optionalTlvTypeToTLVObject[getTagIdx(tag)];
    if ( suaTLVObject ) {
      offset = suaTLVObject->deserialize(packetBuf, offset, valLen);
      _optionalTlvTypeToTLVObject[getTagIdx(tag)] = NULL;
    } else throw io_dispatcher::ProtocolException("parseInputBuffer::: unexpected or duplicate tlv with tag value [=0x%04X]", tag);
    
  }

  if ( _numsOfMandatoryTlvObjects )
    generateProtocolException();

  return offset;
}

unsigned int
SuaTLVFactory::getTagIdx(uint16_t tag)
{
  if ( tag < 0x0019 )
    return tag;
  else if ( tag > 0x0100 && tag < 0x0119 )
    return 0x0019 + tag - 0x0101;
  else if ( tag > 0x8000 && tag < 0x8007 )
    return 0x0019 + 0x0119 - 0x0100 + tag - 0x8001;
  else throw smsc::util::Exception("SuaTLVFactory::getTagIdx::: wrong tag value [=0x%04x]", tag);
}

size_t
SuaTLV::serialize(communication::TP* packetBuf,
                  size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  offset = communication::addField(packetBuf, offset, _tag);
  return communication::addField(packetBuf, offset, getLength());
}

uint32_t
TLV_IntegerPrimitive::getValue() const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_IntegerPrimitive::getValue::: value isn't set");

  return _value;
}

bool
TLV_IntegerPrimitive::isSetValue() const { return _isValueSet; }

size_t
TLV_IntegerPrimitive::serialize(communication::TP* packetBuf,
                                size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_IntegerPrimitive::serialize::: value isn't set");

  offset = SuaTLV::serialize(packetBuf, offset);
  return communication::addField(packetBuf, offset, _value);
}

size_t
TLV_IntegerPrimitive::deserialize(const communication::TP& packetBuf,
                                  size_t offset /*position inside buffer where tag's data started*/,
                                  uint16_t valLen)
{
  offset = communication::extractField(packetBuf, offset, &_value);
  _isValueSet = true;
  return offset;
}

uint16_t
TLV_IntegerPrimitive::getLength() const
{
  return HEADER_SZ + sizeof(_value);
}

template <size_t SZ>
size_t
TLV_StringPrimitive<SZ>::serialize(communication::TP* packetBuf,
                                   size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::serialize::: value isn't set");

  offset = SuaTLV::serialize(packetBuf, offset);
  return communication::addField(packetBuf, offset, reinterpret_cast<const uint8_t*>(_value), _valLen);
}

template <size_t SZ>
size_t
TLV_StringPrimitive<SZ>::deserialize(const communication::TP& packetBuf,
                                     size_t offset /*position inside buffer where tag's data started*/,
                                     uint16_t valLen)
{
  if ( valLen > sizeof(_value) )
    throw smsc::util::Exception("TLV_StringPrimitive::deserialize::: value length [=%d] exceeded max. allowable value [=%d]", valLen, SZ);

  offset = communication::extractField(packetBuf, offset, reinterpret_cast<uint8_t*>(_value), valLen);
  _valLen = valLen; _isValueSet = true;
  return offset;
}

template <size_t SZ>
int
TLV_StringPrimitive<SZ>::getUtf8Len(uint8_t firstUtf8Octet) const
{
  if ( firstUtf8Octet & 0x7f == firstUtf8Octet ) return 1;
  uint8_t mask = 0x80;
  int i = 0;
  while ( (mask & firstUtf8Octet) && i++ < 5) mask >>= 1;
  return i;
}

template <size_t SZ>
std::string
TLV_StringPrimitive<SZ>::getPrintableValue() const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::getValue::: value isn't set");

  std::string result;

  for (int i=0; i<_valLen;) {
    int nextSymbolLen = getUtf8Len(_value[i]);
    if ( nextSymbolLen == 1 ) {
      result += _value[i++];
    } else {
      char hexBuf[4];
      while ( nextSymbolLen-- > 0 ) {
        sprintf(hexBuf, "=%02X", uint8_t(_value[i++]));
        result.append(hexBuf);
      }
    }
  }
  
  return result;
}

template <size_t SZ>
bool
TLV_StringPrimitive<SZ>::isSetValue() const { return _isValueSet; }

template <size_t SZ>
uint16_t
TLV_StringPrimitive<SZ>::getLength() const
{
  return HEADER_SZ + _valLen;
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag)
  : SuaTLV(tag), _valLen(0), _isValueSet(false)
{
  memset(_valueBuffer._value, 0, sizeof(_valueBuffer));
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag, const uint8_t* val, uint16_t valLen)
  : SuaTLV(tag), _valLen(valLen), _isValueSet(true)
{
  if ( _valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive::: argument size [%d] is too large [max size=%d]", _valLen, sizeof(_valueBuffer));
  else
    memcpy(_valueBuffer._value, val, _valLen);
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag, size_t reservedOctetsOffset, const uint8_t* val, uint16_t valLen)
  : SuaTLV(tag), _valLen(reservedOctetsOffset+valLen), _isValueSet(true)
{
  if ( _valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive::: argument size [%d] is too large [max size=%d]", _valLen, sizeof(_valueBuffer));
  else {
    memset(_valueBuffer._value, 0, reservedOctetsOffset);
    memcpy(_valueBuffer._value + reservedOctetsOffset, val, valLen);
  }
}

template <size_t SZ>
size_t
TLV_OctetArrayPrimitive<SZ>::serialize(communication::TP* packetBuf,
                                   size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::serialize::: value isn't set");

  offset = SuaTLV::serialize(packetBuf, offset);
  return communication::addField(packetBuf, offset, _valueBuffer._value, _valLen);
}

template <size_t SZ>
size_t
TLV_OctetArrayPrimitive<SZ>::deserialize(const communication::TP& packetBuf,
                                     size_t offset /*position inside buffer where tag's data started*/,
                                     uint16_t valLen)
{
  if ( valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::deserialize::: value length [=%d] exceeded max. allowable value [=%d]", valLen, sizeof(_valueBuffer));

  offset = communication::extractField(packetBuf, offset, _valueBuffer._value, valLen);
  _valLen = valLen; _isValueSet = true;
  return offset;
}

template <size_t SZ>
const uint8_t*
TLV_OctetArrayPrimitive<SZ>::getValue() const {
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_OctetArrayPrimitive::getValue::: value isn't set");

  return _valueBuffer._value;
}

template <size_t SZ>
bool
TLV_OctetArrayPrimitive<SZ>::isSetValue() const { return _isValueSet; }

template <size_t SZ>
uint16_t
TLV_OctetArrayPrimitive<SZ>::getLength() const
{
  return HEADER_SZ + _valLen;
}

template <size_t SZ>
uint16_t
TLV_OctetArrayPrimitive<SZ>::getValueLength() const
{
  return _valLen;
}

TLV_ApplicationStatus::TLV_ApplicationStatus()
  : TLV_IntegerPrimitive(TAG) {}

TLV_ApplicationStatus::TLV_ApplicationStatus(uint16_t statusType, uint16_t statusId)
  : TLV_IntegerPrimitive(TAG, ((uint32_t)statusType << 16) | statusId) {}

std::string
TLV_ApplicationStatus::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "applicationStatus=[statusType=[%d],statusId=[%d]]", getStatusType(), getStatusId());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_ApplicationStatus::toString::: value wasn't set");
}

uint16_t
TLV_ApplicationStatus::getStatusType() const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_ApplicationStatus::getStatusType::: value isn't set");

  return (uint16_t)((getValue() >> 16) & 0x0000FFFF);
}

uint16_t
TLV_ApplicationStatus::getStatusId() const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_ApplicationStatus::getStatusId::: value isn't set");

  return (uint16_t)(getValue() & 0x0000FFFF);
}

TLV_AspIdentifier::TLV_AspIdentifier()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_AspIdentifier::TLV_AspIdentifier(uint32_t aspId)
  : TLV_IntegerPrimitive(TAG, aspId)
{}

std::string
TLV_AspIdentifier::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "aspIdentifier=[%d]", getValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_AspIdentifier::toString::: value wasn't set");
}

TLV_InfoString::TLV_InfoString()
  : TLV_StringPrimitive(TAG)
{}

TLV_InfoString::TLV_InfoString(const std::string& str)
  : TLV_StringPrimitive(TAG, str.c_str(), str.size())
{}

std::string
TLV_InfoString::toString() const
{
  if ( isSetValue() ) {
    char strBuf[300];
    snprintf(strBuf, sizeof(strBuf), "infoString=[%s]", getPrintableValue().c_str());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_InfoString::toString::: value wasn't set");
}

TLV_DiagnosticInformation::TLV_DiagnosticInformation()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_DiagnosticInformation::TLV_DiagnosticInformation(const std::string& str)
  : TLV_OctetArrayPrimitive(TAG, reinterpret_cast<const uint8_t*>(str.c_str()), str.size())
{}

std::string
TLV_DiagnosticInformation::toString() const
{
  if ( isSetValue() ) {
    char strBuf[MAX_OCTET_ARRAY_SIZE*2];
    snprintf(strBuf, sizeof(strBuf), "diagnosticInformation=[%s]",hexdmp(getValue(), getValueLength()).c_str());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_DiagnosticInformation::toString::: value wasn't set");
}

TLV_ErrorCode::TLV_ErrorCode()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_ErrorCode::TLV_ErrorCode(uint32_t errCode)
  : TLV_IntegerPrimitive(TAG, errCode)
{
  if ( errCode > 0x1c || errCode < 0x01 )
    throw smsc::util::Exception("TLV_ErrorCode::TLV_ErrorCode::: wrong errCode value=[%d], expected value from range [0x01-0x1c]", errCode);
}

size_t
TLV_ErrorCode::deserialize(const communication::TP& packetBuf,
                           size_t offset /*position inside buffer where tag's data started*/,
                           uint16_t valLen)
{
  offset = TLV_IntegerPrimitive::deserialize(packetBuf, offset, valLen);
  uint32_t value  = getValue();
  if ( value > 0x1c || value < 0x01 )
    throw smsc::util::Exception("TLV_ErrorCode::deserialize::: wrong errCode value=[%d], expected value from range [0x01-0x1c]", value);
  return offset;
}

std::string
TLV_ErrorCode::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "errCode=[%d] '%s'", getValue(), getErrorCodeDescription());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_ErrorCode::toString::: value wasn't set");
}

const char*
TLV_ErrorCode::getErrorCodeDescription() const
{
  uint32_t value = getValue();
  if ( value == 0x01 )
    return "Invalid Version";
  if ( value == 0x02 )
    return "Not Used in SUA";
  if ( value == 0x03 )
    return "Unsupported Message Class";
  if ( value == 0x04 )
    return "Unsupported Message Type";
  if ( value == 0x05 )
    return "Unsupported Traffic Handling Mode";
  if ( value == 0x06 )
    return "Unexpected Message";
  if ( value == 0x07 )
    return "Protocol Error";
  if ( value == 0x08 || value == 0x0a || 
       value == 0x0b || value == 0x0c ||
       value == 0x10 || value == 0x17 ||
       value == 0x18 )
    return "Not Used in SUA";
  if ( value == 0x09 )
    return "Invalid Stream Identifier";
  if ( value == 0x0d )
    return "Refused - Management Blocking";
  if ( value == 0x0e )
    return "ASP Identifier Required";
  if ( value == 0x0f )
    return "Invalid ASP Identifier";
  if ( value == 0x11 )
    return "Invalid Parameter Value";
  if ( value == 0x12 )
    return "Parameter Field Error";
  if ( value == 0x13 )
    return "Unexpected Parameter";
  if ( value == 0x14 )
    return "Destination Status Unknown";
  if ( value == 0x15 )
    return "Invalid Network Appearance";
  if ( value == 0x16 )
    return "Missing Parameter";
  if ( value == 0x19 )
    return "Invalid Routing Context";
  if ( value == 0x1a )
    return "No Configured AS for ASP";
  if ( value == 0x1b )
    return "Susbsystem Status Unknown";
  if ( value == 0x1c )
    return "Invalid Loadsharing Label";
}

TLV_TrafficModeType::TLV_TrafficModeType()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_TrafficModeType::TLV_TrafficModeType(uint32_t trafficMode)
  : TLV_IntegerPrimitive(TAG, trafficMode)
{
  if ( trafficMode > 3 || trafficMode < 1 )
    throw smsc::util::Exception("TLV_TrafficModeType::TLV_TrafficModeType::: wrong trafficModeType value=[%d], expected value from set [1,2,3]", trafficMode);
}

size_t
TLV_TrafficModeType::deserialize(const communication::TP& packetBuf,
                                 size_t offset /*position inside buffer where tag's data started*/,
                                 uint16_t valLen)
{
  offset = TLV_IntegerPrimitive::deserialize(packetBuf, offset, valLen);
  uint32_t value = getValue();
  if ( value > 3 || value < 1 )
    throw smsc::util::Exception("TLV_TrafficModeType::deserialize::: wrong trafficModeType value=[%d], expected value from set [1,2,3]", value);
  return offset;
}

std::string
TLV_TrafficModeType::toString() const
{
  if ( isSetValue() ) {
    uint32_t value = getValue();
    if ( value == 1 )
      return std::string("trafficMode=[OVERRIDE]");
    if ( value == 2 )
      return std::string("trafficMode=[LOADSHARE]");
    if ( value == 3 )
      return std::string("trafficMode=[BROADCAST]");
  } else
    throw smsc::util::Exception("TLV_TrafficModeType::toString::: value wasn't set");
}

TLV_DRNLabel::TLV_DRNLabel()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_DRNLabel::TLV_DRNLabel(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue)
  : TLV_OctetArrayPrimitive(TAG, temporary_buf(startLabelPosition, endLabelPosition, labelValue).array, sizeof(uint32_t))
{}

TLV_DRNLabel::temporary_buf::temporary_buf(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue)
{
  if ( startLabelPosition > 23 )
    throw smsc::util::Exception("TLV_DRNLabel::TLV_DRNLabel::: wrong startLabel parameter value=[%d], the value must be in range [0-23]", startLabelPosition);
  if ( endLabelPosition > 23 )
    throw smsc::util::Exception("TLV_DRNLabel::TLV_DRNLabel::: wrong endLabel parameter value=[%d], the value must be in range [0-23]", endLabelPosition);
  array[0] = startLabelPosition; array[1] = endLabelPosition;
  labelValue = htons(labelValue);
  memcpy(&array[2], reinterpret_cast<uint8_t*>(&labelValue), sizeof(labelValue));
}

size_t
TLV_DRNLabel::deserialize(const communication::TP& packetBuf,
                          size_t offset /*position inside buffer where tag's data started*/,
                          uint16_t valLen)
{
  offset = TLV_OctetArrayPrimitive::deserialize(packetBuf, offset, valLen);
  const uint8_t* value = getValue();

  if ( value[0] > 23 )
    throw smsc::util::Exception("TLV_DRNLabel::deserialize::: wrong startLabel parameter value=[%d], expected value from range [0-23]", value[0]);
  if ( value[1] > 23 )
    throw smsc::util::Exception("TLV_DRNLabel::deserialize::: wrong endLabel parameter value=[%d], expected value from range [0-23]", value[1]);

  return offset;
}

std::string
TLV_DRNLabel::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "DRNLabel=[startLabelPosition=[%d],endLabelPosition=[%d],labelValue=[%d]]", getStartLabelPosition(), getEndLabelPosition(), getLabelValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_DRNLabel::toString::: value wasn't set");
}

uint8_t
TLV_DRNLabel::getStartLabelPosition() const
{
  const uint8_t* value = getValue();
  return value[0];
}

uint8_t
TLV_DRNLabel::getEndLabelPosition() const
{
  const uint8_t* value = getValue();
  return value[1];
}

uint16_t
TLV_DRNLabel::getLabelValue() const
{
  const uint8_t* value = getValue();
  uint16_t label;
  memcpy(reinterpret_cast<uint8_t*>(&label), &value[2], sizeof(label));

  return ntohs(label);
}

TLV_TIDLabel::TLV_TIDLabel()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_TIDLabel::TLV_TIDLabel(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue)
  : TLV_OctetArrayPrimitive(TAG, temporary_buf(startLabelPosition, endLabelPosition, labelValue).array, sizeof(uint32_t))
{}

TLV_TIDLabel::temporary_buf::temporary_buf(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue)
{
  if ( startLabelPosition > 31 )
    throw smsc::util::Exception("TLV_TIDLabel::TLV_TIDLabel::: wrong startLabel parameter value=[%d], the value must be in range [0-31]", startLabelPosition);
  if ( endLabelPosition > 31 )
    throw smsc::util::Exception("TLV_TIDLabel::TLV_TIDLabel::: wrong endLabel parameter value=[%d], the value must be in range [0-31]", endLabelPosition);
  array[0] = startLabelPosition; array[1] = endLabelPosition;
  labelValue = htons(labelValue);
  memcpy(&array[2], reinterpret_cast<uint8_t*>(&labelValue), sizeof(labelValue));
}

size_t
TLV_TIDLabel::deserialize(const communication::TP& packetBuf,
                          size_t offset /*position inside buffer where tag's data started*/,
                          uint16_t valLen)
{
  offset = TLV_OctetArrayPrimitive::deserialize(packetBuf, offset, valLen);
  const uint8_t* value = getValue();

  if ( value[0] > 31 )
    throw smsc::util::Exception("TLV_DRNLabel::deserialize::: wrong startLabel parameter value=[%d], expected value from range [0-31]", value[0]);
  if ( value[1] > 31 )
    throw smsc::util::Exception("TLV_DRNLabel::deserialize::: wrong endLabel parameter value=[%d], expected value from range [0-31]", value[1]);

  return offset;
}

std::string
TLV_TIDLabel::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "TIDLabel=[startLabelPosition=[%d],endLabelPosition=[%d],labelValue=[%d]]", getStartLabelPosition(), getEndLabelPosition(), getLabelValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_TIDLabel::toString::: value wasn't set");
}


uint8_t
TLV_TIDLabel::getStartLabelPosition() const
{
  const uint8_t* value = getValue();
  return value[0];
}

uint8_t
TLV_TIDLabel::getEndLabelPosition() const
{
  const uint8_t* value = getValue();
  return value[1];
}

uint16_t
TLV_TIDLabel::getLabelValue() const
{
  const uint8_t* value = getValue();
  return ntohs(*(reinterpret_cast<const uint16_t*>(value + 2)));
}

TLV_RoutingContext::TLV_RoutingContext()
  : TLV_OctetArrayPrimitive(TAG), _numOfIndexes(0)
{}

TLV_RoutingContext::TLV_RoutingContext(uint32_t indexes[] , size_t numOfIndexes)
  : TLV_OctetArrayPrimitive(TAG, temporary_buf(indexes, numOfIndexes).valBuf.array, numOfIndexes*sizeof(uint32_t)), _numOfIndexes(numOfIndexes)
{}

TLV_RoutingContext::temporary_buf::temporary_buf(uint32_t indexes[] , size_t numOfIndexes) {
  size_t maxNumOfIndexes = MAX_OCTET_ARRAY_SIZE / sizeof(uint32_t);
  if ( numOfIndexes > maxNumOfIndexes )
    throw smsc::util::Exception("TLV_RoutingContext::TLV_RoutingContext::: num. of indexes in array=[%d] exceeded max. permitted value=[%d]", numOfIndexes, maxNumOfIndexes);
  for(int i=0; i<numOfIndexes; ++i)
    valBuf.arrayUint32[i] = htonl(indexes[i]);
}

size_t
TLV_RoutingContext::deserialize(const communication::TP& packetBuf,
                                size_t offset /*position inside buffer where tag's data started*/,
                                uint16_t valLen)
{
  offset = TLV_OctetArrayPrimitive::deserialize(packetBuf, offset, valLen);
  _numOfIndexes = getValueLength() / sizeof(uint32_t);

  return offset;
}

std::string
TLV_RoutingContext::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];

    int pos = snprintf(strBuf, sizeof(strBuf), "routingContext=[");
    for(size_t i=0, maxNum=getMaxIndexNum(); i<maxNum; ++i) {
      int ret = snprintf(strBuf+pos, sizeof(strBuf)-pos, "idx_%d=%d,", i, getIndexValue(i));
      if ( ret < 0 ) break;
      pos += ret;
    }
    pos = strlen(strBuf);
    strBuf[pos-1] = ']'; strBuf[pos] = 0;

    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_RoutingContext::toString::: value wasn't set");
}

uint32_t
TLV_RoutingContext::getIndexValue(size_t idxNum) const
{
  if ( idxNum >= _numOfIndexes )
    throw smsc::util::Exception("TLV_RoutingContext::getIndexValue::: idxNum=[%d] exceeded max. possible value=[%d]", idxNum, _numOfIndexes);

  const uint8_t* value = getValue();
  return *(reinterpret_cast<const uint32_t*>(value + (idxNum << 2)));
}

size_t TLV_RoutingContext::getMaxIndexNum() const
{
  return _numOfIndexes;
}

TLV_NetworkAppearance::TLV_NetworkAppearance()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_NetworkAppearance::TLV_NetworkAppearance(uint32_t networkAppearance)
  : TLV_IntegerPrimitive(TAG, networkAppearance)
{}

std::string
TLV_NetworkAppearance::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "networkAppearance=[%d]", getValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_NetworkAppearance::toString::: value wasn't set");
}

TLV_AffectedPointCode::TLV_AffectedPointCode()
  : TLV_OctetArrayPrimitive(TAG), _nextPCOffset(0)
 {}

TLV_AffectedPointCode::TLV_AffectedPointCode(const PointCode& pc)
  : TLV_OctetArrayPrimitive(TAG, pc.getValue(), PointCode::SIZE), 
    _nextPCOffset(0)
{}

std::string
TLV_AffectedPointCode::toString() const
{
  if ( isSetValue() ) {
    const uint8_t* value = getValue();
    
    char strBuf[256];
    int pos = snprintf(strBuf, sizeof(strBuf), "affectedPointCodes=[");

    size_t nextPcOffset = 0, i=0, pcBufLen = getValueLength();
    while ( pcBufLen > nextPcOffset + sizeof(uint32_t) ) {
      int ret = snprintf(strBuf+pos, sizeof(strBuf)-pos, "PC_%d=%s,", i++, hexdmp(value + nextPcOffset, sizeof(uint32_t)).c_str());
      if ( ret < 0 ) break;
      pos += ret; nextPcOffset += sizeof(uint32_t);
    }
    strBuf[pos-1] = ']'; strBuf[pos] = 0;

    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_AffectedPointCode::toString::: value wasn't set");
}

ANSI_PC
TLV_AffectedPointCode::getNext_ANSI_PC()
{
  if ( getValueLength() >= _nextPCOffset + sizeof(uint32_t) ) {
    const uint8_t* value = getValue();
    value += _nextPCOffset;
    _nextPCOffset += sizeof(uint32_t);

    return ANSI_PC(value);
  } else
    throw smsc::util::Exception("TLV_AffectedPointCode::getNext_ANSI_PC::: no more point codes");
}

ITU_PC
TLV_AffectedPointCode::getNext_ITU_PC()
{
  if ( getValueLength() >= _nextPCOffset + sizeof(uint32_t) ) {
    const uint8_t* value = getValue();
    value += _nextPCOffset;
    _nextPCOffset += sizeof(uint32_t);

    return ITU_PC(value);
  } else
    throw smsc::util::Exception("TLV_AffectedPointCode::getNext_ITU_PC:::  no more point codes");
}

TLV_SSN::TLV_SSN()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_SSN::TLV_SSN(uint8_t ssnValue)
  : TLV_IntegerPrimitive(TAG, ssnValue)
{}

std::string
TLV_SSN::toString() const
{
  if ( isSetValue() ) {
    char strBuf[64];
    ssn_t ssn = static_cast<ssn_t>(getValue() & 0xFF);
    snprintf(strBuf, sizeof(strBuf), "SSN=[%d]", ssn);
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_NetworkAppearance::toString::: value wasn't set");
}

TLV_SMI::TLV_SMI()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_SMI::TLV_SMI(uint8_t smi)
  : TLV_IntegerPrimitive(TAG, smi)
{
  if ( smi > 0x04 && smi != 0xff )
    throw smsc::util::Exception("TLV_SMI::TLV_SMI::: wrong smi parameter value=[%d], expected value from range [0x00-0x04,0xff]", smi);
}

size_t
TLV_SMI::deserialize(const communication::TP& packetBuf,
                     size_t offset /*position inside buffer where tag's data started*/,
                     uint16_t valLen)
{
  offset = TLV_IntegerPrimitive::deserialize(packetBuf, offset, valLen);
  uint8_t value = 0xFF & getValue();

  if ( value > 0x04 && value != 0xff )
    throw smsc::util::Exception("TLV_SMI::deserialize::: wrong smi parameter value=[%d], expected value from range [0x00-0x04,0xff]", value);

  return offset;
}

std::string
TLV_SMI::toString() const
{
  if ( isSetValue() ) {
    uint8_t value = 0xFF & getValue();
    if ( value == 0x00 )
      return std::string("SMI=[Reserved/Unknown]");
    if ( value == 0x01 )
      return std::string("SMI=[Solitary]");
    if ( value == 0x02 )
      return std::string("SMI=[Duplicated]");
    if ( value == 0x03 )
      return std::string("SMI=[Triplicated]");
    if ( value == 0xff )
      return std::string("SMI=[Unspecified]");
  } else
    throw smsc::util::Exception("TLV_SMI::toString::: value wasn't set");
}

TLV_UserCause::TLV_UserCause()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_UserCause::TLV_UserCause(uint16_t cause, uint16_t user)
  : TLV_IntegerPrimitive(TAG, ((uint32_t)cause << 16) | user)
{}

size_t
TLV_UserCause::deserialize(const communication::TP& packetBuf,
                           size_t offset /*position inside buffer where tag's data started*/,
                           uint16_t valLen)
{
  offset = TLV_IntegerPrimitive::deserialize(packetBuf, offset, valLen);
  uint16_t cause = getCause();
  if ( cause > 0x1c || cause < 0x01 )
    throw smsc::util::Exception("TLV_UserCause::deserialize::: wrong cause value=[%d], expected value from range [0x00-0x02]", cause);
  return offset;
}

std::string
TLV_UserCause::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "user_cause=[cause=[%d],user=[%d]]", getCause(), getUser());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_UserCause::toString::: value wasn't set");
}

uint16_t
TLV_UserCause::getUser() const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_UserCause::getUser::: value isn't set");

  return (uint16_t)(getValue() & 0x0000FFFF);
}

uint16_t
TLV_UserCause::getCause() const
{
  if ( !isSetValue() )
    throw utilx::FieldNotSetException("TLV_UserCause::getCause::: value isn't set");

  return (uint16_t)((getValue() >> 16) & 0x0000FFFF);

}

TLV_CongestionLevel::TLV_CongestionLevel()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_CongestionLevel::TLV_CongestionLevel(uint8_t congestionLevel)
  : TLV_IntegerPrimitive(TAG, congestionLevel)
{}

std::string
TLV_CongestionLevel::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "congestionLevel=[%d]", getValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_CongestionLevel::toString::: value wasn't set");
}

TLV_ProtocolClass::TLV_ProtocolClass()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_ProtocolClass::TLV_ProtocolClass(const ProtocolClass& protocolClass)
  : TLV_IntegerPrimitive(TAG, protocolClass)
{}

std::string
TLV_ProtocolClass::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "protocolClass=[%d:%s]", getValue(), ProtocolClass(getValue()).toString().c_str());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_ProtocolClass::toString::: value wasn't set");
}

TLV_SequenceControl::TLV_SequenceControl()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_SequenceControl::TLV_SequenceControl(uint32_t sequenceControl)
  : TLV_IntegerPrimitive(TAG, sequenceControl)
{}

std::string
TLV_SequenceControl::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "sequenceControl=[%d]", getValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_SequenceControl::toString::: value wasn't set");
}

TLV_Segmentation::TLV_Segmentation()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_Segmentation::TLV_Segmentation(bool isFirstSegment, uint8_t numOfRemainingSegments, uint32_t reference)
  : TLV_OctetArrayPrimitive(TAG, temporary_buf(isFirstSegment, numOfRemainingSegments, reference).array, sizeof(uint32_t))
{}

TLV_Segmentation::temporary_buf::temporary_buf(bool isFirstSegment, uint8_t numOfRemainingSegments, uint32_t reference)
{
  array[0] = numOfRemainingSegments & 0x7F;
  if ( isFirstSegment ) array[0] |= 0x80;
  reference = htonl(reference);
  memcpy(&array[1], reinterpret_cast<uint8_t*>(&reference) + 1, sizeof(reference) - 1); // copy low 3 bytes
}

std::string
TLV_Segmentation::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "firstSegmentIndication=[%d],numOfRemainingSegments=[%d],refValue=[%d]", isFirstSegment(), getNumOfRemainingSegments(), getReferenceValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_Segmentation::toString::: value wasn't set");
}

bool
TLV_Segmentation::isFirstSegment() const
{
  const uint8_t* value = getValue();
  return value[0] & 0x80;
}

uint8_t
TLV_Segmentation::getNumOfRemainingSegments() const
{
  const uint8_t* value = getValue();
  return value[0] & 0x7F;
}

uint32_t
TLV_Segmentation::getReferenceValue() const
{
  const uint8_t* value = getValue();
  uint32_t referenceValue=0;
  memcpy(reinterpret_cast<uint8_t*>(&referenceValue) + 1, &value[1], 3);

  return ntohl(referenceValue);
}

TLV_SS7HopCount::TLV_SS7HopCount()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_SS7HopCount::TLV_SS7HopCount(uint8_t hopCount)
  : TLV_IntegerPrimitive(TAG, hopCount)
{}

std::string
TLV_SS7HopCount::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "ss7HopCount=[%d]", getHopCountValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_SS7HopCount::toString::: value wasn't set");
}

uint8_t
TLV_SS7HopCount::getHopCountValue() const
{
  return static_cast<uint8_t>(getValue() & 0x000000FF);
}

TLV_Importance::TLV_Importance()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_Importance::TLV_Importance(uint8_t importance)
  : TLV_IntegerPrimitive(TAG, importance)
{}

size_t
TLV_Importance::deserialize(const communication::TP& packetBuf,
                            size_t offset /*position inside buffer where tag's data started*/,
                            uint16_t valLen)
{
  offset = TLV_IntegerPrimitive::deserialize(packetBuf, offset, valLen);
  uint8_t importanceValue = getImportanceValue();
  if ( importanceValue > 7 )
    throw smsc::util::Exception("TLV_Importance::deserialize::: wrong importance value=[%d], expected value from range [0x00-0x07]", importanceValue);
  return offset;
}

std::string
TLV_Importance::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "importance=[%d]", getImportanceValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_Importance::toString::: value wasn't set");
}

uint8_t
TLV_Importance::getImportanceValue() const
{
  return static_cast<uint8_t>(getValue() & 0xff);
}

TLV_MessagePriority::TLV_MessagePriority()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_MessagePriority::TLV_MessagePriority(uint8_t priority)
  : TLV_IntegerPrimitive(TAG, priority)
{
  if ( priority > 0x03 && priority != 0xFF )
    throw smsc::util::Exception("TLV_MessagePriority::TLV_MessagePriority::: wrong messagePriority value=[%d], expected value from set [0x00-0x03,0xFF]", priority);
}

size_t
TLV_MessagePriority::deserialize(const communication::TP& packetBuf,
                                 size_t offset /*position inside buffer where tag's data started*/,
                                 uint16_t valLen)
{
  offset = TLV_IntegerPrimitive::deserialize(packetBuf, offset, valLen);
  uint8_t priority = getMessagePriorityValue();
  if ( priority > 3 && priority != 0xFF )
    throw smsc::util::Exception("TLV_MessagePriority::deserialize::: wrong priority value=[%d], expected value from set [0x00-0x03,0xFF]", priority);
  return offset;
}

std::string
TLV_MessagePriority::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "priority=[%d]", getMessagePriorityValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_MessagePriority::toString::: value wasn't set");
}

uint8_t
TLV_MessagePriority::getMessagePriorityValue() const
{
  return static_cast<uint8_t>(getValue() & 0xff);
}

TLV_CorrelationId::TLV_CorrelationId()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_CorrelationId::TLV_CorrelationId(uint32_t correlationId)
  : TLV_IntegerPrimitive(TAG, correlationId)
{}

std::string
TLV_CorrelationId::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "correlationId=[%d]", getValue());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_CorrelationId::toString::: value wasn't set");
}

TLV_Data::TLV_Data()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_Data::TLV_Data(const uint8_t* val, uint16_t len)
  : TLV_OctetArrayPrimitive(TAG, val, len)
{}

std::string
TLV_Data::toString() const
{
  if ( isSetValue() ) {
    char strBuf[MAX_OCTET_ARRAY_SIZE*2];
    snprintf(strBuf, sizeof(strBuf), "data=[%s]",hexdmp(getValue(), getValueLength()).c_str());
    return std::string(strBuf);
  } else
    throw smsc::util::Exception("TLV_Data::toString::: value wasn't set");
}

TLV_PointCode::TLV_PointCode()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_PointCode::TLV_PointCode(const ANSI_PC& pc)
  : TLV_OctetArrayPrimitive(TAG, pc.getValue(), PointCode::SIZE)
{}

TLV_PointCode::TLV_PointCode(const ITU_PC& pc)
  : TLV_OctetArrayPrimitive(TAG, pc.getValue(), PointCode::SIZE)
{}

std::string
TLV_PointCode::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), "PC=%s", hexdmp(getValue(), getValueLength()).c_str());
  return std::string(strBuf);
}

ANSI_PC
TLV_PointCode::get_ANSI_PC()
{
  return ANSI_PC(getValue());
}

ITU_PC
TLV_PointCode::get_ITU_PC()
{
  return ITU_PC(getValue());
}

TLV_GlobalTitle::TLV_GlobalTitle()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_GlobalTitle::TLV_GlobalTitle(const GlobalTitle& gt)
  : TLV_OctetArrayPrimitive(TAG, RESERVED_OCTETS, gt.getValue(), gt.getValueSz()), _globalTitle(gt)
{}

size_t
TLV_GlobalTitle::deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen)
{
  offset = TLV_OctetArrayPrimitive::deserialize(packetBuf, offset, valLen);
  _globalTitle = GlobalTitle(getValue() + RESERVED_OCTETS, getLength() - RESERVED_OCTETS);
  uint8_t gti = _globalTitle.getGTI();
  if ( gti > GlobalTitle::GT_INCLUDE_FULL_INFO )
    throw smsc::util::Exception("TLV_GlobalTitle::deserialize::: wrong gti value=[%d], expected value from range [0x00-0x04]", gti);
  return offset;
}

std::string
TLV_GlobalTitle::toString() const
{
  return _globalTitle.toString();
}

const GlobalTitle&
TLV_GlobalTitle::getGlobalTitle() const
{
  return _globalTitle;
}

TLV_Address::TLV_Address(uint16_t tag)
  : SuaTLV(tag), _routingIndicator(0), _addressIndicator(0), _isValueSet(false)
{}

TLV_Address::TLV_Address(uint16_t tag, const TLV_PointCode& pointCode, const TLV_SSN& ssn)
  : SuaTLV(tag), _routingIndicator(ROUTE_ON_SSN_PLUS_PC), _addressIndicator(0),
    _pointCode(pointCode), _ssn(ssn), _isValueSet(true)
{}

TLV_Address::TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt, const TLV_SSN& ssn)
  : SuaTLV(tag), _routingIndicator(ROUTE_ON_GT),
    _addressIndicator(GT_INCLUDE_INDICATION|SSN_INCLUDE_INDICATION), _gt(gt), _ssn(ssn), _isValueSet(true)
{}

TLV_Address::TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt)
  : SuaTLV(tag), _routingIndicator(ROUTE_ON_GT), _addressIndicator(GT_INCLUDE_INDICATION), _gt(gt), _isValueSet(true)
{}

size_t
TLV_Address::serialize(communication::TP* packetBuf,
                       size_t offset /*position inside buffer where tag's data will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_Address::serialize::: value isn't set");

  offset = SuaTLV::serialize(packetBuf, offset);

  offset = communication::addField(packetBuf, offset, _routingIndicator);
  offset = communication::addField(packetBuf, offset, _addressIndicator);

  if ( _pointCode.isSetValue() )
    offset = _pointCode.serialize(packetBuf, offset);
  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(packetBuf, offset);
  if ( _gt.isSetValue() )
    offset = _gt.serialize(packetBuf, offset);

  int paddingTo4bytes = offset & 0x03;
  if ( paddingTo4bytes ) { // if tag length is not a multiple of 4 bytes
    uint8_t padding[4]={0};
    offset = communication::addField(packetBuf, offset, padding, 0x04 - paddingTo4bytes);
  }
  return offset;
}

size_t
TLV_Address::deserialize(const communication::TP& packetBuf,
                         size_t offset /*position inside buffer where tag's data started*/,
                         uint16_t valLen)
{
  offset = communication::extractField(packetBuf, offset, &_routingIndicator);
  offset = communication::extractField(packetBuf, offset, &_addressIndicator);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_pointCode);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedOptionalTlv(&_gt);

  communication::TP tmpPacketBuf (packetBuf.packetType, valLen - sizeof(_addressIndicator) - sizeof(_addressIndicator), packetBuf.packetBody + offset);

  offset += tlvFactory.parseInputBuffer(tmpPacketBuf, 0);

  _isValueSet = true;

  return offset;
}

uint16_t
TLV_Address::getLength() const
{
  uint16_t sz = HEADER_SZ + INDICATORS_FIELDS_SZ;
  if ( _pointCode.isSetValue() )
    sz += _pointCode.getLength();
  if ( _ssn.isSetValue() )
    sz += _ssn.getLength();
  if ( _gt.isSetValue() )
    sz += _gt.getLength();

  int paddingTo4bytesSz = sz & 0x03;
  if ( paddingTo4bytesSz )
    sz += 0x04 - paddingTo4bytesSz;

  return sz;
}

std::string
TLV_Address::toString() const
{
  std::string strBuf;
  if ( _pointCode.isSetValue() )
    strBuf = _pointCode.toString() + ",";
  if ( _ssn.isSetValue() )
    strBuf += _ssn.toString() + ",";
  if ( _gt.isSetValue() )
    strBuf += _gt.toString() + ",";

  strBuf.erase(strBuf.length() - 1); 

  return strBuf;
}

uint16_t
TLV_Address::getRoutingIndicator() const
{
  return _routingIndicator;
}

uint16_t
TLV_Address::getAddressIndicator() const
{
  return _addressIndicator;
}

void
TLV_Address::setAddressIndicator(uint16_t addrIndicator)
{
  _addressIndicator = addrIndicator;
}

const TLV_PointCode&
TLV_Address::getPointCode() const
{
  return _pointCode;
}

const TLV_SSN&
TLV_Address::getSSN() const
{
  return _ssn;
}

const TLV_GlobalTitle&
TLV_Address::getGlobalTitle() const
{
  return _gt;
}

bool
TLV_Address::isSetValue() const
{
  return _isValueSet;
}

TLV_SourceAddress::TLV_SourceAddress()
  : TLV_Address(TAG)
{}

TLV_SourceAddress::TLV_SourceAddress(const TLV_PointCode& pointCode, const TLV_SSN& ssn)
  : TLV_Address(TAG, pointCode, ssn)
{}

TLV_SourceAddress::TLV_SourceAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn)
  : TLV_Address(TAG, gt, ssn)
{}

TLV_SourceAddress::TLV_SourceAddress(const TLV_GlobalTitle& gt)
  : TLV_Address(TAG, gt)
{}

std::string
TLV_SourceAddress::toString() const
{
  std::string strBuf("sourceAddress=[");
  return strBuf + TLV_Address::toString() + "]";
}

TLV_DestinationAddress::TLV_DestinationAddress()
  : TLV_Address(TAG)
{}

TLV_DestinationAddress::TLV_DestinationAddress(const TLV_PointCode& pointCode, const TLV_SSN& ssn)
  : TLV_Address(TAG, pointCode, ssn)
{}

TLV_DestinationAddress::TLV_DestinationAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn)
  : TLV_Address(TAG, gt, ssn)
{}

TLV_DestinationAddress::TLV_DestinationAddress(const TLV_GlobalTitle& gt)
  : TLV_Address(TAG, gt)
{}

std::string
TLV_DestinationAddress::toString() const
{
  std::string strBuf("destinationAddress=[");
  return strBuf + TLV_Address::toString() + "]";
}

TLV_SCCP_Cause::TLV_SCCP_Cause()
  : TLV_OctetArrayPrimitive(TAG)
{}

TLV_SCCP_Cause::TLV_SCCP_Cause(communication::return_cause_type_t causeType, uint8_t causeValue)
  : TLV_OctetArrayPrimitive(TAG, temporary_buf(uint8_t(causeType), causeValue).array, sizeof(uint32_t))
{}

size_t
TLV_SCCP_Cause::deserialize(const communication::TP& packetBuf,
                            size_t offset /*position inside buffer where tag's data started*/,
                            uint16_t valLen)
{
  offset = TLV_OctetArrayPrimitive::deserialize(packetBuf, offset, valLen);
  const uint8_t* value = getValue();
  if ( value[2] == 0 || 
       value[2] > communication::ERROR_CAUSE )
    throw smsc::util::Exception("TLV_SCCP_Cause::deserialize::: wrong cause type value=[%d], expected value from range [0x01-0x05]", value[2]);
  return offset;
}

std::string
TLV_SCCP_Cause::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "SCCP_Cause=[causeType=[%d],causeValue=[%d]]", getCauseType(), getCauseValue());
    return std::string(strBuf);
  } else
    return "";
}

communication::return_cause_type_t
TLV_SCCP_Cause::getCauseType() const
{
  const uint8_t* value = getValue();
  return communication::return_cause_type_t(value[2]);
}

uint8_t
TLV_SCCP_Cause::getCauseValue() const
{
  const uint8_t* value = getValue();
  return value[3];
}

TLV_SCCP_Cause::temporary_buf::temporary_buf(uint8_t causeType, uint8_t causeValue)
{
  array[0] = array[1] = 0;

  if ( causeType == 0 || 
       causeType > communication::ERROR_CAUSE )
    throw smsc::util::Exception("TLV_SCCP_Cause::temporary_buf::: wrong cause type value=[%d], expected value from range [0x01-0x05]", causeType);
  array[2] = causeType; array[3] = causeValue;
}

}
