#include "eyeline/load_balancer/cluster/messages/GetServicesStatus.hpp"
#include "eyeline/load_balancer/cluster/messages/GetServicesStatusResp.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectService.hpp"
#include "eyeline/load_balancer/cluster/messages/DisconnectServiceResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

void ClusterControllerMsgsHndlr::handle(const messages::GetServicesStatus& msg)
{
  printf("GetServicesStatus:%s\n",msg.toString().c_str());
  //!!!TODO!!!
}
void ClusterControllerMsgsHndlr::handle(const messages::DisconnectService& msg)
{
  printf("DisconnectService:%s\n",msg.toString().c_str());
  //!!!TODO!!!
}


}
}
}
