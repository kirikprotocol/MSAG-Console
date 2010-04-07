#include "eyeline/utilx/Exception.hpp"

#include "DownMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

const uint32_t DownMessage::_MSG_CODE;

size_t
DownMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);
  return offset;
}

size_t
DownMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  M3uaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_infoString);

  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
DownMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());
  if ( _infoString.isSetValue() )
    result += _infoString.toString();

  return result;
}

uint32_t
DownMessage::getLength() const
{
  uint32_t length=0;
  if ( _infoString.isSetValue() )
    length = _infoString.getLength();

  return HEADER_SIZE + length;
}

const common::TLV_InfoString&
DownMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DownMessage::getInfoString::: infoString wasn't set");
}

}}}}}}
