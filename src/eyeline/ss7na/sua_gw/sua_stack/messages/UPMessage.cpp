#include <stdio.h>
#include "eyeline/utilx/Exception.hpp"

#include "UPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const uint32_t UPMessage::_MSG_CODE;

size_t
UPMessage::serialize(common::TP* result_buf) const
{
  size_t offset = common::AdaptationLayer_Message::serialize(result_buf);

  if ( _aspId.isSetValue() )
    offset = _aspId.serialize(result_buf, offset);
  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(result_buf, offset);

  return offset;
}

size_t
UPMessage::deserialize(const common::TP& packet_buf)
{
  size_t offset = common::AdaptationLayer_Message::deserialize(packet_buf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_aspId);
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packet_buf, offset);
}

std::string
UPMessage::toString() const
{
  std::string result(common::AdaptationLayer_Message::toString());

  if ( _aspId.isSetValue() )
    result += "," + _aspId.toString();

  if ( _infoString.isSetValue() )
    result += "," + _infoString.toString();

  return result;
}

uint32_t
UPMessage::getLength() const
{
  uint32_t length=0;
  if ( _aspId.isSetValue() ) length = _aspId.getLength();
  if ( _infoString.isSetValue() ) length += _infoString.getLength();
  return HEADER_SIZE + length;
}

const common::TLV_AspIdentifier&
UPMessage::getAspIdentifier() const
{
  if ( _aspId.isSetValue() ) return _aspId;
  else throw utilx::FieldNotSetException("UPMessage::getAspIdentifier::: aspId wasn't set");
}

const common::TLV_InfoString&
UPMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("UPMessage::getInfoString::: infoString wasn't set");
}

}}}}}
