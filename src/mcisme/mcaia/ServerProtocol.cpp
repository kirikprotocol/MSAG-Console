#include "ServerProtocol.hpp"
#include "mcisme/mcaia/BusyResponse.hpp"

namespace smsc {
namespace mcisme {
namespace mcaia {

void ServerProtocol::assignHandler(IAProtocolHandler* newHandler)
{
  handler=newHandler;
}

void ServerProtocol::decodeAndHandleMessage(const char* buf,size_t sz)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ServerProtocol::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_BusyResponse:
    {
      BusyResponse msg;
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
