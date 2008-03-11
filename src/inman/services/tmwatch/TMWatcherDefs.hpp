#pragma ident "$Id$"
/* ************************************************************************** *
 * Signalling Timers facility interfaces and helper classes defs.
 * ************************************************************************** */
#ifndef _CORE_SYNC_TIMEWATCHER_DEFS_HPP
#define _CORE_SYNC_TIMEWATCHER_DEFS_HPP

#include <inttypes.h>
#include <sys/time.h>

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
        errOk = 0, errBadTimer, errBadTimeVal, errTimerState
    };
    //result of timer signalling
    enum SignalResult {
        evtOk = 0       //event is succesfully handled
        , evtResignal   //event should be resignaled again
    };
    //
    virtual const char * IdStr(uint32_t tmr_id) = 0;
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
    const uint32_t      timerId;
    TimeWatcherITF *    watcher;

public:
    TimerHdl(uint32_t tmr_id = 0, TimeWatcherITF *  use_watcher = NULL)
        : timerId(tmr_id), watcher(use_watcher)
    {
        if (watcher)
            watcher->RefTimer(timerId);
    }
    TimerHdl(const TimerHdl & use_hdl)
        : timerId(use_hdl.timerId), watcher(use_hdl.watcher)
    {
        if (watcher)
            watcher->RefTimer(timerId);
    }
    ~TimerHdl()
    { 
        if (watcher)
            watcher->UnRefTimer(timerId);
    }

    inline uint32_t Id(void) const { return timerId; }
    inline const char * IdStr(void)
    {
        return watcher ? watcher->IdStr(timerId) : "unknown";
    }
    //
    inline TMError Start(void)
    {
        return watcher ? watcher->StartTimer(timerId) : TimeWatcherITF::errBadTimer;
    }
    //releases timer
    inline void Stop(void)
    {
        if (watcher)
            watcher->StopTimer(timerId);
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
    virtual TimeWatcherITF::SignalResult
        onTimerEvent(TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj = NULL) = 0;
};

} //timers
} //core
} //smsc
#endif /* _CORE_SYNC_TIMEWATCHER_DEFS_HPP */

