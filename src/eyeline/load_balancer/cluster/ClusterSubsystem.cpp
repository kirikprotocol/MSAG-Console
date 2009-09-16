#include <netinet/in.h>
#include "ClusterSubsystem.hpp"

namespace eyeline {
namespace load_balancer {
namespace cluster {

ClusterSubsystem::ClusterSubsystem()
  : _logger(smsc::logger::Logger::getInstance("cluster"))
  {}

void
ClusterSubsystem::initialize(const smsc::util::config::ConfigView& cluster_cfg_entry)
{
  std::string peerHost = cluster_cfg_entry.getString("peer_host");
  in_port_t peerPort = static_cast<in_port_t>(cluster_cfg_entry.getInt("peer_port"));
  unsigned connectTimeout = static_cast<unsigned>(cluster_cfg_entry.getInt("connect_timeout"));
  unsigned connectAttemptPeriod = static_cast<unsigned>(cluster_cfg_entry.getInt("connect_attempt_period"));
  _clusterCtrlMgr = new ClusterControllerMgr(peerHost,
                                             peerPort,
                                             connectTimeout,
                                             connectAttemptPeriod);

}

void
ClusterSubsystem::start()
{
  smsc_log_debug(_logger, "ClusterSubsystem::start::: try start ClusterControllerMgr");
  _clusterCtrlMgr->startup();
}

void
ClusterSubsystem::stop()
{
  smsc_log_debug(_logger, "ClusterSubsystem::start::: try shutdown ClusterControllerMgr");
  _clusterCtrlMgr->shutdown();
}

}}}
