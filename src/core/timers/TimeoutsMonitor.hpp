/* ************************************************************************** *
 * MT Realtime timers monitoring facility.
 * TimeoutsMonitor:  timers monitor implementation, that handles expiration                        .
 * time in relative form.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMEOUTS_MONITOR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMEOUTS_MONITOR_HPP

#include "core/timers/TimeoutsQueue.hpp"
#include "core/timers/TimersMonitorCore.hpp"

namespace smsc {
namespace core {
namespace timers {

class TimeoutsMonitor : public TimersMonitorCore {
private:
  TimeoutsQueue mTmoQueue;

public:
  explicit TimeoutsMonitor(const char * use_id, Logger * use_log = NULL)
    : TimersMonitorCore(use_id, mTmoQueue, use_log)
  { }
  ~TimeoutsMonitor()
  { }

  //Reserves resources for handling of 'num_tmo' timeout values.
  void reserveTimeouts(uint16_t num_tmo)
  {
    smsc::core::synchronization::MutexGuard  grd(getSync());
    mTmoQueue.reserve(num_tmo);
  }

  TimerHdl createTimer(TimerListenerIface & tmr_lsr, const TimeSlice & use_tmo)
  {
    return allcTimer(tmr_lsr, ExpirationTime(use_tmo));
  }
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_TIMEOUTS_MONITOR_HPP */

