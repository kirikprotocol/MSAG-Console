#include "eyeline/utilx/Exception.hpp"

#include "UPAckMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t UPAckMessage::_MSG_CODE;

size_t
UPAckMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);

  return offset;
}

size_t
UPAckMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
UPAckMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());
  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
UPAckMessage::getLength() const
{
  uint32_t length=0;
  if ( _infoString.isSetValue() ) length += _infoString.getLength();
  return HEADER_SIZE + length;
}

const common::TLV_InfoString&
UPAckMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("UPAckMessage::getInfoString::: infoString wasn't set");
}

}}}}}
