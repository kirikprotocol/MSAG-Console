#include <eyeline/utilx/Exception.hpp>

#include "DownAckMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

const uint32_t DownAckMessage::_MSG_CODE;

DownAckMessage::DownAckMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
DownAckMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);

  return offset;
}

size_t
DownAckMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
DownAckMessage::toString() const
{
  std::string result(SUAMessage::toString());

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

const char*
DownAckMessage::getMsgCodeTextDescription() const
{
  return "ASP DOWN ACK";
}

void
DownAckMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
DownAckMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DownAckMessage::getInfoString::: infoString wasn't set");
}

}}}}
