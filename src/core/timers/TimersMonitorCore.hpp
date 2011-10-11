/* ************************************************************************** *
 * MT Realtime timers watching facility: 
 * TimersMonitorCore - monitors timers expiration and performs its signaling
 * in separate threads.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMERS_MONITOR_CORE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMERS_MONITOR_CORE_HPP

#include "core/timers/TimerNotifier.hpp"
#include "core/timers/TimersMonitorDefs.hpp"
#include "core/timers/ExpirationQueueDefs.hpp"

namespace smsc {
namespace core {
namespace timers {

//TimeWatcher facility core functionality.
class TimersMonitorCore : public TimersMonitorIface
                        , protected TimerRefereeIface
                        , protected smsc::core::threads::Thread {
public:
  enum RunState_e {
    monStopped = 0x00
  , monStopping = 0x01  //no new timers are started, currently monitored are
                        //handle for a while.
  , monRunning = 0x02
  };
  enum ShutdownReason_e {
    tmwUnexpected = -1  //unexpected fatal exception was caught
  , tmwStopped = 0      //normal shutdown
  , tmwError = 1        //fatal internal error
  };

  virtual ~TimersMonitorCore();

  //
  bool isState(RunState_e tgt_state) const;

  //Reserves resources for fiven number of TimerFSM objects.
  void reserveTimers(TimerUId num_tmrs);
  //Sets maximum number of threads used for signalling of expired timers.
  //NOTE:  0 - turns off limit on threads number.
  void setMaxThreads(uint16_t max_threads) { mSwNtfr.init(1, max_threads); }

  //
  bool start(void);
  //Switches monitor to 'monStopping' state. 
  void stopNotify(void);
  //Finally stops timer monitoring. Waits for a given timeout and reports all
  //remaining timers as expired.
  //NOTE: If shutdown timeout isn't set the default one is used.
  void stop(const TimeSlice * use_tmo = NULL);

  // ------------------------------------------
  // -- TimersMonitorIface interface methods:
  // ------------------------------------------
  //Returns TimeWatcher unique identifier string
  virtual const IdentStr_t & getIdent(void) const { return mIdent; }
  //Returns handle of previously allocated timer.
  virtual TimerHdl  getTimer(TimerUId sw_uid) /*throw()*/;
  //Creates and initializes a TimerFSM object.
  //Returns handle to manage created timer.
  //NOTE: in case of failure return empty handle!!!
  virtual TimerHdl allcTimer(TimerListenerIface & tmr_lsr, const ExpirationTime & exp_time) /*throw()*/;

protected:
  TimersMonitorCore(const char * use_id, ExpirationQueueIface & exp_queue,
                    Logger * use_log = NULL);
  //
  const char * logId(void) const { return mIdent.c_str(); }

  smsc::core::synchronization::Mutex & getSync(void) const { return mSync; }
  // ------------------------------------------
  // -- TimerRefereeIface interface methods:
  // ------------------------------------------
  //Inserts given TimerLink to pool of monitored timers.
  virtual TimerFSM::ActivationResult_e enqueue(TimerLink & sw_link) /*throw()*/;
  //NOTE: It's a caller responsibility to ensure that given TimerLink was
  //      inserted previously !!!
  virtual void unlink(TimerLink & sw_link) /*throw()*/;

private:
  static const TimeSlice k_dfltSleepTmo;

  using smsc::core::threads::Thread::Start;
  // -- -------------------------
  // -- Thread interface methods
  // -- -------------------------
  virtual int  Execute(void);

  mutable smsc::core::synchronization::EventMonitor  mSync;
  volatile RunState_e     mRunState;
  const TimeSlice *       mpShtdTmo;    //notifier shutdown timeout
  char                    mIdPfx[TimersMonitorIface::k_maxIdPrefixLen + 1];
  IdentStr_t              mIdent;
  ExpirationQueueIface &  mExpQueue;
  Logger *                mLogger;
  TimersPool              mSwStore;
  TimerNotifier           mSwNtfr;
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_TIMERS_MONITOR_CORE_HPP */

