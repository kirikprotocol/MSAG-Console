#include "AdaptationLayer_MsgCodesIndexer.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

void
AdaptationLayer_MsgCodesIndexer::registerMessageCode(uint32_t msg_code)
{
  uint8_t msgClass = (msg_code & 0x0000FF00) >> 8, msgType = msg_code & 0x000000FF;

  if ( msgClass > MAX_MSG_CLASS_VALUE ) throw smsc::util::Exception("AdaptationLayer_MsgCodesIndexer::registerMessageCode::: msgClass value [=0x%x] is greater than max allowable value [=0x%x]", msgClass, MAX_MSG_CLASS_VALUE);
  if ( msgType > MAX_MSG_TYPE_VALUE) throw smsc::util::Exception("AdaptationLayer_MsgCodesIndexer::registerMessageCode::: msgType value [=0x%x] is greater than max allowable value [=0x%x]", msgType, MAX_MSG_TYPE_VALUE);
  _msgClassTypeToMessageIdx[msgClass][msgType] = generateMessageIndexValue();
}

unsigned int
AdaptationLayer_MsgCodesIndexer::getMessageIndex(uint32_t msg_code)
{
  uint8_t msgClass = (msg_code & 0x0000FF00) >> 8, msgType = msg_code & 0x000000FF;

  if ( msgClass > MAX_MSG_CLASS_VALUE ) throw smsc::util::Exception("AdaptationLayer_MsgCodesIndexer::getMessageIndex::: msgClass value [=0x%x] is greater than max allowable value [=0x%x]", msgClass, MAX_MSG_CLASS_VALUE);
  if ( msgType > MAX_MSG_TYPE_VALUE) throw smsc::util::Exception("AdaptationLayer_MsgCodesIndexer::getMessageIndex::: msgType value [=0x%x] is greater than max allowable value [=0x%x]", msgType, MAX_MSG_TYPE_VALUE);

  unsigned int idx = _msgClassTypeToMessageIdx[msgClass][msgType];
  if ( idx == 0 )
    throw smsc::util::Exception("AdaptationLayer_MsgCodesIndexer::getMessageIndex::: unknown msgCode value [=0x%x]", msg_code);

  return idx;
}

unsigned int AdaptationLayer_MsgCodesIndexer::_msgClassTypeToMessageIdx[MAX_MSG_CLASS_VALUE+1][MAX_MSG_TYPE_VALUE+1];

unsigned int AdaptationLayer_MsgCodesIndexer::_MSG_IDX;

}}}
