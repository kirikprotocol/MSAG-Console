#ifndef __EYELINE_LOAD_BALANCER_CLUSTER_CLUSTERLOADBALANCERPROTOCOL_HPP__
#define __EYELINE_LOAD_BALANCER_CLUSTER_CLUSTERLOADBALANCERPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "eyeline/load_balancer/cluster/ClusterControllerMsgsHndlr.hpp"
#include "eyeline/load_balancer/cluster/messages/RegisterAsLoadBalancer.hpp"
#include "eyeline/load_balancer/cluster/messages/GetServicesStatusResp.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectServiceResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

class ClusterLoadBalancerProtocol{
public:
  enum{
    tag_GetServicesStatus=101,
    tag_DisconnectService=102,
    tag_RegisterAsLoadBalancer=301,
    tag_GetServicesStatusResp=1101,
    tag_DisconnectServiceResp=1102
  };
 
  ClusterLoadBalancerProtocol():handler(0)
  {
  }

  void assignHandler(ClusterControllerMsgsHndlr* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz);
  void decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss);

  template <class MsgType>
  void encodeMessage(const MsgType& msg,eyeline::protogen::framework::SerializerBuffer* ss)
  {
    ss->writeInt32(4+4+msg.template length<eyeline::protogen::framework::SerializerBuffer>());
    ss->writeInt32(msg.getTag());
    ss->writeInt32(msg.getSeqNum());
    msg.serialize(*ss);
  }

protected:
  ClusterControllerMsgsHndlr* handler;
};

}
}
}

#endif
