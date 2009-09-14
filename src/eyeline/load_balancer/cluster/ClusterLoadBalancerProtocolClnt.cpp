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
  protogen::framework::SerializerBuffer ss;
  ss.setExternalData(buf,sz);
  decodeAndHandleMessage(ss);
}

void ClusterLoadBalancerProtocol::decodeAndHandleMessage(protogen::framework::SerializerBuffer& ss)
{
  uint32_t tag=ss.readInt32();
  uint32_t seq=ss.readInt32();
  switch(tag)
  {
    case tag_GetServicesStatus:
    {
      messages::GetServicesStatus msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    case tag_DisconnectService:
    {
      messages::DisconnectService msg;
      msg.deserialize(ss);
      msg.setSeqNum(seq);
      handler->handle(msg);
    }break;
    default:
      throw protogen::framework::UnhandledMessage(tag);
  }
}

void ClusterLoadBalancerProtocol::encodeMessage(const messages::RegisterAsLoadBalancer& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_RegisterAsLoadBalancer);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ClusterLoadBalancerProtocol::encodeMessage(const messages::GetServicesStatusResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_GetServicesStatusResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}
void ClusterLoadBalancerProtocol::encodeMessage(const messages::DisconnectServiceResp& msg,protogen::framework::SerializerBuffer* ss)
{
  ss->writeInt32(4+4+msg.length<protogen::framework::SerializerBuffer>());
  ss->writeInt32(tag_DisconnectServiceResp);
  ss->writeInt32(msg.getSeqNum());
  msg.serialize(*ss);
}

}
}
}
