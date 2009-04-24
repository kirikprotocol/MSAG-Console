/* ************************************************************************** *
 * Signalling Timers facility interfaces and helper classes defs.
 * ************************************************************************** */
#ifndef _CORE_SYNC_TIMEWATCHER_DEFS_HPP
#ident "@(#)$Id$"
#define _CORE_SYNC_TIMEWATCHER_DEFS_HPP

#include <inttypes.h>
//#include <sys/time.h>

namespace smsc {
namespace core {
namespace timers {

class TimeWatcherITF {
//NOTE1: It's assumed that interface implementations do not expose
//below methods, but possess method like following
//virtual TimerHdl CreateTimer(TimerListenerITF * listener, ...) = 0;
//
//NOTE2: TimerId 0 is reserved !!! 
public:
    enum Error {
        errOk = 0, errBadTimer, errBadTimeVal, errTimerState, errWatcherState
    };
    //result of timer signalling
    enum SignalResult {
        evtOk = 0       //event is succesfully handled
        , evtResignal   //event should be resignaled again
    };
    //
    virtual const char * IdStr(uint32_t tmr_id) const = 0;
    //
    virtual Error StartTimer(uint32_t tmr_id) = 0;
    //
    virtual Error StopTimer(uint32_t tmr_id) = 0;
    //
    virtual SignalResult SignalTimer(uint32_t tmr_id) = 0;
    //
    virtual Error RefTimer(uint32_t tmr_id) = 0;
    //releases timer on last unref
    virtual Error UnRefTimer(uint32_t tmr_id) = 0;

};

typedef TimeWatcherITF::Error TMError;

class TimerHdl {
protected:
    uint32_t          _timerId;
    TimeWatcherITF *  _watcher;

public:
    TimerHdl(uint32_t tmr_id = 0, TimeWatcherITF *  use_watcher = NULL)
        : _timerId(tmr_id), _watcher(use_watcher)
    {
        if (_watcher && _watcher->RefTimer(_timerId)) {
          _watcher = 0; _timerId = 0;
        }
    }
    TimerHdl(const TimerHdl & use_hdl)
        : _timerId(use_hdl._timerId), _watcher(use_hdl._watcher)
    {
        if (_watcher && _watcher->RefTimer(_timerId)) {
          _watcher = 0; _timerId = 0;
        }
    }
    ~TimerHdl()
    { 
        if (_watcher)
            _watcher->UnRefTimer(_timerId);
    }

    uint32_t Id(void) const { return _timerId; }
    const char * IdStr(void) const
    {
        return _watcher ? _watcher->IdStr(_timerId) : "unknown";
    }
    //
    TMError Start(void) const
    {
        return _watcher ? _watcher->StartTimer(_timerId) : TimeWatcherITF::errBadTimer;
    }
    //releases timer
    void Stop(void) const
    {
        if (_watcher)
            _watcher->StopTimer(_timerId);
    }
};

struct OPAQUE_OBJ {
    enum { objNone = 0, objUint, objPtr } kind;
    union {
        unsigned ui;
        void * ptr;
    } val;

    OPAQUE_OBJ() : kind(objNone)
    { val.ui = 0; }
    OPAQUE_OBJ(unsigned o_ui) : kind(objUint)
    { val.ui = o_ui; }
    OPAQUE_OBJ(void * o_ptr) : kind(objPtr)
    { val.ptr = o_ptr; }

    void clear(void)          { kind = objNone; }
    void setPtr(void * o_ptr) { kind = objPtr; val.ptr = o_ptr;  }
    void setUInt(unsigned o_ui) { kind = objUint; val.ui = o_ui;  }
};

class TimerListenerITF {
public:
    //NOTE: TimerListener shouldn't block while this call! It's recommended
    //to return TimeWatcherITF::evtResignal instead.
    virtual TimeWatcherITF::SignalResult
        onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj = NULL) = 0;
};

} //timers
} //core
} //smsc
#endif /* _CORE_SYNC_TIMEWATCHER_DEFS_HPP */

