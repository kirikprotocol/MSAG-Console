#ifndef __EYELINE_LOAD_BALANCER_CLUSTER_CLUSTERCONTROLLERMSGHNDLR_HPP__
#define __EYELINE_LOAD_BALANCER_CLUSTER_CLUSTERCONTROLLERMSGHNDLR_HPP__

#include "eyeline/load_balancer/cluster/messages/GetServicesStatus.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectService.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

class ClusterControllerMsgsHndlr{
public:
  void handle(const messages::GetServicesStatus& msg);
  void handle(const messages::DisconnectService& msg);
};


}
}
}

#endif
