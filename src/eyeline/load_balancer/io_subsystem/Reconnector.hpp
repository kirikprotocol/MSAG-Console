#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_RECONNECTOR_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_RECONNECTOR_HPP__

# include <set>
# include <list>
# include <sys/types.h>
# include <time.h>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "core/synchronization/EventMonitor.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/SetOfFailedConnections.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class Reconnector : public smsc::core::threads::Thread {
public:
  Reconnector(IOProcessor& io_processor, int reconnect_attemptPeriod);
  virtual int Execute();
  void shutdown();
  void addFailedConnections(SetOfFailedConnections* failed_conns_set);
  void cancelReconnectionAttempts(const LinkId& link_set_id);

protected:
  struct failed_conn_info {
    failed_conn_info(SetOfFailedConnections* a_failed_conns=NULL)
      : failedConns(a_failed_conns), scheduledProcessingTime(time(NULL))
    {}
    SetOfFailedConnections* failedConns;
    time_t scheduledProcessingTime;
  };

  void tryReconnect(failed_conn_info& next_failed_connset_info);
  void addFailedConnSetToListHead(SetOfFailedConnections* failed_conns_set);
  void waitForPeriodAndGetNewFailedConnections(bool sleep_infinitiy);

private:
  smsc::logger::Logger* _logger;
  IOProcessor& _ioProcessor;
  bool _shutdownInProgress;

  std::list<failed_conn_info> _scheduledSets, _failedSetsInProcessing;
  smsc::core::synchronization::EventMonitor _scheduledSetsLock;

  std::set<LinkId> _scheduledCanceledLinksets, _canceledLinksets;

  int _reconnectAttemptPeriod;
};

}}}

#endif
