/* ************************************************************************** *
 * MT Realtime timers monitoring facility: basic definitions and interfaces.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_MONITIR_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_MONITIR_DEFS_HPP

#include "core/timers/TimerFSM.hpp"

namespace smsc {
namespace core {
namespace timers {

//Timer handle - addresses the StopWatch maintained by generic TimersMonitor.
class TimerHdl {
public:
  enum Error_e {
    errOk = 0, errBadTimer, errBadTimeVal, errTimerState, errMonitorState
  };

  TimerHdl()
  { }
  TimerHdl(const TimerHdl & cp_obj) : mSwRef(cp_obj.mSwRef)
  { }
  ~TimerHdl()
  {
    release();
  }

  TimerHdl & operator=(const TimerHdl & cp_obj)
  {
    if (this != &cp_obj) {
      release();
      mSwRef = cp_obj.mSwRef;
    }
    return *this;
  }

  void release(void) { mSwRef.release(); }
  bool empty(void) const { return mSwRef.empty(); }
  //Returns true if referenced TimerFSM has a UId equal to given one.
  bool isTimerUId(TimerUId tmr_uid) const
  {
    return mSwRef.empty() ? false : mSwRef->getUIdx();
  }

  //Returns timer statistics string or "NULL" in case of uninitialized timer.
  const char * getStatStr(void) const;
  //Starts timer monitoring.
  Error_e start(void) const;
  //Stops timer expiration monitoring. If timer's expiration time is of timeout form,
  //timer may be started again.
  //NOTE: 'errTimerState' is returned if timer is currently signalling
  Error_e stop(void) const;

private:
  TimerRef  mSwRef;

  static Error_e resultToErr(TimerFSM::ActivationResult_e res_val);

protected:
  friend class TimersMonitorIface; //forward declaration
  //
  explicit TimerHdl(const TimerRef & p_tmr) : mSwRef(p_tmr)
  { }
};
typedef TimerHdl::Error_e   TimerError_e;

//Generic Timers monitor interface.
class TimersMonitorIface {
public:
  static const size_t k_maxIdPrefixLen = 16;
  static const size_t k_maxLogIdLen = sizeof("TmWT[%s]") + k_maxIdPrefixLen;
  typedef smsc::core::buffers::FixedLengthString<k_maxLogIdLen + 1> IdentStr_t;

  // ------------------------------------------
  // -- TimersMonitorIface interface methods:
  // ------------------------------------------
  //Returns TimeWatcher unique identifier string
  virtual const IdentStr_t & getIdent(void) const /*throw()*/= 0;
  //Returns handle of previously allocated timer.
  virtual TimerHdl  getTimer(TimerUId sw_uid) /*throw()*/= 0;
  //Creates and initializes a TimerFSM object. Returns its handle.
  //NOTE: in case of failure return empty handle!!!
  virtual TimerHdl allcTimer(TimerListenerIface & tmr_lsr, const ExpirationTime & exp_time) /*throw()*/= 0;

protected:
  static TimerHdl newTimerHdl(const TimerRef & p_tmr) { return TimerHdl(p_tmr); }

  TimersMonitorIface()
  { }
  virtual ~TimersMonitorIface() //forbid interface destruction
  { }
};
typedef TimersMonitorIface::IdentStr_t      TimersMonitorIdent;

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_MONITIR_DEFS_HPP */

