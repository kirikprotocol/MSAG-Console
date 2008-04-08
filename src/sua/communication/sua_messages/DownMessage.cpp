#include <stdio.h>
#include <util/Exception.hpp>
#include <sua/sua_layer/io_dispatcher/SuaLayerMessagesFactory.hpp>

#include "DownMessage.hpp"

namespace sua_messages {

const uint32_t DownMessage::_MSG_CODE;

DownMessage::DownMessage ()
  : SUAMessage(_MSG_CODE) {}

size_t
DownMessage::serialize(communication::TP* resultBuf) const
{
  size_t offset = SUAMessage::serialize(resultBuf);

  if ( _infoString.isSetValue() )
    offset = _infoString.serialize(resultBuf, offset);
  return offset;
}

size_t
DownMessage::deserialize(const communication::TP& packetBuf)
{
  size_t offset = SUAMessage::deserialize(packetBuf);

  SuaTLVFactory tlvFactory;
  tlvFactory.registerExpectedOptionalTlv(&_infoString);
  return tlvFactory.parseInputBuffer(packetBuf, offset);
}

std::string
DownMessage::toString() const
{
  std::string result(SUAMessage::toString());
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

const char*
DownMessage::getMsgCodeTextDescription() const
{
  return "ASP DOWN";
}

void
DownMessage::setInfoString(const TLV_InfoString& infoString)
{
  _infoString = infoString;
}

const TLV_InfoString&
DownMessage::getInfoString() const
{
  if ( _infoString.isSetValue() ) return _infoString;
  else throw utilx::FieldNotSetException("DownMessage::getInfoString::: infoString wasn't set");
}

}
