#ifndef __EYELINE_LOADBALANCER_CLUSTER_CLUSTERSUBSYSTEM_HPP__
# define __EYELINE_LOADBALANCER_CLUSTER_CLUSTERSUBSYSTEM_HPP__

# include <sys/types.h>
# include <string>

# include "logger/Logger.h"
# include "util/config/ConfigView.h"
# include "eyeline/load_balancer/cluster/ClusterControllerMgr.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

class ClusterSubsystem {
public:
  ClusterSubsystem();
  void initialize(const smsc::util::config::ConfigView& cluster_cfg_entry);
  void start();
  void stop();
  void waitForCompletion();
private:
  smsc::logger::Logger* _logger;
  ClusterControllerMgr* _clusterCtrlMgr;
};

}}}

#endif

