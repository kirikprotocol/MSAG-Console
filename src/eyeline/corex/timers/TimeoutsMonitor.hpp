/* ************************************************************************** *
 * MT Realtime timers watching facility: timewatcher implementation, that
 * monitors timers with expiration time based on same timeout value.
 * ************************************************************************** */
#ifndef _CORE_TIMERS_TIMEOUTS_MONITOR_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_TIMEOUTS_MONITOR_HPP

#include "eyeline/corex/timers/TimeoutsQueue.hpp"
#include "eyeline/corex/timers/TMWatcherCore.hpp"

namespace eyeline {
namespace corex {
namespace timers {

class TimeoutsMonitor : public TimeoutsMonitorIface, public TimeWatcherCore {
private:
  TimeoutsQueue _tmoQueue;

public:
  TimeoutsMonitor(const TimeWatcherIdent & use_id, Logger * use_log = NULL)
    : TimeWatcherCore(use_id, _tmoQueue, use_log)
  { }
  ~TimeoutsMonitor()
  { }

  // -------------------------------------------
  // -- TimeoutsMonitorIface methods
  // -------------------------------------------
  virtual TimerHdl CreateTimer(TimerListenerIface * tmr_listener, const TimeSlice & use_tmo,
                               const OpaqueArg * opaque_obj = NULL)
  {
    return _createTimer(tmr_listener, ExpirationTime(use_tmo.Value(), use_tmo.Units()), opaque_obj);
  }
};

} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_TIMEOUTS_MONITOR_HPP */

