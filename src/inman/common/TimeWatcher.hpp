#ident "$Id$"
#ifndef _SMSC_INMAN_SYNCHRONIZATION_HPP
#define _SMSC_INMAN_SYNCHRONIZATION_HPP

#include <list>

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

//NOTE: timevals should be normalized(microsecs adjusted) !
inline bool operator< (const struct timeval& tm1, const struct timeval& tm2)
{
    if (tm1.tv_sec > tm2.tv_sec)
        return false;
    if (tm1.tv_sec < tm2.tv_sec)
        return true;
    //equal secs, compare microsecs
    return tm1.tv_usec < tm2.tv_usec ? true : false;
}

inline bool operator>= (const struct timeval& tm1, const struct timeval& tm2)
{
    if (tm1.tv_sec > tm2.tv_sec)
        return true;
    if (tm1.tv_sec < tm2.tv_sec)
        return false;
    //equal secs, compare microsecs
    return tm1.tv_usec >= tm2.tv_usec ? true : false;
}


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
    OPAQUE_OBJ() : kind(objNone) { }
    void setPtr(void * o_ptr) { kind = objPtr; val.ptr = o_ptr;  }
    void setUInt(unsigned o_ui) { kind = objUint; val.ui = o_ui;  }
};

class TimerListenerITF;
class TimeNotifier;
class TimeWatcher;

class StopWatch {
public:
    //NOTE: the order of states is important!!!
    typedef enum { tmrIdle = 0, tmrInited, tmrActive, tmrAborted, tmrStopped, tmrExpired 
    } SWStatus;
    typedef enum { noErr = 0, errBadTimer = 1, errBadTimeVal = 2, errActiveTimer
    } TMError;

    void init(TimerListenerITF * listener, OPAQUE_OBJ * opaque_obj = NULL);
    void release(void);

    //timeout is either in millisecs or in microsecs
    TMError    start(long timeout, bool millisecs = true);
    TMError    start(struct timeval & abs_time);
    void       stop(void);

    const struct timeval & targetTime(void) const   { return tms;}
    SWStatus getStatus(void) const                  { return _tmrState; }
    unsigned getId(void) const                      { return _id; }

protected:
    friend class TimeWatcher;
    friend class TimeNotifier;

    StopWatch(TimeWatcher * owner, unsigned tm_id);
    ~StopWatch() {}

    void activate(const struct timeval & tm_val);
    void reset(void);
    void setStatus(SWStatus status);

    void signal(void);
    void signal(SWStatus status);

private:
    unsigned        _id;
    TimerListenerITF * _cb_event;
    OPAQUE_OBJ      _opaqueObj;   //externally defined object that is passed to listeners
    SWStatus        _tmrState;
    struct timeval  tms;
    Mutex           _sync;
    TimeWatcher *   _owner;
//    Logger *        logger;
};

class TimerListenerITF {
public:
    virtual void onTimerEvent(StopWatch* timer, OPAQUE_OBJ * opaque_obj = NULL) = 0;
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

    TimerID createTimer(TimerListenerITF * listener = NULL, OPAQUE_OBJ * opaque_obj = NULL);
    void    releaseTimer(TimerID tmr);

    //timeout is either in millisecs or in seconds
    StopWatch::TMError    startTimer(TimerID tmr, long timeout, bool millisecs = true);
    StopWatch::TMError    startTimer(TimerID tmr, struct timeval & abs_time);
    void              stopTimer(TimerID tmr);

private:
    Event           awake;
    unsigned        _lastId;
    bool            _running;
    EventMonitorPSX _sync;
    TimersLIST      started;    //started timers
    TimersLIST      signaled;   //timers are to signal
    TimersLIST      pool;       //pool of idle timers
    TimersLIST      assigned;   //initialized, but not started timers
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
/*

protected:
    friend class TimeNotifier;
    Event       awake;

*/
} //sync
} //inman
} //smsc
#endif /* _SMSC_INMAN_SYNCHRONIZATION_HPP */

