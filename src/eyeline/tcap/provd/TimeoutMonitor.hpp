#ifndef __EYELINE_TCAP_PROVD_TIMEOUTMONITOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TIMEOUTMONITOR_HPP__

# include <sys/time.h>
# include <list>
# include <map>

# include "logger/Logger.h"
# include "core/buffers/XHash.hpp"
# include "core/threads/Thread.hpp"
# include "core/synchronization/EventMonitor.hpp"

# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TDialogueServiceData;

class TimeoutHandler {
public:
  virtual ~TimeoutHandler() {}
  virtual void handle() = 0;
};

class TimeoutMonitor : public smsc::core::threads::Thread, 
                       public utilx::Singleton<TimeoutMonitor> {
public:
  explicit TimeoutMonitor(unsigned int maxTimeoutValue);

  ~TimeoutMonitor();

  virtual int Execute();

  typedef unsigned int timeout_id_t;

  //  timeout_id_t schedule(unsigned int timeoutValue, TDialogueServiceData* dlgSvcData);
  timeout_id_t schedule(unsigned int timeoutValue, TimeoutHandler* timeoutHandler);
  void cancel(timeout_id_t timeoutId);
  void restart(unsigned int timeoutValue, timeout_id_t timeoutId);

private:
  smsc::logger::Logger* _logger;
  bool _isRunning;
  unsigned int _maxTimeoutValue;
  timeout_id_t _timeoutIdSourceGenerator;

  smsc::core::synchronization::EventMonitor _timeoutScheduleEvent;

  smsc::core::synchronization::Mutex _usedTimeoutsLock;
  typedef std::list<unsigned> used_timeouts_t;
  used_timeouts_t _usedTimeouts;

  struct TimeoutInfo {
    TimeoutInfo(timeout_id_t aTimeoutId,
                unsigned int timeoutValue,
                TimeoutHandler* aTimeoutHandler)
      : timeoutId(aTimeoutId), timeoutHandler(aTimeoutHandler)/*, timeoutLock(NULL)*/ {
      gettimeofday(&timeToExpire, 0);
      timeToExpire.tv_sec += timeoutValue;
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

    timeout_ref_t(unsigned int aTimeoutValue, active_timeouts_t::iterator anIterToActiveTimeout)
      : timeoutValue(aTimeoutValue), iterToActiveTimeout(anIterToActiveTimeout)
    {}

    unsigned int timeoutValue;
    active_timeouts_t::iterator iterToActiveTimeout;
  };

  smsc::core::synchronization::Mutex _timeoutRefsRegistryLock;
  smsc::core::buffers::XHash<timeout_id_t, timeout_ref_t> _timeoutRefsRgistry;

  timeout_id_t generateTimeoutId();

  void waitForNewScheduledTimeouts(int sleepInterval);

  void registerTimeoutId(timeout_id_t timeoutId,
                         unsigned int timeoutValue,
                         active_timeouts_t::iterator ins_iter);

  void unregisterTimeoutId(timeout_id_t timeoutId);
};

}}}

#endif
