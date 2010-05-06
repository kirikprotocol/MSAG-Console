#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_RECONNECTOR_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_RECONNECTOR_HPP__

# include <time.h>
# include <set>
# include <list>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Link.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class Reconnector : public utilx::Singleton<Reconnector>,
                    public smsc::core::threads::Thread {
public:
  void initialize(ConnectMgr* conn_mgr, unsigned reconnect_attempt_period) {
    _connMgr = conn_mgr; _reconnectAttemptPeriod = reconnect_attempt_period;
  }

  virtual int Execute();
  void shutdown();

  void schedule(const LinkPtr& broken_link);

  void setReconnectPeriod(unsigned reconnect_attempt_period) {
    _reconnectAttemptPeriod = reconnect_attempt_period;
  }

private:
  Reconnector()
  : _logger(smsc::logger::Logger::getInstance("io_dsptch")),
    _shutdownInProgress(false), _connMgr(NULL), _reconnectAttemptPeriod(0)
  {}
  friend class utilx::Singleton<Reconnector>;

  struct broken_link_info {
    broken_link_info(const LinkPtr& broken_link=LinkPtr())
      : brokenLink(broken_link), scheduledProcessingTime(time(NULL))
    {}
    LinkPtr brokenLink;
    time_t scheduledProcessingTime;
  };

  void tryReconnect(const broken_link_info& broken_link_info);
  void moveFailedLinkToNotReconnectedList(const broken_link_info& broken_link_info);
  void waitForPeriodAndGetNewBrokenLinks(bool sleep_infinitiy);

  smsc::logger::Logger* _logger;
  ConnectMgr* _connMgr;
  bool _shutdownInProgress;

  typedef std::set<LinkId> sched_linkids_t;
  std::set<LinkId> _idsForAlreadyScheduledLinks;
  std::list<broken_link_info> _scheduledLinks, _brokenLinksInProcessing, _notReconnectedLinks;
  smsc::core::synchronization::EventMonitor _scheduledLinksLock;
  smsc::core::synchronization::Mutex _alreadyScheduledLinkIdsLock;
  int _reconnectAttemptPeriod;
};

}}}}

#endif
