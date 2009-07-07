#include "eyeline/utilx/Exception.hpp"

#include "QuerySmResp.hpp"
#include "codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

QuerySmResp::QuerySmResp()
  : FastParsableSmppMessage(_MSGCODE), _isSetMessageId(false), _isItTransitMessage(true)
{}

QuerySmResp::QuerySmResp(const QuerySm& querySm, uint32_t status)
  : FastParsableSmppMessage(_MSGCODE), _isSetMessageId(false), _isItTransitMessage(false)
{
  setCommandStatus(status);
  setMessageId(querySm.getMessageId());
}

void
QuerySmResp::setMessageId(uint64_t messageId)
{
  _isSetMessageId = true;
  _messageId = messageId;
  snprintf(_messageIdCStr, sizeof(_messageIdCStr), "%llu", messageId);
}

uint64_t
QuerySmResp::getMessageId() const
{
  if ( !_isSetMessageId )
    throw utilx::FieldNotSetException("QuerySmResp::getMessageId::: field is not set");
  return _messageId;
}

size_t
QuerySmResp::serialize(io_subsystem::Packet* packet) const
{
  if ( _isItTransitMessage )
    return FastParsableSmppMessage::serialize(packet);
  else {
    SMPPMessage::serialize(packet);
    addCOctetString(packet, _messageIdCStr, sizeof(_messageIdCStr));
    addCOctetString(packet, NULL, 0);
    packet->addValue(uint8_t(0));
    return packet->addValue(uint8_t(0));
  }
}

std::string
QuerySmResp::toString() const
{
  if ( _isItTransitMessage )
    return FastParsableSmppMessage::toString();
  else {
    char message_dump[256];
    snprintf(message_dump, sizeof(message_dump), ",messageId='%s',final_date=NULL,message_state=0,error_code=0", _messageIdCStr);
    return SMPPMessage::toString() + message_dump;
  }
}

uint32_t
QuerySmResp::calculateCommandBodyLength() const
{
  if ( _isItTransitMessage )
    return FastParsableSmppMessage::calculateCommandBodyLength();
  else {
    uint32_t messageIdCStrLen = static_cast<uint32_t>(strlen(_messageIdCStr)) + 1;
    return
      messageIdCStrLen + LENGTH_OF_NULL_VALUE_FINAL_DATE_FIELD +
      MESSAGE_STATE_FIELD_LENGTH + ERROR_CODE_FIELD_LENGTH;
  }
}

}}}}
