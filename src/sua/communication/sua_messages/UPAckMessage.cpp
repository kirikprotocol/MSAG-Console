#include <eyeline/utilx/Exception.hpp>

#include "UPAckMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t UPAckMessage::_MSG_CODE;

UPAckMessage::UPAckMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
UPAckMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);

  return offset;
}

size_t
UPAckMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
UPAckMessage::toString() const
{
  std::string result(SUAMessage::toString());
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

const char*
UPAckMessage::getMsgCodeTextDescription() const
{
  return "ASP UP ACK";
}

void
UPAckMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
UPAckMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("UPAckMessage::getInfoString::: infoString wasn't set");
}

}}}}
