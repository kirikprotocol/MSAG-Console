#include "ClusterLoadBalancerProtocolClnt.hpp"
#include "eyeline/load_balancer/cluster/messages/GetServicesStatus.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectService.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

void ClusterLoadBalancerProtocol::assignHandler(ClusterControllerMsgsHndlr* newHandler)
{
  handler=newHandler;
}

void ClusterLoadBalancerProtocol::decodeAndHandleMessage(const char* buf,size_t sz)
{
  eyeline::protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ClusterLoadBalancerProtocol::decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_GetServicesStatus:
    {
      messages::GetServicesStatus msg;
      msg.deserialize(ss);
      msg.messageSetSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DisconnectService:
    {
      messages::DisconnectService msg;
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
