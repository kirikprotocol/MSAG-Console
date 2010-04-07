#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"

#include "SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

TLV_DRNLabel::TLV_DRNLabel()
  : TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG)
{}

TLV_DRNLabel::TLV_DRNLabel(uint8_t start_label_position,
                           uint8_t end_label_position, uint16_t label_value)
  : TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG, temporary_buf(start_label_position,
                                                                 end_label_position,
                                                                 label_value).array, static_cast<uint16_t>(sizeof(uint32_t)))
{}

TLV_DRNLabel::temporary_buf::temporary_buf(uint8_t start_label_position,
                                           uint8_t end_label_position,
                                           uint16_t label_value)
{
  if ( start_label_position > 23 )
    throw smsc::util::Exception("TLV_DRNLabel::TLV_DRNLabel::: wrong startLabel parameter value=[%d], the value must be in range [0-23]", start_label_position);
  if ( end_label_position > 23 )
    throw smsc::util::Exception("TLV_DRNLabel::TLV_DRNLabel::: wrong endLabel parameter value=[%d], the value must be in range [0-23]", end_label_position);
  array[0] = start_label_position; array[1] = end_label_position;
  label_value = htons(label_value);
  memcpy(&array[2], reinterpret_cast<uint8_t*>(&label_value), sizeof(label_value));
}

size_t
TLV_DRNLabel::deserialize(const common::TP& packet_buf,
                          size_t offset /*position inside buffer where tag's data started*/,
                          uint16_t val_len)
{
  offset = common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>::deserialize(packet_buf, offset, val_len);
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
    return strBuf;
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
  : TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG)
{}

TLV_TIDLabel::TLV_TIDLabel(uint8_t start_label_position,
                           uint8_t end_label_position, uint16_t label_value)
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG,
                                                      temporary_buf(start_label_position,
                                                                    end_label_position,
                                                                    label_value).array,
                                                      static_cast<uint16_t>(sizeof(uint32_t)))
{}

TLV_TIDLabel::temporary_buf::temporary_buf(uint8_t start_label_position,
                                           uint8_t end_label_position,
                                           uint16_t label_value)
{
  if ( start_label_position > 31 )
    throw smsc::util::Exception("TLV_TIDLabel::TLV_TIDLabel::: wrong startLabel parameter value=[%d], the value must be in range [0-31]", start_label_position);
  if ( end_label_position > 31 )
    throw smsc::util::Exception("TLV_TIDLabel::TLV_TIDLabel::: wrong endLabel parameter value=[%d], the value must be in range [0-31]", end_label_position);
  array[0] = start_label_position; array[1] = end_label_position;
  label_value = htons(label_value);
  memcpy(&array[2], reinterpret_cast<uint8_t*>(&label_value), sizeof(label_value));
}

size_t
TLV_TIDLabel::deserialize(const common::TP& packet_buf,
                          size_t offset /*position inside buffer where tag's data started*/,
                          uint16_t val_len)
{
  offset = common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>::deserialize(packet_buf, offset, val_len);
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
    return strBuf;
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

TLV_NetworkAppearance::TLV_NetworkAppearance()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_NetworkAppearance::TLV_NetworkAppearance(uint32_t network_appearance)
  : common::TLV_IntegerPrimitive(TAG, network_appearance)
{}

std::string
TLV_NetworkAppearance::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "networkAppearance=[%d]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_NetworkAppearance::toString::: value wasn't set");
}

TLV_SSN::TLV_SSN()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_SSN::TLV_SSN(uint8_t ssn_value)
  : common::TLV_IntegerPrimitive(TAG, ssn_value)
{}

std::string
TLV_SSN::toString() const
{
  if ( isSetValue() ) {
    char strBuf[64];
    ssn_t ssn = static_cast<ssn_t>(getValue() & 0xFF);
    snprintf(strBuf, sizeof(strBuf), "SSN=[%d]", ssn);
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_NetworkAppearance::toString::: value wasn't set");
}

TLV_SMI::TLV_SMI()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_SMI::TLV_SMI(uint8_t smi)
  : common::TLV_IntegerPrimitive(TAG, smi)
{
  if ( smi > 0x04 && smi != 0xff )
    throw smsc::util::Exception("TLV_SMI::TLV_SMI::: wrong smi parameter value=[%d], expected value from range [0x00-0x04,0xff]", smi);
}

size_t
TLV_SMI::deserialize(const common::TP& packet_buf,
                     size_t offset /*position inside buffer where tag's data started*/,
                     uint16_t val_len)
{
  offset = common::TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
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
      return "SMI=[Reserved/Unknown]";
    if ( value == 0x01 )
      return "SMI=[Solitary]";
    if ( value == 0x02 )
      return "SMI=[Duplicated]";
    if ( value == 0x03 )
      return "SMI=[Triplicated]";
    if ( value == 0xff )
      return "SMI=[Unspecified]";

    return "SMI=[Unknown]";
  } else
    throw smsc::util::Exception("TLV_SMI::toString::: value wasn't set");
}

TLV_UserCause::TLV_UserCause()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_UserCause::TLV_UserCause(uint16_t cause, uint16_t user)
  : common::TLV_IntegerPrimitive(TAG, ((uint32_t)cause << 16) | user)
{}

size_t
TLV_UserCause::deserialize(const common::TP& packet_buf,
                           size_t offset /*position inside buffer where tag's data started*/,
                           uint16_t val_len)
{
  offset = common::TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
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
    return strBuf;
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
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_CongestionLevel::TLV_CongestionLevel(uint8_t congestion_level)
  : common::TLV_IntegerPrimitive(TAG, congestion_level)
{}

std::string
TLV_CongestionLevel::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "congestionLevel=[%d]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_CongestionLevel::toString::: value wasn't set");
}

TLV_ProtocolClass::TLV_ProtocolClass()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_ProtocolClass::TLV_ProtocolClass(const ProtocolClass& protocol_class)
  : common::TLV_IntegerPrimitive(TAG, protocol_class)
{}

std::string
TLV_ProtocolClass::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "protocolClass=[%d:%s]", getValue(), ProtocolClass(getValue()).toString().c_str());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_ProtocolClass::toString::: value wasn't set");
}

ProtocolClass
TLV_ProtocolClass::getProtocolClassValue() const
{
  return ProtocolClass(getValue());
}

TLV_SequenceControl::TLV_SequenceControl()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_SequenceControl::TLV_SequenceControl(uint32_t sequence_control)
  : common::TLV_IntegerPrimitive(TAG, sequence_control)
{}

std::string
TLV_SequenceControl::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "sequenceControl=[%d]", getValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_SequenceControl::toString::: value wasn't set");
}

TLV_Segmentation::TLV_Segmentation()
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG)
{}

TLV_Segmentation::TLV_Segmentation(bool is_first_segment,
                                   uint8_t num_of_remaining_segments, uint32_t reference)
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG, temporary_buf(is_first_segment,
                                                                         num_of_remaining_segments,
                                                                         reference).array,
                                                                         static_cast<uint16_t>(sizeof(uint32_t)))
{}

TLV_Segmentation::temporary_buf::temporary_buf(bool is_first_segment, uint8_t num_of_remaining_segments, uint32_t reference)
{
  array[0] = num_of_remaining_segments & 0x7F;
  if ( is_first_segment ) array[0] |= 0x80;
  reference = htonl(reference);
  memcpy(&array[1], reinterpret_cast<uint8_t*>(&reference) + 1, sizeof(reference) - 1); // copy low 3 bytes
}

std::string
TLV_Segmentation::toString() const
{
  if ( isSetValue() ) {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "firstSegmentIndication=[%d],numOfRemainingSegments=[%d],refValue=[%d]", isFirstSegment(), getNumOfRemainingSegments(), getReferenceValue());
    return strBuf;
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
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_SS7HopCount::TLV_SS7HopCount(uint8_t hop_count)
  : common::TLV_IntegerPrimitive(TAG, hop_count)
{}

std::string
TLV_SS7HopCount::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "ss7HopCount=[%d]", getHopCountValue());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_SS7HopCount::toString::: value wasn't set");
}

uint8_t
TLV_SS7HopCount::getHopCountValue() const
{
  return static_cast<uint8_t>(getValue() & 0x000000FF);
}

TLV_Importance::TLV_Importance()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_Importance::TLV_Importance(uint8_t importance)
  : common::TLV_IntegerPrimitive(TAG, importance)
{}

size_t
TLV_Importance::deserialize(const common::TP& packet_buf,
                            size_t offset /*position inside buffer where tag's data started*/,
                            uint16_t val_len)
{
  offset = common::TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
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
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_Importance::toString::: value wasn't set");
}

uint8_t
TLV_Importance::getImportanceValue() const
{
  return static_cast<uint8_t>(getValue() & 0xff);
}

TLV_MessagePriority::TLV_MessagePriority()
  : common::TLV_IntegerPrimitive(TAG)
{}

TLV_MessagePriority::TLV_MessagePriority(uint8_t priority)
  : common::TLV_IntegerPrimitive(TAG, priority)
{
  if ( priority > 0x03 && priority != 0xFF )
    throw smsc::util::Exception("TLV_MessagePriority::TLV_MessagePriority::: wrong messagePriority value=[%d], expected value from set [0x00-0x03,0xFF]", priority);
}

size_t
TLV_MessagePriority::deserialize(const common::TP& packet_buf,
                                 size_t offset /*position inside buffer where tag's data started*/,
                                 uint16_t val_len)
{
  offset = common::TLV_IntegerPrimitive::deserialize(packet_buf, offset, val_len);
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
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_MessagePriority::toString::: value wasn't set");
}

uint8_t
TLV_MessagePriority::getMessagePriorityValue() const
{
  return static_cast<uint8_t>(getValue() & 0xff);
}

TLV_Data::TLV_Data()
  : common::TLV_OctetArrayPrimitive<common::MAX_OCTET_ARRAY_SIZE>(TAG)
{}

TLV_Data::TLV_Data(const uint8_t* val, uint16_t len)
  : common::TLV_OctetArrayPrimitive<common::MAX_OCTET_ARRAY_SIZE>(TAG, val, len)
{}

std::string
TLV_Data::toString() const
{
  if ( isSetValue() ) {
    char strBuf[common::MAX_OCTET_ARRAY_SIZE*2];
    snprintf(strBuf, sizeof(strBuf), "data=[%s]", utilx::hexdmp(getValue(), getValueLength()).c_str());
    return strBuf;
  } else
    throw smsc::util::Exception("TLV_Data::toString::: value wasn't set");
}

std::string
TLV_PointCode::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), "PC=%s", utilx::hexdmp(getValue(), getValueLength()).c_str());
  return strBuf;
}

common::ANSI_PC
TLV_PointCode::get_ANSI_PC() const
{
  const uint8_t* value = getValue();
  return common::ANSI_PC(value[0], value[1], value[2], value[3]);
}

common::ITU_PC
TLV_PointCode::get_ITU_PC() const
{
  const uint8_t* value = getValue();
  return common::ITU_PC(value[0], uint8_t((value[2] >> 3) & 0x07),
                        ((value[2] & 0x07 << 5) |  value[3] >> 3), value[3] & 0x07);
}

TLV_GlobalTitle::TLV_GlobalTitle()
  : common::TLV_OctetArrayPrimitive<GT_TOTAL_MAX_SZ>(TAG)
{}

TLV_GlobalTitle::TLV_GlobalTitle(const GlobalTitle& gt)
  : common::TLV_OctetArrayPrimitive<GT_TOTAL_MAX_SZ>(TAG, RESERVED_OCTETS, gt.getValue(), static_cast<uint16_t>(gt.getValueSz())), _globalTitle(gt)
{}

size_t
TLV_GlobalTitle::deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len)
{
  offset = common::TLV_OctetArrayPrimitive<GT_TOTAL_MAX_SZ>::deserialize(packet_buf, offset, val_len);

  _globalTitle = GlobalTitle(getValue() + RESERVED_OCTETS, getValueLength() - RESERVED_OCTETS);
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
TLV_GlobalTitle::getGlobalTitleValue() const
{
  return _globalTitle;
}

TLV_Address::TLV_Address(uint16_t tag)
  : common::AdaptationLayer_TLV(tag), _routingIndicator(0), _addressIndicator(0),
    _isValueSet(false)
{}

TLV_Address::TLV_Address(uint16_t tag, const TLV_PointCode& point_code, const TLV_SSN& ssn)
  : common::AdaptationLayer_TLV(tag), _routingIndicator(ROUTE_ON_SSN_PLUS_PC),
    _addressIndicator(0), _pointCode(point_code), _ssn(ssn), _isValueSet(true)
{}

TLV_Address::TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt, const TLV_SSN& ssn)
  : common::AdaptationLayer_TLV(tag), _routingIndicator(ROUTE_ON_GT),
    _addressIndicator(GT_INCLUDE_INDICATION|SSN_INCLUDE_INDICATION),  _ssn(ssn), _gt(gt), _isValueSet(true)
{}

TLV_Address::TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt)
  : common::AdaptationLayer_TLV(tag), _routingIndicator(ROUTE_ON_GT), _addressIndicator(GT_INCLUDE_INDICATION), _gt(gt), _isValueSet(true)
{}

size_t
TLV_Address::serialize(common::TP* packet_buf,
                       size_t offset /*position inside buffer where tag's data will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_Address::serialize::: value isn't set");

  offset = common::AdaptationLayer_TLV::serialize(packet_buf, offset);

  offset = common::addField(packet_buf, offset, _routingIndicator);
  offset = common::addField(packet_buf, offset, _addressIndicator);

  if ( _pointCode.isSetValue() )
    offset = _pointCode.serialize(packet_buf, offset);
  if ( _ssn.isSetValue() )
    offset = _ssn.serialize(packet_buf, offset);
  if ( _gt.isSetValue() )
    offset = _gt.serialize(packet_buf, offset);

  size_t paddingTo4bytes = offset & 0x03;
  if ( paddingTo4bytes ) { // if tag length is not a multiple of 4 bytes
    uint8_t padding[4]={0};
    offset = common::addField(packet_buf, offset, padding, 0x04 - paddingTo4bytes);
  }
  return offset;
}

size_t
TLV_Address::deserialize(const common::TP& packet_buf,
                         size_t offset /*position inside buffer where tag's data started*/,
                         uint16_t val_len)
{
  offset = common::extractField(packet_buf, offset, &_routingIndicator);
  offset = common::extractField(packet_buf, offset, &_addressIndicator);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_pointCode);
  tlvFactory.registerExpectedOptionalTlv(&_ssn);
  tlvFactory.registerExpectedOptionalTlv(&_gt);

  common::TP tmppacket_buf (packet_buf.packetType, val_len - sizeof(_addressIndicator) - sizeof(_addressIndicator), packet_buf.packetBody + offset, common::TP::MAX_PACKET_SIZE - offset);

  offset += tlvFactory.parseInputBuffer(tmppacket_buf, 0);

  _isValueSet = true;

  return offset;
}

uint16_t
TLV_Address::getLength() const
{
  uint16_t sz = static_cast<uint16_t>(HEADER_SZ) + static_cast<uint16_t>(INDICATORS_FIELDS_SZ);
  if ( _pointCode.isSetValue() )
    sz += _pointCode.getLength();
  if ( _ssn.isSetValue() )
    sz += _ssn.getLength();
  if ( _gt.isSetValue() )
    sz += _gt.getLength();

  uint16_t paddingTo4bytesSz = sz & 0x03;
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
TLV_Address::setAddressIndicator(uint16_t addr_indicator)
{
  _addressIndicator = addr_indicator;
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

TLV_SourceAddress::TLV_SourceAddress(const TLV_PointCode& point_code, const TLV_SSN& ssn)
  : TLV_Address(TAG, point_code, ssn)
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

TLV_DestinationAddress::TLV_DestinationAddress(const TLV_PointCode& point_code, const TLV_SSN& ssn)
  : TLV_Address(TAG, point_code, ssn)
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
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG)
{}

TLV_SCCP_Cause::TLV_SCCP_Cause(common::return_cause_type_t cause_type, uint8_t cause_value)
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG, temporary_buf(uint8_t(cause_type), cause_value).array, static_cast<uint16_t>(sizeof(uint32_t)))
{}

size_t
TLV_SCCP_Cause::deserialize(const common::TP& packet_buf,
                            size_t offset /*position inside buffer where tag's data started*/,
                            uint16_t val_len)
{
  offset = common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>::deserialize(packet_buf, offset, val_len);
  const uint8_t* value = getValue();
  if ( value[2] == 0 || 
       value[2] > common::ERROR_CAUSE )
    throw smsc::util::Exception("TLV_SCCP_Cause::deserialize::: wrong cause type value=[%d], expected value from range [0x01-0x05]", value[2]);
  return offset;
}

std::string
TLV_SCCP_Cause::toString() const
{
  if ( isSetValue() ) {
    char strBuf[128];
    snprintf(strBuf, sizeof(strBuf), "SCCP_Cause=[causeType=[%d],causeValue=[%d]]", getCauseType(), getCauseValue());
    return strBuf;
  } else
    return "";
}

common::return_cause_type_t
TLV_SCCP_Cause::getCauseType() const
{
  const uint8_t* value = getValue();
  return common::return_cause_type_t(value[2]);
}

uint8_t
TLV_SCCP_Cause::getCauseValue() const
{
  const uint8_t* value = getValue();
  return value[3];
}

TLV_SCCP_Cause::temporary_buf::temporary_buf(uint8_t cause_type, uint8_t cause_value)
{
  array[0] = array[1] = 0;

  if ( cause_type == 0 ||
       cause_type > common::ERROR_CAUSE )
    throw smsc::util::Exception("TLV_SCCP_Cause::temporary_buf::: wrong cause type value=[%d], expected value from range [0x01-0x05]", cause_type);
  array[2] = cause_type; array[3] = cause_value;
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
  else throw smsc::util::Exception("AdaptationLayer_TLV_Factory::getTagIdx::: wrong tag value [=0x%04x]", tag);
}

}}}}}
