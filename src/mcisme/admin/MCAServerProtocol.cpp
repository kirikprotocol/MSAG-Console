#include "MCAServerProtocol.hpp"
#include "messages/FlushStats.hpp"
#include "messages/GetStats.hpp"
#include "messages/GetRunStats.hpp"
#include "messages/GetSched.hpp"
#include "messages/GetSchedList.hpp"
#include "messages/GetProfile.hpp"
#include "messages/SetProfile.hpp"

namespace smsc {
namespace mcisme {
namespace admin {

void MCAServerProtocol::assignHandler(MCAServerProtocolHandler* newHandler)
{
  handler=newHandler;
}

void MCAServerProtocol::decodeAndHandleMessage(const char* buf,size_t sz)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void MCAServerProtocol::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_FlushStats:
    {
      messages::FlushStats msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetStats:
    {
      messages::GetStats msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetRunStats:
    {
      messages::GetRunStats msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetSched:
    {
      messages::GetSched msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetSchedList:
    {
      messages::GetSchedList msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_GetProfile:
    {
      messages::GetProfile msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_SetProfile:
    {
      messages::SetProfile msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    default:
      throw eyeline::protogen::framework::UnhandledMessage(tag);
  }
}


}
}
}
