#pragma ident "$Id$"
#ifndef _SMSC_INMAN_SYNCHRONIZATION_HPP
#define _SMSC_INMAN_SYNCHRONIZATION_HPP

#include <list>

#include "inman/common/TimeOps.hpp"
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"

using smsc::logger::Logger;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::EventMonitor;

namespace smsc {
namespace inman {
namespace sync {

class EventMonitorPSX : public EventMonitor {
public:
    int wait(struct timeval upto)
    {
    #ifdef linux
        struct timespec tv;
    #else
        timestruc_t tv;
    #endif
        tv.tv_sec = upto.tv_sec;
        tv.tv_nsec = upto.tv_usec * 1000;
        return pthread_cond_timedwait(&event, &mutex, &tv);
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

    void setPtr(void * o_ptr) { kind = objPtr; val.ptr = o_ptr;  }
    void setUInt(unsigned o_ui) { kind = objUint; val.ui = o_ui;  }
};

class TimerListenerITF;
class TimeNotifier;
class TimeWatcher;

class StopWatch {
public:
    //NOTE: the order of states is important!!!
    typedef enum { tmrNone = 0, tmrExpired, tmrAborted, tmrStopped
    } SWStatus;
    typedef enum { tmrIdle = 0, tmrInited, tmrActive, tmrIsToSignal, tmrSignaled
    } SWState;

    typedef enum { errNoErr = 0, errBadTimer = 1, errBadTimeVal = 2, errActiveTimer
    } TMError;

    //timeout is either in millisecs or in microsecs
    TMError start(long timeout, bool millisecs = true);
    TMError start(const struct timeval & abs_time);
    //stops timer (timer ready for reusing)
    void    stop(void);
    //releases timer
    void    release(void);

    const struct timeval & targetTime(void) const   { return _tms;}
    SWStatus getStatus(void) const                  { return _result; }
    unsigned getId(void) const                      { return _id; }

protected:
    StopWatch(TimeWatcher * owner, unsigned tm_id);
    ~StopWatch() {}

    friend class TimeWatcher;
    //if multi_run is set, then timer may be reused(started) several times
    //otherwise timer is destoyed on signalling or stopping.
    void init(TimerListenerITF * listener, OPAQUE_OBJ * opaque_obj = NULL,
              bool multi_run = false);
    void signalStatus(SWStatus status);

    friend class TimeNotifier;
    int signal(void);

private:
    void reset(void);
    void deactivate(bool doRelease);

    unsigned        _id;
    bool            _multiRun;  //auto release on signalling
    TimerListenerITF * _cb_event;
    OPAQUE_OBJ      _opaqueObj;   //externally defined object that is passed to listeners
    SWStatus        _result;
    SWState         _tmrState;
    struct timeval  _tms;
    Mutex           _sync;
    Event           _sigEvent;
    TimeWatcher *   _owner;
//    Logger *        logger;
};

class TimerListenerITF {
public:
    //Returning the non-zero value listener requests resignaling of event
    virtual short onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj = NULL) = 0;
};

typedef std::list<StopWatch*> TimersLIST;

typedef StopWatch * TimerID;

class TimeWatcher : public Thread {
public:
    typedef enum {
        tmwUnexpected = -1, //unexpected fatal exception was caught
        tmwStopped = 0,     //normal shutdown
        tmwError = 1        //
    } ShutdownReason;

    TimeWatcher(Logger * uselog = NULL);
    ~TimeWatcher();

    static void getTime(struct timeval & abs_time);

    int  Execute(void);
    //NOTE: forces all remaining timers to being reported as aborted (not expired)!
    void Stop(void);
    bool isRunning(void);

    //if multi_run is set, then timer may be reused(started) several times
    //otherwise timer is destoyed on signalling or stopping.
    TimerID createTimer(TimerListenerITF * listener = NULL, OPAQUE_OBJ * opaque_obj = NULL,
                        bool multi_run = false);

protected:
    friend class StopWatch;
    StopWatch::TMError activateTimer(TimerID tmr);
    //discharges timer, moving it to pool
    void dischargeTimer(TimerID tmr);
    //deactivates assigned timer
    void deactivateTimer(TimerID tmr);

private:
    Event           awake;
    unsigned        _lastId;
    bool            _running;
    EventMonitorPSX _sync;
    TimersLIST      started;    //started timers
    TimersLIST      signaled;   //timers are to signal
    TimersLIST      assigned;   //initialized, but not started timers
    TimersLIST      pool;       //pool of idle timers
    TimeNotifier *  ntfr;
    Logger *        logger;
};


class TimeNotifier : public Thread {
public:
    TimeNotifier(TimeWatcher * master, Logger * uselog = NULL);
    ~TimeNotifier() {};
    
    void signalTimer(StopWatch* tmr);
    void signalTimers(TimersLIST& tmm, StopWatch::SWStatus status);

    int Execute();
    void Stop(void);
    bool isRunning(void);

protected:
    bool            _running;
    Event           awaked, stopped;
    TimeWatcher *   watcher;
    EventMonitor    _sync;
    TimersLIST      timers;
    Logger *        logger;
};

} //sync
} //inman
} //smsc
#endif /* _SMSC_INMAN_SYNCHRONIZATION_HPP */

