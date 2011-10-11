/* ************************************************************************** *
 * TimeWatcher service interface definition.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TIME_WATCHER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TIME_WATCHER_DEFS_HPP

#include "core/timers/TimersMonitorDefs.hpp"

namespace smsc {
namespace inman {

using smsc::core::synchronization::TimeSlice;
using smsc::core::timers::TimerUId;
using smsc::core::timers::TimerHdl;
using smsc::core::timers::TimerListenerIface;

class TimeWatcherIface {
public:
  virtual TimerHdl  createTimer(TimerListenerIface & tmr_lsr, const TimeSlice & use_tmo) = 0;
  virtual void      reserveTimers(TimerUId num_tmr, uint16_t num_tmo = 0) = 0;

protected:
  TimeWatcherIface()
  { }
  virtual ~TimeWatcherIface()
  { }
};


class TimerFact : public TimeSlice {
protected:
  TimeWatcherIface * mpMon;

public:
  explicit TimerFact(long use_tmo = 0, UnitType_e use_unit = tuSecs)
    : TimeSlice(use_tmo, use_unit), mpMon(0)
  { }
  ~TimerFact()
  { }

  bool empty(void) const { return mpMon == NULL; }

  void assign(long use_tmo, UnitType_e use_unit = tuSecs)
  {
    unitId = use_unit; tmoVal = use_tmo;
  }

  void init(TimeWatcherIface & p_mon, TimerUId num_tmrs)
  {
    (mpMon = &p_mon)->reserveTimers(num_tmrs);
  }

  TimerHdl createTimer(TimerListenerIface & tmr_lsr) const
  {
    return mpMon->createTimer(tmr_lsr, *this);
  }

  TimerFact & operator= (const TimeSlice & use_tmo)
  {
    *static_cast<TimeSlice*>(this) = use_tmo;
    return *this;
  }
};


} //inman
} //smsc
#endif /* __INMAN_ICS_TIME_WATCHER_DEFS_HPP */

