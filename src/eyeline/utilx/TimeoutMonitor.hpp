#ifndef __EYELINE_UTILX_TIMEOUTMONITOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_UTILX_TIMEOUTMONITOR_HPP__

# include <sys/time.h>
# include <list>
# include <map>

# include "logger/Logger.h"
# include "core/buffers/XHash.hpp"
# include "core/threads/Thread.hpp"
# include "core/synchronization/EventMonitor.hpp"

namespace eyeline {
namespace utilx {

class TimeoutHandler {
public:
  virtual ~TimeoutHandler() {}
  virtual void handle() = 0;
};

class TimeoutMonitor : public smsc::core::threads::Thread {
public:
  TimeoutMonitor(unsigned timeout_id_seed=0);
  virtual ~TimeoutMonitor();

  virtual int Execute();

  typedef unsigned int timeout_id_t;

  void initialize(unsigned int max_timeout_value);

  timeout_id_t schedule(unsigned int timeout_value, TimeoutHandler* timeout_handler);
  bool cancel(timeout_id_t timeout_id);
  void restart(unsigned int timeout_value, timeout_id_t timeout_id);

  void shutdown();
  bool stopped() const { return _stopped; }

private:
  smsc::logger::Logger* _logger;
  bool _isRunning, _shutdownInProgress, _stopped;
  unsigned int _maxTimeoutValue;
  timeout_id_t _timeoutIdSourceGenerator;

  smsc::core::synchronization::EventMonitor _timeoutScheduleEvent;

  smsc::core::synchronization::Mutex _usedTimeoutsLock;
  typedef std::list<unsigned> used_timeouts_t;
  used_timeouts_t _usedTimeouts;

  struct TimeoutInfo {
    TimeoutInfo(timeout_id_t timeout_id,
                unsigned int timeout_value,
                TimeoutHandler* timeout_handler)
      : timeoutId(timeout_id), timeoutHandler(timeout_handler)/*, timeoutLock(NULL)*/ {
      gettimeofday(&timeToExpire, 0);
      timeToExpire.tv_sec += timeout_value;
    }

    timeout_id_t timeoutId;
    struct timeval timeToExpire;
    TimeoutHandler* timeoutHandler;
  };

  typedef std::list<TimeoutInfo> active_timeouts_t;

  struct TimeoutsEntryInfo {
    TimeoutsEntryInfo() : activeTimeouts(NULL) {}

    used_timeouts_t::iterator iterToUsedTimeouts;
    active_timeouts_t* activeTimeouts;
    smsc::core::synchronization::Mutex entryLock;
  };

  TimeoutsEntryInfo* _knownTimeouts;

  struct timeout_ref_t {
    timeout_ref_t()
      : timeoutValue(0)
    {}

    timeout_ref_t(unsigned int timeout_value, active_timeouts_t::iterator iter_to_active_timeout)
      : timeoutValue(timeout_value), iterToActiveTimeout(iter_to_active_timeout)
    {}

    unsigned int timeoutValue;
    active_timeouts_t::iterator iterToActiveTimeout;
  };

  smsc::core::synchronization::Mutex _timeoutRefsRegistryLock;
  smsc::core::buffers::XHash<timeout_id_t, timeout_ref_t> _timeoutRefsRgistry;

  timeout_id_t generateTimeoutId();

  void waitForNewScheduledTimeouts(int sleep_interval);

  void registerTimeoutId(timeout_id_t timeout_id,
                         unsigned int timeout_value,
                         active_timeouts_t::iterator ins_iter);

  void unregisterTimeoutId(timeout_id_t timeout_id);
};

}}

#endif
