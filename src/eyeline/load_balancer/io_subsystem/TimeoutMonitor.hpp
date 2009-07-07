#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_TIMEOUTMONITOR_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_TIMEOUTMONITOR_HPP__

# include <sys/types.h>
# include <map>
# include <set>
# include <string>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/Event.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/load_balancer/io_subsystem/TimeoutEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class TimeoutMonitor : public smsc::core::threads::Thread,
                       public utilx::Singleton<TimeoutMonitor> {
public:
  void shutdown();
  virtual int Execute();

  typedef std::string timeout_id_t;

  void scheduleTimeout(timeout_id_t timeout_id,
                       unsigned int timeout_value,
                       TimeoutEvent* timeout_event);

  void cancelTimeout(timeout_id_t timeout_id);
private:
  TimeoutMonitor()
  : _logger(smsc::logger::Logger::getInstance("io_susbsystem")),
    _shutdownInProgress(false)
  {}
  friend class utilx::Singleton<TimeoutMonitor>;

  smsc::logger::Logger* _logger;
  bool _shutdownInProgress;

  struct TimeoutEntry {
    TimeoutEntry(time_t time_to_expire, TimeoutEvent* event_to_fire,
                 timeout_id_t timeout_id)
    : timeToExpire(time_to_expire), eventToFire(event_to_fire),
      timeoutId(timeout_id)
    {}

    bool operator< (const TimeoutEntry& rhs) const {
      return timeToExpire < rhs.timeToExpire;
    }

    time_t timeToExpire;
    TimeoutEvent* eventToFire;
    timeout_id_t timeoutId;
  };

  typedef std::set<TimeoutEntry> timeouts_sched_time_t;
  timeouts_sched_time_t _timeoutExpirationTimes;

  typedef std::map<timeout_id_t, timeouts_sched_time_t::iterator> registered_timeouts_t;
  registered_timeouts_t _registeredTimeouts;

  smsc::core::synchronization::Mutex _lock;
  smsc::core::synchronization::Event _newTimeoutEvent;
};

}}}

#endif
