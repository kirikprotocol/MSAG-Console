/* ************************************************************************** *
 * TimeWatchers service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TIME_WATCHER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TIME_WATCHER_HPP

#include "core/timers/TimeoutsMonitor.hpp"

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/tmwatch/TMWatcherDefs.hpp"

namespace smsc {
namespace inman {

class ICSTMWatcher : public ICServiceAC, public TimeWatcherIface {
protected:
  mutable smsc::core::synchronization::Mutex  mSync;
  smsc::core::timers::TimeoutsMonitor         mTmoMon;

  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  virtual smsc::core::synchronization::Mutex & _icsSync(void) const { return mSync; }
  //Initializes service verifying that all dependent services are inited
  virtual RCode _icsInit(void) { return ICServiceAC::icsRcOk; }
  //Starts service verifying that all dependent services are started
  virtual RCode _icsStart(void)
  {
    return mTmoMon.start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
  }
  //Stops service
  virtual void _icsStop(bool do_wait = false)
  {
    if (do_wait)
      mTmoMon.stop();
    else
      mTmoMon.stopNotify();
  }

public:
  explicit ICSTMWatcher(const ICServicesHostITF * svc_host, Logger * use_log = NULL)
      : ICServiceAC(ICSIdent::icsIdTimeWatcher, svc_host, use_log)
      , mTmoMon("0", use_log)
  {
    _icsState = ICServiceAC::icsStConfig;
  }
  virtual ~ICSTMWatcher()
  {
    ICSStop(true);
  }

  // -------------------------------------
  // ICServiceAC interface methods:
  // -------------------------------------
  //Returns TimeWatcherIface
  virtual void * Interface(void) const
  {
    return (TimeWatcherIface*)this;
  }

  // -------------------------------------
  // TimeWatcherIface interface methods:
  // -------------------------------------
  virtual TimerHdl createTimer(TimerListenerIface & tmr_lsr, const TimeSlice & use_tmo)
  {
    return mTmoMon.createTimer(tmr_lsr, use_tmo);
  }
  virtual void      reserveTimers(TimerUId num_tmr, uint16_t num_tmo = 0)
  {
    mTmoMon.reserveTimers(num_tmr);
    if (num_tmo)
      mTmoMon.reserveTimeouts(num_tmo);
  }
};

} //inman
} //smsc
#endif /* __INMAN_ICS_TIME_WATCHER_HPP */

