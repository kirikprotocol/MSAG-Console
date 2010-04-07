#include "SCCPMessage.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

size_t
SCCPMessage::serialize(common::TP* result_buf) const
{
  return common::addField(result_buf, 0, _msgCode);
}

size_t
SCCPMessage::deserialize(const common::TP& packet_buf)
{
  return common::extractField(packet_buf, 0, &_msgCode);
}

size_t
SCCPMessage::addMFixedField(common::TP* result_buf, size_t offset, uint8_t value) const
{
  return common::addField(result_buf, offset, value);
}

size_t
SCCPMessage::addOneOctetPointer(common::TP* result_buf, size_t offset) const
{
  return common::addField(result_buf, offset, uint8_t(0));
}

size_t
SCCPMessage::addMVariablePartValue(common::TP* result_buf, size_t offset,
                                   size_t pointer_to_param, unsigned data_len,
                                   const uint8_t* data) const
{
  size_t distance = offset - pointer_to_param;
  if ( distance > 0xff )
    throw utilx::SerializationException("SCCPMessage::addMVariablePartValue::: mandatory variable parameter offset is too long [%u]",
                                        distance);
  if ( data_len < 0xff ) {
    offset = common::addField(result_buf, offset, uint8_t(data_len));
    offset = common::addField(result_buf, offset, data, data_len);
    // update sccp parameter pointer value
    common::addField(result_buf, pointer_to_param, uint8_t(distance));
  } else
    throw utilx::SerializationException("SCCPMessage::addMVariablePartValue::: too long [%d octets] data",
                                        data_len);

  return offset;
}

size_t
SCCPMessage::addOptionalParameter(common::TP* result_buf, size_t offset, uint8_t tag,
                                  uint8_t value_length, const uint8_t* value) const
{
  offset = common::addField(result_buf, offset, tag);
  offset = common::addField(result_buf, offset, value_length);
  offset = common::addField(result_buf, offset, value, value_length);

  return offset;
}

size_t
SCCPMessage::addEndOfOptionalParams(common::TP* result_buf, size_t offset) const
{
  return common::addField(result_buf, offset, uint8_t(0));
}

void
SCCPMessage::updateOptinalPointer(common::TP* result_buf, size_t optional_part_ptr,
                                  size_t begin_optional_part) const
{
  size_t distance = begin_optional_part - optional_part_ptr;
  if ( distance > 0xff )
    throw utilx::SerializationException("SCCPMessage::updateOptinalPointer::: optional parameter offset is too long [%u]",
                                        distance);

  common::addField(result_buf, optional_part_ptr, uint8_t(distance));
}

size_t
SCCPMessage::extractMFixedField(const common::TP& packet_buf, size_t offset,
                                uint8_t* value) const
{
  return common::extractField(packet_buf, offset, value);
}

size_t
SCCPMessage::extractOneOctetPointer(const common::TP& packet_buf, size_t offset,
                                    uint8_t* value) const
{
  return common::extractField(packet_buf, offset, value);
}

size_t
SCCPMessage::extractParamName(const common::TP& packet_buf, size_t offset, uint8_t* value) const
{
  return common::extractField(packet_buf, offset, value);
}

size_t
SCCPMessage::extractLengthIndicator(const common::TP& packet_buf, size_t offset, uint8_t* value) const
{
  return common::extractField(packet_buf, offset, value);
}

}}}}}
