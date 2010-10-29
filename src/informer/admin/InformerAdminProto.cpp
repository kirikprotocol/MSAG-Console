#include "InformerAdminProto.hpp"
#include "messages/ConfigOp.hpp"
#include "messages/SetDefaultSmsc.hpp"
#include "messages/LoggerGetCategories.hpp"
#include "messages/LoggerSetCategories.hpp"

namespace eyeline {
namespace informer {
namespace admin {

void InformerAdminProto::assignHandler(AdminCommandsHandler* newHandler)
{
  handler=newHandler;
}

void InformerAdminProto::decodeAndHandleMessage(const char* buf,size_t sz,int connId)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss,connId);
}

void InformerAdminProto::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss,int connId)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_ConfigOp:
    {
      messages::ConfigOp msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_SetDefaultSmsc:
    {
      messages::SetDefaultSmsc msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_LoggerGetCategories:
    {
      messages::LoggerGetCategories msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    case tag_LoggerSetCategories:
    {
      messages::LoggerSetCategories msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      msg.messageSetConnId(connId);
      handler->handle(msg);
    }break;
    default:
      throw eyeline::protogen::framework::UnhandledMessage(tag);
  }
}


}
}
}
