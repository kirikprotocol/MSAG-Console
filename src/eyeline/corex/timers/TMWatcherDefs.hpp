/* ************************************************************************** *
 * MT Realtime timers watching facility: basic definitions and interfaces.
 * ************************************************************************** */
#ifndef _CORE_TIMERS_TIMEWATCHER_DEFS_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_TIMEWATCHER_DEFS_HPP

#include <inttypes.h>
#include "core/synchronization/TimeSlice.hpp"
#include "core/buffers/FixedLengthString.hpp"

#define TIMEWATCHER_DFLT_LOGGER "smsc.core.timers"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::core::synchronization::TimeSlice;

//TimeWatcher internal interface. It's assumed that TimeWatcher implementations
//do not expose below methods, but possess method like following
//  virtual TimerHdl CreateTimer(TimerListenerIface * listener, ...) = 0;
//
class TimeWatcherIface {
protected:
  virtual ~TimeWatcherIface() //forbid interface destruction
  { }

public:
  static const size_t _MAX_IDENT_LEN = 32;
  typedef smsc::core::buffers::FixedLengthString<_MAX_IDENT_LEN> IdentStr_t;

  //String representation of Timer Unique identifier. Concatenates info about
  //monitoring TimeWatcher, timer ordinal numbver, timer usage statistics.
  static const size_t _MAX_TIMER_UID_LEN = _MAX_IDENT_LEN + 2*sizeof(uint32_t)*3 + 4;
  typedef smsc::core::buffers::FixedLengthString<_MAX_TIMER_UID_LEN>  TimerIdent_t;

  enum Error_e {
    errOk = 0, errBadTimer, errBadTimeVal, errTimerState, errWatcherState
  };
  //result of timer signalling
  enum SignalResult_e {
    evtOk = 0       //event is succesfully handled
    , evtResignal   //event should be resignaled again
  };

  //Returns TimeWatcher unique identifier string
  virtual const IdentStr_t & IdStr(void) const = 0;

  //Returns timer unique identifier string or NULL in case of unknown timer.
  virtual const char * getTimerIdent(uint32_t tmr_id) const = 0;
  //
  virtual Error_e startTimer(uint32_t tmr_id) = 0;
  //
  virtual Error_e stopTimer(uint32_t tmr_id) = 0;
  //
  virtual Error_e refTimer(uint32_t tmr_id) = 0;
  //releases timer on last unref
  virtual Error_e unRefTimer(uint32_t tmr_id) = 0;
};

typedef TimeWatcherIface::Error_e         TimerError_e;
typedef TimeWatcherIface::IdentStr_t      TimeWatcherIdent;
typedef TimeWatcherIface::TimerIdent_t    TimerIdent;
typedef TimeWatcherIface::SignalResult_e  TimerSignalResult_e;


//Timer handle - addresses the timer maintained by TimeWatcher. 
//All timer related operations are perfomed using such objects.
class TimerHdl {
protected:
  uint32_t          _timerId;
  TimeWatcherIface *  _watcher;

  TimerHdl(uint32_t tmr_id, TimeWatcherIface *  use_watcher)
    : _timerId(tmr_id), _watcher(use_watcher)
  {
    if (_watcher && _watcher->refTimer(_timerId)) {
      _watcher = 0; _timerId = 0;
    }
  }

public:
  TimerHdl() : _timerId(0), _watcher(0)
  { }
  TimerHdl(const TimerHdl & use_hdl)
  {
    if (_watcher)
        _watcher->unRefTimer(_timerId);
    _watcher = use_hdl._watcher;
    _timerId = use_hdl._timerId;
    if (_watcher && _watcher->refTimer(_timerId)) {
      _watcher = 0; _timerId = 0;
    }
  }
  ~TimerHdl()
  { 
    if (_watcher)
        _watcher->unRefTimer(_timerId);
  }

  bool empty(void) const { return _watcher == 0; }

  //Returns timer unique identifier string or NULL in case of uninitialized timer.
  const char * getIdent(void) const
  {
    return _watcher ? _watcher->getTimerIdent(_timerId) : NULL;
  }
  //Starts timer monitoring.
  TimerError_e start(void) const
  {
    return _watcher ? _watcher->startTimer(_timerId) : TimeWatcherIface::errBadTimer;
  }
  //Stops timer monitoring.
  //NOTE: 'errTimerState' is returned if timer is currently signalling
  TimerError_e stop(void) const
  {
    return _watcher ? _watcher->stopTimer(_timerId) : TimeWatcherIface::errOk;
  }
};

//Helper class. Just an opaque argument, that may be of either unsigned
//integer type or void pointer.
class OpaqueArg {
public:
  enum Kind_e { objPtr = 0, objUint };

  OpaqueArg(void * o_ptr = 0) : _kind(objPtr)
  {
    _val.ptr = o_ptr;
  }
  OpaqueArg(unsigned o_ui) : _kind(objUint)
  {
    _val.ui = o_ui;
  }

  bool isUInt(void) const { return _kind == objUint; }
  bool isPtr(void) const { return _kind == objPtr; }
  bool empty(void) const { return (_kind == objPtr) && !_val.ptr; }

  void * getPtr(void) const { return _val.ptr; }
  uint32_t getUInt(void) const { return _val.ui; }

  void clear(void)          { _kind = objPtr; _val.ptr = 0; }
  void setPtr(void * o_ptr) { _kind = objPtr; _val.ptr = o_ptr;  }
  void setUInt(uint32_t o_ui) { _kind = objUint; _val.ui = o_ui;  }

private:
  Kind_e _kind;
  union {
    uint32_t  ui;
    void *    ptr;
  } _val;
};

//Timer expiration signal handler interface
class TimerListenerIface {
protected:
  virtual ~TimerListenerIface() //forbid interface destruction
  { }

public:
  //NOTE: Though TimerListener is allowed to block for a long time while this call,
  //It's recommended to return TimeWatcherIface::evtResignal if TimerListener unable
  //to serve immediately timer signal. In that case timer will signal again later.
  virtual TimerSignalResult_e
    onTimerEvent(const TimerIdent & tm_uid, const OpaqueArg * opaque_arg = NULL) = 0;
};


class TimeoutsMonitorIface {
protected:
  virtual ~TimeoutsMonitorIface();

public:
  // -------------------------------------------
  // -- TimeoutsMonitorIface methods
  // -------------------------------------------
  virtual TimerHdl CreateTimer(TimerListenerIface * tmr_listener, const TimeSlice & use_tmo,
                               const OpaqueArg * opaque_obj = NULL) = 0;
};


} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_TIMEWATCHER_DEFS_HPP */

