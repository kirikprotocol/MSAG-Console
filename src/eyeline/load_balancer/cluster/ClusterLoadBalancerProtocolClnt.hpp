#ifndef __EYELINE_LOAD_BALANCER_CLUSTER_CLUSTERLOADBALANCERPROTOCOL_HPP__
#define __EYELINE_LOAD_BALANCER_CLUSTER_CLUSTERLOADBALANCERPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "eyeline/load_balancer/cluster/ClusterControllerMsgsHndlr.hpp"
#include "eyeline/load_balancer/cluster/messages/GetServicesStatusResp.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectServiceResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

class ClusterLoadBalancerProtocol{
public:
  enum{
    tag_GetServicesStatus=63,
    tag_DisconnectService=64,
    tag_GetServicesStatusResp=1063,
    tag_DisconnectServiceResp=1064
  };
 
  ClusterLoadBalancerProtocol():handler(0)
  {
  }

  void assignHandler(ClusterControllerMsgsHndlr* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz);
  void decodeAndHandleMessage(protogen::framework::SerializerBuffer& ss);

  void encodeMessage(const messages::GetServicesStatusResp& msg,protogen::framework::SerializerBuffer* ss);
  void encodeMessage(const messages::DisconnectServiceResp& msg,protogen::framework::SerializerBuffer* ss);
protected:
  ClusterControllerMsgsHndlr* handler;
};

}
}
}

#endif
