/* ************************************************************************** *
 * Signalling Timers Watcher implementation.
 * ************************************************************************** */
#ifndef _CORE_SYNC_TIMEWATCHER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _CORE_SYNC_TIMEWATCHER_HPP

#include <list>
#include <set>
#include <map>
#include <vector>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/TimeSlice.hpp"

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
using smsc::util::Exception;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::TimeSlice;


typedef std::list<TimerHdl> TimersLIST;

//StopWatch target time
class SWTgtTime {
public:
    enum ValueType {
        valTime = 0     //absolute time
        , valTmo        //relative timeout
    };

protected:
    ValueType   kind;
    struct {
        TimeSlice           tmo;
        struct  timespec    tms;
    }           val;

public:
    SWTgtTime(const struct timespec & abs_time)
        : kind(valTime)
    {
        val.tms = abs_time;
    }
    SWTgtTime(const TimeSlice & use_tmo)
        : kind(valTmo)
    {
        val.tmo = use_tmo;
    }
    SWTgtTime(long use_tmo = 0, TimeSlice::UnitType_e use_unit = TimeSlice::tuSecs)
        : kind(valTmo)
    {
        val.tmo = TimeSlice(use_tmo, use_unit);
    }
    ~SWTgtTime()
    { }

    ValueType Type(void) const { return kind; }
    //Note: Returns valid value only in case of valTmo
    const TimeSlice & ValueTMO(void) const { return val.tmo; }
    //Note: Returns valid value only in case of valTime
    const struct timespec & ValueTMS(void) const { return val.tms; }
    //
    struct timespec adjust(const struct timespec * cur_time = 0) const
    {
        if (kind == valTime)
            return ValueTMS();

        return ValueTMO().adjust2Nano(cur_time);
    }

    bool operator== (const SWTgtTime & obj2) const
    {
        if (kind != obj2.kind)
            return false;
        if (kind == valTmo)
            return val.tmo == obj2.val.tmo;
        return val.tms == obj2.val.tms;
    }

    bool operator< (const SWTgtTime & obj2) const //throw(Exception)
    {
        if (kind != obj2.kind)
            throw Exception("comparing SWTgtTime of different kind");

        if (kind == valTmo)
            return val.tmo < obj2.val.tmo;
        return val.tms < obj2.val.tms;
    }
};


class SWQueueITF {
protected:
    virtual ~SWQueueITF() //forbid interface destruction
    { }

public:
    class SWIdTime {
    public:
        uint32_t        swId;
        struct timespec swTms;

        SWIdTime() : swId(0)
        { swTms.tv_sec = swTms.tv_nsec = 0; }

        SWIdTime(const struct timespec & use_tmv, uint32_t use_id)
            : swId(use_id), swTms(use_tmv)
        { }
        ~SWIdTime()
        { }

        bool operator< (const SWIdTime & obj2) const
        {
            return swTms < obj2.swTms;
        }
    };

    virtual bool empty(void) const = 0;
    virtual uint32_t size(void) const = 0;
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

    bool empty(void) const { return swQueue.empty(); }
    //
    uint32_t size(void) const { return (uint32_t)swQueue.size(); }
    //
    const SWIdTime & front(void) { return *swQueue.begin(); }
    //
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

    bool empty(void) const { return swQueue.empty(); }
    //
    uint32_t size(void) const { return (uint32_t)swQueue.size(); }
    //
    const SWIdTime & front(void) { return *swQueue.begin(); }

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
        enum SWState { //reverse order of FSM states
            swIsToSignal = 0, swActive, swInited, swIdle
        };

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

        void mkIdent(void)
        {
            char buf[2*sizeof(uint32_t)*3 + 4]; //":%u{%u}"
            snprintf(buf, sizeof(buf)-1, ":%u{%u}", _id, _usage);
            _idStr = _ownerId; _idStr += buf;
        }

    public:
        StopWatch(uint32_t tm_id, TimeWatcherITF * use_owner, std::string & owner_id)
            : _id(tm_id), _refNum(0), _owner(use_owner), _ownerId(owner_id)
            , _swState(swIdle), _nextState(swIsToSignal), _usage(0), _eventHdl(0)
        {
            mkIdent();
        }
        ~StopWatch()
        { }

        uint32_t Id(void) const { return _id; }

        //notifies event handler
        TimeWatcherITF::SignalResult notify(void);

        // -- ******************************************************************** --
        // -- NOTE: following methods require StopWatch being locked !!!
        // -- ******************************************************************** --
        //
        SWState State(void) const { return _swState; }
        //
        const char * IdStr(void) const { return _idStr.c_str(); }
        //
        struct timespec tgtTime(const struct timespec * cur_time = 0) const
        {
            return _tgtTime.adjust(cur_time);
        }

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
        //switches FSM to swIsToSignal 
        bool setToSignal(void);
        //switches FSM to swInited
        bool init(TimerListenerITF * listener, const SWTgtTime & tgt_time,
                  const OPAQUE_OBJ * opaque_obj = NULL);
        //switches FSM to swActive
        bool activate(void);
        //tries to switche FSM to swInited, returns true if succeded,
        //false if stopwatch is currently signaling
        bool stop(void);
        //tries to switche FSM to swIdle, returns true if succeded,
        //false if stopwatch is currently signaling
        bool release(void);
    };

    //
    class SWNotifier : public Thread {
    private:
        using Thread::Start; //hide it to avoid annoying CC warnings

    protected:
        mutable EventMonitor  _sync;
        volatile bool         _running;

        std::string       _ident;
        TimeWatcherITF * _watcher;
        TimersLIST        _timers;
        Logger *          logger;

        int Execute();

    public:
        SWNotifier(const char * use_id, TimeWatcherITF * master, Logger * uselog = NULL)
            : _running(false), _ident("TmNtfr["), _watcher(master)
        {
            logger = uselog ? uselog : Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
            _ident += use_id; _ident += ']';
        }
        ~SWNotifier()
        {
            Stop(true);
        }

        const char * logId(void) const { return _ident.c_str(); }
        void signalTimer(uint32_t sw_id);

        bool isRunning(void) const;
        bool Start(void);
        //if do_wait is set, Notifier waits for last stopwatch being signaled
        void Stop(bool do_wait = false);
    };

    class SWStore {
    protected:
      class SWRegistry : public std::vector<StopWatch *> {
      public:
        StopWatch * getStopWatch(uint32_t tm_id) const
        {
          return (tm_id && (tm_id <= size())) ? at(tm_id - 1) : NULL;
        }
      };
      typedef std::list<StopWatch *> SWList;
      typedef std::multimap<struct timespec, uint32_t/*swId*/> SWTimesQueue;
      typedef std::map<uint32_t, SWTimesQueue::iterator> SWSupervised;

      SWQueueITF &  swQueue;  //sorted queue of supervised timers
      SWRegistry    swReg;    //timers registry
      SWList        swPool;   //list of idle timers

    public:
      SWStore(SWQueueITF & use_queue) : swQueue(use_queue)
      { }
      ~SWStore()
      {
        //swPool.clear();
        for (SWRegistry::iterator it = swReg.begin(); it != swReg.end(); ++it)
          delete *it;
        //swReg.clear();
      }

      SWQueueITF & Queue(void) const { return swQueue; }
      //
      void reserve(uint32_t num_tmrs)
      {
        swReg.reserve(num_tmrs); //reallocate only if num_tmrs > size()
      }

      void clear(void)
      {
        swPool.clear();
        for (SWRegistry::iterator it = swReg.begin(); it != swReg.end(); ++it)
          delete *it;
        swReg.clear();
      }

      //
      void addTimer(StopWatch * use_sw)
      {
        swReg.push_back(use_sw);
      }
      //
      StopWatch * getIdleTimer(void)
      {
        StopWatch * pSw = NULL;
        if (!swPool.empty()) {
          pSw = swPool.front();
          swPool.pop_front();
        }
        return pSw;
      }

      //adds timer to list of supervised ones
      void watchTimer(StopWatch * p_sw, const struct timespec & abs_time)
      {
        swQueue.add(SWQueueITF::SWIdTime(abs_time, p_sw->Id()));
      }
      //removes timer from list of supervised ones
      bool cancelTimer(uint32_t tm_id)
      {
        return swQueue.remove(tm_id);
      }
      //moves timer to pool of idle ones
      void releaseTimer(StopWatch * p_sw)
      {
        swPool.push_back(p_sw);
      }
      //switches timer to activated state, and adds it to list of supervised ones
      //NOTE: Timer should be locked upon entry!
      bool activateTimer(StopWatch * p_sw, const struct timespec & abs_time)
      {
        bool rval = p_sw->activate();
        if (rval)
          watchTimer(p_sw, abs_time);
        return rval;
      }
      //
      StopWatch * getTimer(uint32_t tm_id) const
      {
        return swReg.getStopWatch(tm_id);
      }
      //
      StopWatch * lockTimer(uint32_t tm_id) const
      {
        StopWatch * pSw = swReg.getStopWatch(tm_id);
        if (pSw)
          pSw->Lock();
        return pSw;
      }
    };
    //
    class SWGuard {
    protected:
      const SWStore & _store;
      StopWatch *     _pSw;

    public:
      SWGuard(const SWStore & use_store, uint32_t sw_id)
        : _store(use_store)
      {
        _pSw = _store.lockTimer(sw_id);
      }
      ~SWGuard()
      {
        if (_pSw)
          _pSw->Unlock();
      }
      StopWatch * get(void) const { return _pSw; }
      //
      StopWatch * operator->() const { return get(); }
    };


    mutable EventMonitor    _sync;
    volatile bool           _running;

    std::string     _ident, _idStr;
    uint32_t        _lastId;
    SWNotifier *    _ntfr;
    //NOTE: any access to _swStore should be guarded by _sync
    SWStore         _swStore;
    Logger *        logger;

    //
    StopWatch * getTimer(uint32_t tm_id) const
    {
        MutexGuard tmp(_sync);
        return _swStore.getTimer(tm_id);
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
    const char * IdStr(uint32_t tmr_id) const;
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

    const char * logId(void) const { return _idStr.c_str(); }

    void getTime(struct timespec & abs_time) const { TimeSlice::getRealTime(abs_time); }


    void Reserve(uint32_t num_tmrs);
    bool isRunning(void) const;
    bool Start(void);
    //NOTE: forces all remaining timers to being reported as expired !
    void Stop(bool do_wait = false);
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

    TimerHdl CreateTimer(TimerListenerITF * listener, const SWTgtTime & tgt_time,
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
    TimeWatcherTMO(const char * use_id, uint32_t init_tmrs,
                   const TimeSlice & use_tmo, Logger * uselog = NULL)
        : TimeWatcherAC(use_id, _queue, init_tmrs, uselog), _tmo(use_tmo)
    { }
    ~TimeWatcherTMO()
    { }

    //
    TimerHdl CreateTimer(TimerListenerITF * listener, OPAQUE_OBJ * opaque_obj = NULL)
    {
        return _createTimer(listener, _tmo, opaque_obj);
    }
};


class TimeWatchersRegistryITF {
protected:
    virtual ~TimeWatchersRegistryITF() //forbid interface destruction
    { }

public:
    //Returns generic TimeWatcher (optionally started)
    virtual TimeWatcher *
        getTimeWatcher(uint32_t num_tmrs = 0, bool do_start = true) = 0;
    //Returns TimeWatcherTMO responsible for given timeout (optionally started)
    virtual TimeWatcherTMO *
        getTmoTimeWatcher(const TimeSlice & use_tmo,
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
    TimeWatcherTMO * getTmoTimeWatcher(const TimeSlice & use_tmo,
                                       uint32_t num_tmrs = 0, bool do_start = true);
};


class TimeoutHDL : public TimeSlice {
protected:
    TimeWatcherTMO * _tw;

public:
    TimeoutHDL(long use_tmo = 0, UnitType_e use_unit = tuSecs)
        : TimeSlice(use_tmo, use_unit), _tw(0)
    { }
    ~TimeoutHDL()
    { }

    bool empty(void) const { return _tw == NULL; }

    void assign(long use_tmo, UnitType_e use_unit = tuSecs)
    {
      unitId = use_unit; tmoVal = use_tmo;
    }

    bool Init(TimeWatchersRegistryITF * tw_reg, uint32_t num_tmrs = 0, bool do_start = false)
    {
        _tw = tw_reg->getTmoTimeWatcher(*this, num_tmrs, do_start);
        return _tw ? true : false;
    }
    bool Start(void) const
    {
        return _tw ? _tw->Start() : false;
    }
    TimerHdl CreateTimer(TimerListenerITF * listener,
                                OPAQUE_OBJ * opaque_obj = NULL) const
    {
        return _tw->CreateTimer(listener, opaque_obj);
    }

    TimeoutHDL & operator= (const TimeSlice & use_tmo)
    {
        *((TimeSlice*)this) = use_tmo;
        return *this;
    }
};

} //timers
} //core
} //smsc
#endif /* _CORE_SYNC_TIMEWATCHER_HPP */

