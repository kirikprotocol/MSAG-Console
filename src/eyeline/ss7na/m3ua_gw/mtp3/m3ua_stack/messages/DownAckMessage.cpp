#include "eyeline/utilx/Exception.hpp"

#include "DownAckMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

const uint32_t DownAckMessage::_MSG_CODE;

size_t
DownAckMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);

  return offset;
}

size_t
DownAckMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  M3uaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
DownAckMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _infoString.isSetValue() )
    result += _infoString.toString();

  return result;
}

uint32_t
DownAckMessage::getLength() const
{
  uint32_t length=0;

  if ( _infoString.isSetValue() )
    length = _infoString.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_InfoString&
DownAckMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DownAckMessage::getInfoString::: infoString wasn't set");
}

}}}}}}
