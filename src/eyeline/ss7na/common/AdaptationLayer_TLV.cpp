#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "AdaptationLayer_TLV.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "io_dispatcher/Exceptions.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

size_t
AdaptationLayer_TLV::serialize(TP* packet_buf,
                               size_t offset) const
{
  offset = addField(packet_buf, offset, _tag);
  return addField(packet_buf, offset, getActualLength());
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
TLV_IntegerPrimitive::serialize(TP* packet_buf,
                                size_t offset) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_IntegerPrimitive::serialize::: value isn't set");

  offset = AdaptationLayer_TLV::serialize(packet_buf, offset);
  return addField(packet_buf, offset, _value);
}

size_t
TLV_IntegerPrimitive::deserialize(const TP& packet_buf,
                                  size_t offset,
                                  uint16_t valLen)
{
  offset = extractField(packet_buf, offset, &_value);
  _isValueSet = true;
  return offset;
}

uint16_t
TLV_IntegerPrimitive::getLength() const
{
  return uint16_t(HEADER_SZ + sizeof(_value));
}

TLV_InfoString::TLV_InfoString()
  : TLV_StringPrimitive<255>(TAG)
{}

TLV_InfoString::TLV_InfoString(const std::string& str)
  : TLV_StringPrimitive<255>(TAG, str.c_str(), static_cast<uint16_t>(str.size()))
{}

std::string
TLV_InfoString::toString() const
{
  if ( isSetValue() ) {
    char strBuf[300];
    snprintf(strBuf, sizeof(strBuf), "infoString=[%s]", getPrintableValue().c_str());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_InfoString::toString::: value wasn't set");
}

TLV_RoutingContext::TLV_RoutingContext()
  : TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>(TAG), _numOfIndexes(0)
{}

TLV_RoutingContext::TLV_RoutingContext(uint32_t indexes[] , size_t num_of_indexes)
  : TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>(TAG, temporary_buf(indexes, num_of_indexes).valBuf.array, static_cast<uint16_t>(num_of_indexes*sizeof(uint32_t))), _numOfIndexes(num_of_indexes)
{}

TLV_RoutingContext::temporary_buf::temporary_buf(uint32_t indexes[] , size_t num_of_indexes) {
  size_t maxNumOfIndexes = MAX_OCTET_ARRAY_SIZE / sizeof(uint32_t);
  if ( num_of_indexes > maxNumOfIndexes )
    throw smsc::util::Exception("TLV_RoutingContext::TLV_RoutingContext::: num. of indexes in array=[%d] exceeded max. permitted value=[%d]", num_of_indexes, maxNumOfIndexes);
  for(size_t i=0; i<num_of_indexes; ++i)
    valBuf.arrayUint32[i] = htonl(indexes[i]);
}

size_t
TLV_RoutingContext::deserialize(const TP& packet_buf,
                                size_t offset /*position inside buffer where tag's data started*/,
                                uint16_t val_len)
{
  offset = TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>::deserialize(packet_buf, offset, val_len);
  _numOfIndexes = getValueLength() / sizeof(uint32_t);

  return offset;
}

std::string
TLV_RoutingContext::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];

    size_t pos = snprintf(strBuf, sizeof(strBuf), "routingContext=[");
    for(size_t i=0, maxNum=getMaxIndexNum(); i<maxNum; ++i) {
      int ret = snprintf(strBuf+pos, sizeof(strBuf)-pos, "idx_%d=%d,", i, getIndexValue(i));
      if ( ret < 0 ) break;
      pos += ret;
    }
    pos = strlen(strBuf);
    strBuf[pos-1] = ']'; strBuf[pos] = 0;

    return strBuf;
  } else
    throw smsc::util::Exception("TLV_RoutingContext::toString::: value wasn't set");
}

uint32_t
TLV_RoutingContext::getIndexValue(size_t idx_num) const
{
  if ( idx_num >= _numOfIndexes )
    throw smsc::util::Exception("TLV_RoutingContext::getIndexValue::: idxNum=[%d] exceeded max. possible value=[%d]", idx_num, _numOfIndexes);

  const uint8_t* value = getValue();
  return *(reinterpret_cast<const uint32_t*>(value + (idx_num << 2)));
}

size_t
TLV_RoutingContext::getMaxIndexNum() const
{
  return _numOfIndexes;
}

TLV_DiagnosticInformation::TLV_DiagnosticInformation()
  : TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>(TAG)
{}

TLV_DiagnosticInformation::TLV_DiagnosticInformation(const std::string& str)
  : TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>(TAG, reinterpret_cast<const uint8_t*>(str.c_str()), static_cast<uint16_t>(str.size()))
{}

std::string
TLV_DiagnosticInformation::toString() const
{
  if ( isSetValue() ) {
    char strBuf[MAX_OCTET_ARRAY_SIZE*2];
    snprintf(strBuf, sizeof(strBuf), "diagnosticInformation=[%s]", utilx::hexdmp(getValue(), getValueLength()).c_str());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_DiagnosticInformation::toString::: value wasn't set");
}

TLV_TrafficModeType::TLV_TrafficModeType()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_TrafficModeType::TLV_TrafficModeType(uint32_t traffic_mode)
  : TLV_IntegerPrimitive(TAG, traffic_mode)
{
  if ( traffic_mode > 3 || traffic_mode < 1 )
    throw smsc::util::Exception("TLV_TrafficModeType::TLV_TrafficModeType::: wrong trafficModeType value=[%d], expected value from set [1,2,3]", traffic_mode);
}

size_t
TLV_TrafficModeType::deserialize(const TP& packet_buf,
                                 size_t offset /*position inside buffer where tag's data started*/,
                                 uint16_t val_len)
{
  offset = TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
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
      return "trafficMode=[OVERRIDE]";
    if ( value == 2 )
      return "trafficMode=[LOADSHARE]";
    if ( value == 3 )
      return "trafficMode=[BROADCAST]";

    return "trafficMode=[UNKNOWN]";
  } else
    throw smsc::util::Exception("TLV_TrafficModeType::toString::: value wasn't set");
}

TLV_ErrorCode::TLV_ErrorCode()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_ErrorCode::TLV_ErrorCode(error_codes_e err_code)
  : TLV_IntegerPrimitive(TAG, err_code)
{}

size_t
TLV_ErrorCode::deserialize(const common::TP& packet_buf,
                           size_t offset /*position inside buffer where tag's data started*/,
                           uint16_t val_len)
{
  offset = TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
  uint32_t value  = getValue();
  if ( value > InvalidLoadsharingLabel || value == 0 )
    throw smsc::util::Exception("TLV_ErrorCode::deserialize::: wrong errCode value=[0x%x], expected value from range [0x01-0x%02X]", value, InvalidLoadsharingLabel);
  return offset;
}

std::string
TLV_ErrorCode::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "errCode=[0x%x] '%s'", getValue(), getErrorCodeDescription());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_ErrorCode::toString::: value wasn't set");
}

const char*
TLV_ErrorCode::_errCodeText[] = {
 "Not Used 0x00", "Invalid Version", "Not Used 0x02", "Unsupported Message Class",
 "Unsupported Message Type", "Unsupported Traffic Handling Mode", "Unexpected Message",
 "Protocol Error", "Not Used 0x08", "Invalid Stream Identifier", "Not Used 0x0a", "Not Used 0x0b", "Not Used 0x0c",
 "Refused - Management Blocking", "ASP Identifier Required", "Invalid ASP Identifier", "Not Used 0x10",
 "Invalid Parameter Value", "Parameter Field Error", "Unexpected Parameter",
 "Destination Status Unknown","Invalid Network Appearance", "Missing Parameter",
 "Not Used 0x17", "Not Used 0x18", "Invalid Routing Context", "No Configured AS for ASP",
 "Susbsystem Status Unknown", "Invalid Loadsharing Label"
};

const char*
TLV_ErrorCode::getErrorCodeDescription() const
{
  uint32_t value = getValue();
  if ( value > InvalidLoadsharingLabel )
    return "Unknown error code";
  else
    return _errCodeText[value];
}

TLV_ApplicationStatus::TLV_ApplicationStatus()
  : TLV_IntegerPrimitive(TAG) {}

TLV_ApplicationStatus::TLV_ApplicationStatus(uint16_t status_type, uint16_t status_id)
  : TLV_IntegerPrimitive(TAG, ((uint32_t)status_type << 16) | status_id) {}

std::string
TLV_ApplicationStatus::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "applicationStatus=[statusType=[%d],statusId=[%d]]", getStatusType(), getStatusId());
    return strBuf;
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

TLV_AspIdentifier::TLV_AspIdentifier(uint32_t asp_id)
  : TLV_IntegerPrimitive(TAG, asp_id)
{}

std::string
TLV_AspIdentifier::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "aspIdentifier=[%d]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_AspIdentifier::toString::: value wasn't set");
}

std::string
TLV_AffectedPointCode::toString() const
{
  if ( isSetValue() ) {
    const uint8_t* value = getValue();

    char strBuf[512];
    size_t pos = snprintf(strBuf, sizeof(strBuf), "affectedPointCodes=[");

    size_t nextPcOffset = 0, i=0, pcBufLen = getValueLength();
    while ( pcBufLen >= nextPcOffset + sizeof(uint32_t) ) {
      const uint8_t* pcValue = value + nextPcOffset;
      int ret = snprintf(strBuf+pos, sizeof(strBuf)-pos, "PC_%d={mask:%u,value:0x%x%x%x},", i++, pcValue[0], pcValue[1], pcValue[2], pcValue[3]);
      if ( ret < 0 ) break;
      pos += ret; nextPcOffset += sizeof(uint32_t);
    }
    strBuf[pos-1] = ']'; strBuf[pos] = 0;

    return strBuf;
  } else
    throw smsc::util::Exception("TLV_AffectedPointCode::toString::: value wasn't set");
}

bool
TLV_AffectedPointCode::getNextPC(ANSI_PC* point_code, size_t* next_pc_offset) const
{
  if ( getValueLength() >= *next_pc_offset + sizeof(point_code_t) ) {
    const uint8_t* value = getValue();
    value += *next_pc_offset;
    *next_pc_offset += sizeof(point_code_t);

    *point_code = ANSI_PC(value[0], value[1], value[2], value[3]);
    return true;
  } else
    return false;
}

bool
TLV_AffectedPointCode::getNextPC(ITU_PC* point_code, size_t* next_pc_offset) const
{
  if ( getValueLength() >= *next_pc_offset + sizeof(uint32_t) ) {
    const uint8_t* value = getValue();
    value += *next_pc_offset;
    *next_pc_offset += sizeof(uint32_t);

    *point_code = ITU_PC((uint32_t(value[0]) << 24) | (uint32_t(value[1]) << 16) |
                         (uint32_t(value[2]) << 8) | uint32_t(value[3]) );
    return true;
  } else
    return false;
}

TLV_CorrelationId::TLV_CorrelationId()
  : TLV_IntegerPrimitive(TAG)
{}

TLV_CorrelationId::TLV_CorrelationId(uint32_t correlation_id)
  : TLV_IntegerPrimitive(TAG, correlation_id)
{}

std::string
TLV_CorrelationId::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "correlationId=[%d]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_CorrelationId::toString::: value wasn't set");
}

}}}
