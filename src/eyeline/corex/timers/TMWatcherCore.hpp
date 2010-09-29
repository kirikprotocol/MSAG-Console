/* ************************************************************************** *
 * MT Realtime timers watching facility: single thread monitors timers 
 * expiration, and thread pool serves timers signalling.
 * ************************************************************************** */
#ifndef _CORE_TIMERS_TIMEWATCHER_CORE_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_TIMEWATCHER_CORE_HPP

#include "eyeline/corex/timers/ExpirationTime.hpp"
#include "eyeline/corex/timers/TimerNotifier.hpp"

namespace eyeline {
namespace corex {
namespace timers {

//TimeWatcher facility core functionality.
class TimeWatcherCore : protected TimeWatcherIface,
                        protected smsc::core::threads::Thread {
public:
  enum RunState_e {
    twrsStopped = 0
    , twrsStopping = 1
    , twrsRunning = 2
  };
  enum ShutdownReason_e {
      tmwUnexpected = -1  //unexpected fatal exception was caught
    , tmwStopped = 0      //normal shutdown
    , tmwError = 1        //fatal internal error
  };

  static const size_t _MAX_LOGNAME_LEN = sizeof("TmWT[]") + _MAX_IDENT_LEN;
  typedef smsc::core::buffers::FixedLengthString<_MAX_LOGNAME_LEN> Logname_t;

  TimeWatcherCore(const TimeWatcherIdent & use_id, ExpirationQueueIface & exp_queue,
                  Logger * uselog = NULL);
  virtual ~TimeWatcherCore();
  //
  const char * logId(void) const { return _logId.c_str(); }
  //
  void getTime(struct timespec & abs_time) const { TimeSlice::getRealTime(abs_time); }
  //
  void initTimers(uint32_t num_tmrs);
  //
  void initThreads(unsigned ini_threads, unsigned max_threads = 0);
  //
  bool isRunning(void) const;
  //
  bool Start(void);

  void StopNotify(void);

  //NOTE: upon timeout expiration forces all remaining timers to being reported as expired !
  void Stop(TimeSlice::UnitType_e time_unit, long use_tmo);

protected:
  class TimerHdlComposer : public TimerHdl {
  public:
    TimerHdlComposer()
      : TimerHdl()
    { }
    TimerHdlComposer(uint32_t tmr_id, TimeWatcherIface *  use_watcher)
      : TimerHdl(tmr_id, use_watcher)
    { }
    ~TimerHdlComposer()
    { }
  };

  TimerHdlComposer _createTimer(TimerListenerIface * tmr_listener,
                                const ExpirationTime & exp_time,
                                const OpaqueArg * opaque_obj = NULL);

  // -- -------------------------
  // -- Thread interface methods
  // -- -------------------------
  virtual int  Execute(void);

  // -- -------------------------
  // -- TimeWatcherIface methods
  // -- -------------------------
  //Returns TimeWatcher unique identifier string
  virtual const IdentStr_t & IdStr(void) const { return _ident; }
  //
  virtual const char * getTimerIdent(uint32_t tmr_id) const;
  //
  virtual TimerError_e startTimer(uint32_t tmr_id);
  //
  virtual TimerError_e stopTimer(uint32_t tmr_id);
  //
  virtual TimerError_e refTimer(uint32_t tmr_id);
  //releases timer on last unref
  virtual TimerError_e unRefTimer(uint32_t tmr_id);

private:
  using smsc::core::threads::Thread::Start; //hide it to avoid annoying CC warnings

  mutable smsc::core::synchronization::EventMonitor  _sync;
  volatile RunState_e     _runState;
  TimeSlice               _shdTmo;  //shutdown timeout

  IdentStr_t              _ident;
  Logname_t               _logId;
  Logger *                _logger;

  StopWatchStore          _swStore;
  TimerNotifier           _swNotifier;
  ExpirationQueueIface &  _expQueue;
};

} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_TIMEWATCHER_CORE_HPP */

