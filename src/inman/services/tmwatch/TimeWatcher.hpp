#pragma ident "$Id$"
/* ************************************************************************** *
 * Signalling Timers Watcher implementation.
 * ************************************************************************** */
#ifndef _CORE_SYNC_TIMEWATCHER_HPP
#define _CORE_SYNC_TIMEWATCHER_HPP

#include <list>
#include <set>
#include <map>
#include <vector>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "inman/common/TimeOps.hpp"
#include "inman/common/ObjRegistryT.hpp"
using smsc::util::POBJRegistry_T;

#include "inman/services/tmwatch/TMWatcherDefs.hpp"

#define TIMEWATCHER_DFLT_LOGGER "smsc.core.timers"

namespace smsc {
namespace core {
namespace timers {

using smsc::logger::Logger;
using smsc::core::threads::Thread;
//using smsc::core::synchronization::Event;
using smsc::core::synchronization::EventMonitor;

#ifdef linux
#define NANOTIME_STRUCT struct timespec
#else
#define NANOTIME_STRUCT timestruc_t
#endif

typedef std::list<TimerHdl> TimersLIST;

//StopWatch target time
class SWTgtTime {
public:
    enum ValueType {
        valTmoSecs = 0  //relative, timeout in seconds
        , valTmoUSecs   //relative, timeout in microseconds
        , valTime       //absolute time
    };

protected:
    ValueType    kind;
    struct  timeval tms;

public:
    SWTgtTime(const struct timeval & abs_time)
        : kind(valTime), tms(abs_time)
    { }
    //timeout is either in millisecs or seconds
    SWTgtTime(long timeout = 0, bool unit_mlsecs = false)
    {
        if (unit_mlsecs) {
            kind = valTmoUSecs;
            tms.tv_sec = timeout/1000L;
            tms.tv_usec = (timeout % 1000L)*1000L;
        } else {
            kind = valTmoSecs;
            tms.tv_sec = timeout;
            tms.tv_usec = 0;
        }
    }
    ~SWTgtTime()
    { }

    inline ValueType Type(void) const { return kind; }
    inline const struct timeval & Value(void) const { return tms; }

    struct timeval adjust(const struct timeval & cur_time) const
    {
        struct timeval tgt_time = tms;
        if (kind != valTime) {
            tgt_time.tv_sec += cur_time.tv_sec;
            tgt_time.tv_usec += cur_time.tv_usec;
            //normalize microsecs
            if (tgt_time.tv_usec > 1000000L) {
                tgt_time.tv_sec += tgt_time.tv_usec / 1000000L;
                tgt_time.tv_usec = tgt_time.tv_usec % 1000000L;
            }
        }
        return tgt_time;
    }

    inline bool operator== (const SWTgtTime & obj2) const
    {
        return ((kind == obj2.kind) && (tms == obj2.tms));
    }

    bool operator< (const SWTgtTime & obj2) const
    {
        if (kind == valTmoSecs)
            return tms.tv_sec < obj2.tms.tv_sec;
        if (kind == valTmoUSecs)
            return tms.tv_usec < obj2.tms.tv_usec;
        return tms < obj2.tms;
    }
};


class SWQueueITF {
public:
    class SWIdTime {
    public:
        uint32_t       swId;
        struct timeval swTms;

        SWIdTime() : swId(0)
        { swTms.tv_sec = swTms.tv_usec = 0; }

        SWIdTime(const struct timeval & use_tmv, uint32_t use_id)
            : swId(use_id), swTms(use_tmv)
        { }
        ~SWIdTime()
        { }

        inline bool operator< (const SWIdTime & obj2) const
        {
            return swTms < obj2.swTms;
        }
    };

    virtual bool empty(void) const = 0;
    virtual const SWIdTime & front(void) = 0;
    virtual void add(const SWIdTime & use_val) = 0;
    virtual bool remove(uint32_t sw_id) = 0;
    virtual void pop_front(void) = 0;
};

//Indexed multiset, elements are sorted upon insertion
//Container for arbitrary expiration times.
class SWQueueMSet : public SWQueueITF {
protected:
    typedef std::multiset<SWIdTime> SWQueue;
    typedef std::map<uint32_t, SWQueue::iterator> SWQIdx;

    SWQueue swQueue;
    SWQIdx  swQIdx;

public:
    SWQueueMSet()
    { }
    ~SWQueueMSet()
    { }

    inline bool empty(void) const { return swQueue.empty(); }
    inline const SWIdTime & front(void) { return *swQueue.begin(); }

    void add(const SWIdTime & use_val)
    {
        SWQueue::iterator it = swQueue.insert(use_val);
        swQIdx.insert(SWQIdx::value_type(use_val.swId, it));
    }
    bool remove(uint32_t sw_id)
    {
        SWQIdx::iterator it = swQIdx.find(sw_id);
        if (it != swQIdx.end()) {
            swQueue.erase(it->second);
            swQIdx.erase(it);
            return true;
        }
        return false;
    }
    void pop_front(void)
    {
        SWQueue::iterator it = swQueue.begin();
        swQIdx.erase(it->swId);
        swQueue.erase(it);
    }
};

//Indexed FIFO list
//Container for expiration times based on same timeout
class SWQueueList : public SWQueueITF {
protected:
    typedef std::list<SWIdTime> SWQueue;
    typedef std::map<uint32_t, SWQueue::iterator> SWQIdx;

    SWQueue swQueue;
    SWQIdx  swQIdx;

public:
    SWQueueList()
    { }
    ~SWQueueList()
    { }

    inline bool empty(void) const { return swQueue.empty(); }
    inline const SWIdTime & front(void) { return *swQueue.begin(); }

    void add(const SWIdTime & use_val)
    {
        swQueue.push_back(use_val);
        swQIdx.insert(SWQIdx::value_type(use_val.swId, --swQueue.end()));
    }

    bool remove(uint32_t sw_id)
    {
        SWQIdx::iterator it = swQIdx.find(sw_id);
        if (it != swQIdx.end()) {
            swQueue.erase(it->second);
            swQIdx.erase(it);
            return true;
        }
        return false;
    }
    void pop_front(void)
    {
        SWQueue::iterator it = swQueue.begin();
        swQIdx.erase(it->swId);
        swQueue.erase(it);
    }
};


//TimeWatcher core functionality.
class TimeWatcherAC : TimeWatcherITF, Thread {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

    class StopWatch : public Mutex {
    public:
        typedef enum { //reverse order of FSM states
            swIsToSignal = 0, swActive, swInited, swIdle
        } SWState;

    private:
        const uint32_t  _id;
        uint32_t        _refNum;    //number of active handles
        TimeWatcherITF *_owner;
        std::string &   _ownerId;
        SWState         _swState;   //current stopwatch FSM state
        SWState         _nextState; //FSM state the stopwatch ought to
                                    //switch to after signalling
        uint32_t        _usage;
        SWTgtTime       _tgtTime;
        std::string     _idStr;

        TimerListenerITF *  _eventHdl;
        OPAQUE_OBJ          _opaqueObj;   //externally defined object that
                                          //is passed to listeners

        inline void mkIdent(void)
        {
            char buf[2*sizeof(uint32_t)*3 + 4]; //":%u{%u}"
            snprintf(buf, sizeof(buf)-1, ":%u{%u}", _id, _usage);
            _idStr = _ownerId; _idStr += buf;
        }

    public:
        inline uint32_t Id(void) const { return _id; }
        inline const char * IdStr(void) const { return _idStr.c_str(); }
        inline SWState State(void) const { return _swState; }

        StopWatch(uint32_t tm_id, TimeWatcherITF * use_owner, std::string & owner_id)
            : _id(tm_id), _owner(use_owner), _ownerId(owner_id)
            , _eventHdl(0), _usage(0), _swState(swIdle), _nextState(swIsToSignal)
            , _refNum(0)
        {
            mkIdent();
        }
        ~StopWatch()
        { }

        inline struct timeval tgtTime(const struct timeval & cur_time) const
        {
            return _tgtTime.adjust(cur_time);
        }

        //switches FSM to swIsToSignal
        bool setToSignal(void)
        {
            MutexGuard grd(*this);
            if (_swState != swActive)
                return false;
            _swState = swIsToSignal;
            return true;
        }
        // -- ******************************************************************** --
        // -- NOTE: all next FSM switching methods require StopWatch being locked !!!
        // -- ******************************************************************** --
        bool Ref(void)
        {
            if (!++_refNum) { //too much refs !!!
                --_refNum;
                return false;
            }
            return true;
        }
        bool UnRef(void)
        {
            if (_refNum)
                --_refNum;
            return !_refNum;
        }
        //switches FSM to swInited
        bool init(TimerListenerITF * listener, const SWTgtTime & tgt_time,
                  const OPAQUE_OBJ * opaque_obj = NULL)
        {
            if (_swState != swIdle)
                return false;
            _eventHdl = listener;
            if (opaque_obj)
                _opaqueObj = *opaque_obj;
            else
                _opaqueObj.clear();
            
            _swState = swInited;
            ++_usage;
            _tgtTime = tgt_time;
            mkIdent();
            return true;
        }
        //switches FSM to swActive
        bool activate(void)
        {
            if (_swState != swInited)
                return false;
            _swState = swActive;
            return true;
        }
        //tries to switche FSM to swInited, returns true if succeded,
        //false if stopwatch is currently signaling
        bool stop(void)
        {
            if (_swState == swIsToSignal) {
                _nextState = swInited;
                return false;
            }
            _swState = swInited;
            _nextState = swIsToSignal;
            return true;
        }

        //tries to switche FSM to swIdle, returns true if succeded,
        //false if stopwatch is currently signaling
        bool release(void)
        {
            if (_swState == swIsToSignal) {
                _nextState = swIdle;
                return false;
            }
            _swState = swIdle;
            _nextState = swIsToSignal;
            return true;
        }

        TimeWatcherITF::SignalResult notify(void)
        {
            TimerListenerITF *  evtHdl = NULL;
            OPAQUE_OBJ          evtObj;
            {
                MutexGuard grd(*this);
                if (_swState != swIsToSignal)       //wrong state
                    return TimeWatcherITF::evtOk;
                if (_nextState != swIsToSignal) {   //signalling was cancelled
                    _swState = _nextState;
                    return TimeWatcherITF::evtOk;
                }
                evtHdl = _eventHdl;
                evtObj = _opaqueObj;
                _nextState = swInited;
            }
            TimeWatcherITF::SignalResult rval =
                evtHdl->onTimerEvent(TimerHdl(_id, _owner), evtObj.kind ? &evtObj : NULL);

            if (rval == TimeWatcherITF::evtOk) {
                MutexGuard grd(*this);
                //switch to _nextState (swIdle or swInited)
                _swState = _nextState;
                _nextState = swIsToSignal;
            }
            return rval;
        }
    };
    //
    class SWNotifier : public Thread {
    private:
        using Thread::Start; //hide it to avoid annoying CC warnings

    protected:
        EventMonitor    _sync;
        volatile bool   _running;
        std::string     _ident;
        TimeWatcherITF * watcher;
        TimersLIST      timers;
        Logger *        logger;

        int Execute();
    public:
        SWNotifier(const char * use_id, TimeWatcherITF * master, Logger * uselog = NULL)
            : _running(false), watcher(master), _ident("TmNtfr[")
        {
            logger = uselog ? uselog : Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
            _ident += use_id; _ident += ']';
        }
        ~SWNotifier()
        {
            Stop(true);
        }

        inline const char * logId(void) const { return _ident.c_str(); }
        void signalTimer(TimerHdl & sw_id);
        void signalTimers(TimersLIST & sw_list);

        bool isRunning(void);
        bool Start(void);
        //if do_wait is set, Notifier waits for last stopwatch being signaled
        void Stop(bool do_wait = false);
    };

    //
    class EventMonitorPSX : public EventMonitor {
    public:
        using EventMonitor::wait;
        int wait(const struct timeval & upto)
        {
            NANOTIME_STRUCT tv;
            tv.tv_sec = upto.tv_sec;
            tv.tv_nsec = upto.tv_usec * 1000;
            return pthread_cond_timedwait(&event, &mutex, &tv);
        }
    };

    typedef std::vector<StopWatch*> SWRegistry;
    typedef std::multimap<struct timeval, uint32_t/*swId*/> SWTimesQueue;
    typedef std::map<uint32_t, SWTimesQueue::iterator> SWSupervised;

    mutable EventMonitorPSX _sync;
    volatile bool   _running;
    std::string     _ident, _idStr;
    uint32_t        _lastId;
    SWNotifier *    ntfr;
    Logger *        logger;

    SWQueueITF &    swQueue;
    SWRegistry      swReg;  //timers registry
    std::list<StopWatch*>   swPool; //list of idle timers

    //adds timer to list of supervised ones
    TMError watchTimer(StopWatch * p_sw, const struct timeval & abs_time)
    {
        MutexGuard tmp(_sync);
        if (!_running)
            return TimeWatcherITF::errTimerState;
        swQueue.add(SWQueueITF::SWIdTime(abs_time, p_sw->Id()));
//        smsc_log_debug(logger, "%s: timer[%s] monitoring ON", logId(), p_sw->IdStr());
        _sync.notify();
        return TimeWatcherITF::errOk;
    }
    //removes timer from list of supervised ones
    void cancelTimer(uint32_t tm_id)
    {
        MutexGuard tmp(_sync);
        swQueue.remove(tm_id);
//        if (swQueue.remove(tm_id))
//            smsc_log_debug(logger, "%s: timer[%u] monitoring OFF", logId(), tm_id);
    }
    //moves timer to pool of idle ones
    void releaseTimer(StopWatch * p_sw)
    {
        MutexGuard tmp(_sync);
        swPool.push_back(p_sw);
        smsc_log_debug(logger, "%s: timer[%s] moved to pool", logId(), p_sw->IdStr());
    }
    //adds timer to list of supervised ones
    TMError activateTimer(StopWatch * p_sw, const struct timeval & abs_time)
    {
        MutexGuard swGrd(*p_sw);
        if (p_sw->activate())
            return watchTimer(p_sw, abs_time);
        smsc_log_warn(logger, "%s: timer[%s] activation failed", logId(), p_sw->IdStr());
        return TimeWatcherITF::errTimerState;
    }

    StopWatch * getTimer(uint32_t tm_id)
    {
        MutexGuard tmp(_sync);
        return (tm_id && (tm_id <= swReg.size())) ? swReg[tm_id-1] : NULL;
    }

protected:
    TimerHdl _createTimer(TimerListenerITF * listener, const SWTgtTime & tgt_time,
                            OPAQUE_OBJ * opaque_obj = NULL);

    // -- -------------------------
    // -- Thread interface methods
    // -- -------------------------
    int  Execute(void);

    // -- -------------------------
    // -- TimeWatcherITF methods
    // -- -------------------------
    const char * IdStr(uint32_t tmr_id);
    //
    TMError StartTimer(uint32_t tmr_id);
    //
    TMError StopTimer(uint32_t tmr_id);
    //
    TMError RefTimer(uint32_t tmr_id);
    //releases timer on last unref
    TMError UnRefTimer(uint32_t tmr_id);
    //
    SignalResult SignalTimer(uint32_t tmr_id);

public:
    typedef enum {
        tmwUnexpected = -1, //unexpected fatal exception was caught
        tmwStopped = 0,     //normal shutdown
        tmwError = 1        //
    } ShutdownReason;

    TimeWatcherAC(const char * use_id, SWQueueITF & use_queue,
                  uint32_t init_tmrs = 0, Logger * uselog = NULL);
    virtual ~TimeWatcherAC();

    inline const char * logId(void) const { return _idStr.c_str(); }
    inline void getTime(struct timeval & abs_time) { gettimeofday(&abs_time, 0); }

    void Reserve(uint32_t num_tmrs);
    bool isRunning(void);
    bool Start(void);
    //NOTE: forces all remaining timers to being reported as expired !
    void Stop(bool do_wait = false);

    virtual TimerHdl CreateTimer(TimerListenerITF * listener,
                        const SWTgtTime & tgt_time, OPAQUE_OBJ * opaque_obj = NULL) = 0;
};


//Generic timewatcher: monitors for timers with arbitrary expiration times
class TimeWatcher : public TimeWatcherAC {
protected:
    SWQueueMSet _queue;

public:
    TimeWatcher(const char * use_id, uint32_t init_tmrs, Logger * uselog = NULL)
        : TimeWatcherAC(use_id, _queue, init_tmrs, uselog)
    { }
    ~TimeWatcher()
    { }

    inline TimerHdl CreateTimer(TimerListenerITF * listener, const SWTgtTime & tgt_time,
                            OPAQUE_OBJ * opaque_obj = NULL)
    {
        return _createTimer(listener, tgt_time, opaque_obj);
    }
};

//Specific timewatcher: monitors for timers with expiration times based on same timeout
class TimeWatcherTMO : public TimeWatcherAC {
protected:
    SWQueueList _queue;
    SWTgtTime   _tmo;

public:
    TimeWatcherTMO(const char * use_id, uint32_t init_tmrs, long timeout,
                   bool unit_mlsecs = false, Logger * uselog = NULL)
        : _tmo(timeout, unit_mlsecs)
        , TimeWatcherAC(use_id, _queue, init_tmrs, uselog)
    { }
    ~TimeWatcherTMO()
    { }

    inline TimerHdl CreateTimer(TimerListenerITF * listener, const SWTgtTime & tgt_time,
                            OPAQUE_OBJ * opaque_obj = NULL)
    {
        return !(_tmo == tgt_time) ? TimerHdl() :
                _createTimer(listener, tgt_time, opaque_obj);
    }
    //
    inline TimerHdl CreateTimer(TimerListenerITF * listener, OPAQUE_OBJ * opaque_obj = NULL)
    {
        return _createTimer(listener, _tmo, opaque_obj);
    }
};


class TimeWatchersRegistryITF {
public:
    //Returns generic TimeWatcher (optionally started)
    virtual TimeWatcher *
        getTimeWatcher(uint32_t num_tmrs = 0, bool do_start = true) = 0;
    //Returns TimeWatcherTMO responsible for given timeout (optionally started)
    virtual TimeWatcherTMO *
        getTmoTimeWatcher(long timeout, bool unit_mlsecs = false,
                          uint32_t num_tmrs = 0, bool do_start = true) = 0;
};

//NOTE: this registry isn't multithread safe!
//Use external _sync if safety required.
class TimeWatchersRegistry : public TimeWatchersRegistryITF {
protected:
    typedef POBJRegistry_T<SWTgtTime, TimeWatcherTMO> TWRegistry;

    uint32_t    _lastId; //0 is reserved for generic TimeWatcher
    TWRegistry  twReg;
    std::auto_ptr<TimeWatcher> twMain; //generic TimeWatcher
    Logger *    logger;

public:
    TimeWatchersRegistry(Logger * use_log = NULL)
        : _lastId(0)
    {
        logger = use_log ? use_log : Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
    }
    virtual ~TimeWatchersRegistry()
    {
        StopAll(true);
    }

    //Returns false if at least one timewatcher failed to start
    bool StartAll(void);
    void StopAll(bool do_wait = false);

    //Returns generic TimeWatcher (optionally started)
    TimeWatcher *    getTimeWatcher(uint32_t num_tmrs = 0, bool do_start = true);
    //Returns TimeWatcherTMO responsible for given timeout (optionally started)
    TimeWatcherTMO * getTmoTimeWatcher(long timeout, bool unit_mlsecs = false,
                                       uint32_t num_tmrs = 0, bool do_start = true);
};


class TimeoutHDL {
protected:
    unsigned short  _tmo; //timeout in seconds
    TimeWatcherTMO * _tw;

public:
    TimeoutHDL(unsigned short tmo_secs = 0)
        : _tmo(tmo_secs), _tw(0)
    { }
    ~TimeoutHDL()
    { }

    inline unsigned short Value(void) const { return _tmo; }

    inline bool Init(TimeWatchersRegistryITF * tw_reg, uint32_t num_tmrs = 0, bool do_start = false)
    {
        _tw = tw_reg->getTmoTimeWatcher((long)_tmo, false, num_tmrs, do_start);
        return _tw ? true : false;
    }
    inline bool Start(void)
    {
        return _tw ? _tw->Start() : false;
    }
    inline TimerHdl CreateTimer(TimerListenerITF * listener,
                                OPAQUE_OBJ * opaque_obj = NULL) const
    {
        return _tw->CreateTimer(listener, opaque_obj);
    }

    inline TimeoutHDL & operator= (unsigned short tmo_secs)
    {
        _tmo = tmo_secs;
        return *this;
    }
};

} //timers
} //core
} //smsc
#endif /* _CORE_SYNC_TIMEWATCHER_HPP */

