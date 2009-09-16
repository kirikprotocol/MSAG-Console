#ifndef __EYELINE_LOADBALANCER_CLUSTER_CLUSTERCONTROLLERMSGHNDLR_HPP__
# define __EYELINE_LOADBALANCER_CLUSTER_CLUSTERCONTROLLERMSGHNDLR_HPP__

# include <vector>

# include "logger/Logger.h"

# include "eyeline/load_balancer/cluster/messages/GetServicesStatus.hpp"
# include "eyeline/load_balancer/cluster/messages/DisconnectService.hpp"
# include "eyeline/load_balancer/cluster/messages/ServiceStatus.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

class ClusterControllerMgr;
class ClusterLoadBalancerProtocol;

class ClusterControllerMsgsHndlr{
public:
  ClusterControllerMsgsHndlr(ClusterControllerMgr& mgr,
                             ClusterLoadBalancerProtocol& protocol)
  : _logger(smsc::logger::Logger::getInstance("cluster")), _mgr(mgr),
    _protocol(protocol)
  {}

  void handle(const messages::GetServicesStatus& msg);
  void handle(const messages::DisconnectService& msg);
private:
  void getAllSmeStatus(std::vector<messages::ServiceStatus>* services_atatus_array);
  void disconnectSme(const std::string& service_id);

  smsc::logger::Logger* _logger;
  ClusterControllerMgr& _mgr;
  ClusterLoadBalancerProtocol& _protocol;
  enum { RESPONSE_OK = 0, RESPONSE_FAILED = 1 };
};

}}}

#endif
