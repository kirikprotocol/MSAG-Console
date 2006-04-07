static char const ident[] = "$Id$";
#include <assert.h>
#include <algorithm>
#include "inman/common/TimeWatcher.hpp"

namespace smsc {
namespace inman {
namespace sync {

/* ************************************************************************** *
 * class StopWatch implementation:
 * ************************************************************************** */
StopWatch::StopWatch(TimeWatcher * owner, unsigned tm_id)
    : _owner(owner), _id(tm_id)
{
    reset();
    //logger = Logger::getInstance("smsc.inman.sync");
}

void StopWatch::init(TimerListenerITF * listener, OPAQUE_OBJ * opaque_obj/* = NULL*/)
{
    _cb_event = listener;
    _tmrState = tmrInited;
    if (opaque_obj)
        _opaqueObj = *opaque_obj;
}

void StopWatch::reset(void)
{
    _cb_event = NULL;
    _opaqueObj.kind = OPAQUE_OBJ::objNone;
    _tmrState = tmrIdle;
}

void StopWatch::activate(const struct timeval & tm_val)
{
    MutexGuard  tmp(_sync);
    tms = tm_val;
    _tmrState = tmrActive;
}

void StopWatch::setStatus(SWStatus status)
{
    MutexGuard  tmp(_sync);
    if ((status != tmrStopped) || (status > _tmrState))
        _tmrState = status; 
}

void StopWatch::signal(void)
{
    MutexGuard  tmp(_sync);
//    smsc_log_debug(logger, "Timer[%u]: signaling state %u, argKind: %d", _id, _tmrState, _opaqueObj.kind);
    if ((_tmrState != tmrStopped) && _cb_event) {
        _sync.Unlock();
        _cb_event->onTimerEvent(this,
            (_opaqueObj.kind != OPAQUE_OBJ::objNone) ? &_opaqueObj : NULL);
    }
}

void StopWatch::signal(SWStatus status)
{
    MutexGuard  tmp(_sync);
    if ((status != tmrStopped) || (status > _tmrState))
        _tmrState = status; 
    if ((_tmrState != tmrStopped) && _cb_event) {
        _sync.Unlock();
        _cb_event->onTimerEvent(this,
            (_opaqueObj.kind != OPAQUE_OBJ::objNone) ? &_opaqueObj : NULL);
    }
}

void StopWatch::release(void)
{
    _owner->releaseTimer(this);
}

//timeout is either in millisecs or in microsecs
StopWatch::TMError    StopWatch::start(long timeout, bool millisecs/* = true*/)
{
    return _owner->startTimer(this, timeout, millisecs);
}

StopWatch::TMError    StopWatch::start(struct timeval & abs_time)
{
    return _owner->startTimer(this, abs_time);
}

void       StopWatch::stop(void)
{
    _owner->stopTimer(this);
}
/* ************************************************************************** *
 * class TimeNotifier implementation:
 * ************************************************************************** */
#define TIMEOUT_STEP 50 //millisecs

TimeNotifier::TimeNotifier(TimeWatcher * master, Logger * uselog/* = NULL*/)
    : _running(false), watcher(master), logger(uselog)
{
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.sync");
}

bool TimeNotifier::isRunning(void)
{
    if (!_running)
        awaked.Wait(TIMEOUT_STEP);
    return _running;
}

void TimeNotifier::Stop(void)
{
    _sync.Lock();
    _running = false;
    unsigned cnt = timers.size();
    _sync.Unlock();
    smsc_log_debug(logger, "TmNtfr: stopping, %u timers pending ..", cnt);
    stopped.Wait();
    smsc_log_debug(logger, "TmNtfr: stopped");
}

int TimeNotifier::Execute(void)
{
    smsc_log_debug(logger, "TmNtfr: started.");
    _sync.Lock();
    _running = true;
    awaked.Signal();
    while (_running || timers.size()) {
        _sync.Unlock();
        if (timers.size()) {
            do {
                StopWatch * tmr = *(timers.begin());
                try {
                    smsc_log_debug(logger, "TmNtfr: signaling timer[%u]", tmr->getId());
                    tmr->signal();
                } catch (std::exception& exc) {
                    smsc_log_error(logger, "TmNtfr: timer[%u] listener exception: %s", tmr->getId(), exc.what());
                } catch (...) {
                    smsc_log_error(logger, "TmNtfr: timer[%u] listener unknown exception", tmr->getId());
                }
                watcher->releaseTimer(tmr);
                _sync.Lock();
                timers.pop_front();
                _sync.Unlock();
            } while (timers.size());
        }
        _sync.Lock();
        _sync.wait(TIMEOUT_STEP); //unlocks and waits
    }
    _sync.Unlock();
    stopped.Signal();
    return 0;
}

void TimeNotifier::signalTimer(StopWatch* tmr)
{
    _sync.Lock();
    timers.push_back(tmr);
    _sync.Unlock();
    _sync.notify();
}

void TimeNotifier::signalTimers(TimersLIST& tmm, StopWatch::SWStatus status)
{
    _sync.Lock();
    for (TimersLIST::iterator it = tmm.begin(); it != tmm.end(); it++) {
        (*it)->setStatus(status);
    }
    timers.insert(timers.end(), tmm.begin(), tmm.end());
    _sync.Unlock();
    _sync.notify();
}

/* ************************************************************************** *
 * class TimeWatcher implementation:
 * ************************************************************************** */
TimeWatcher::TimeWatcher(Logger * uselog/* = NULL*/)
    : _running(false), _lastId(0)
{
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.sync");
    ntfr = new TimeNotifier(this, logger);
    assert(ntfr);
}

TimeWatcher::~TimeWatcher()
{
    if (_running)
        Stop();
    WaitFor();
    delete ntfr;

    TimersLIST::iterator it = pool.begin();
    for (; it != pool.end(); it++)
        delete *it;
}

bool TimeWatcher::isRunning(void)
{
    if (!_running)
        awake.Wait(TIMEOUT_STEP);
    return _running;
}

void TimeWatcher::Stop(void)
{
    smsc_log_debug(logger, "TmWT: stopping.");
    _sync.Lock();
    _running = false;
    _sync.Unlock();
    ntfr->WaitFor();
    smsc_log_debug(logger, "TmWT: stopped.");
}

int TimeWatcher::Execute(void)
{
    ntfr->Start();
    if (!ntfr->isRunning())
        return tmwUnexpected;

    smsc_log_debug(logger, "TmWT: started.");
    _sync.Lock();
    _running = true;
    awake.Signal();
    while (_running) {
        _sync.Unlock();
        //determine expired timers, pass them to Notifier,
        //count next time to awake
        struct timeval tms, tmNext;
        getTime(tms);
//        smsc_log_debug(logger, "TmWT: time %lu:%lu.", tms.tv_sec, tms.tv_usec);
        tmNext = tms;
        tmNext.tv_usec += 1000L*TIMEOUT_STEP;

        _sync.Lock();
        for (TimersLIST::iterator it = started.begin(); it != started.end(); ) {
            StopWatch * tmr = *it;
            TimersLIST::iterator curr = it;
            it++;
            if (tms >= tmr->targetTime()) {
                smsc_log_debug(logger, "TmWT: timer[%u] is expired.", tmr->getId());
                tmr->setStatus(StopWatch::tmrExpired);
                signaled.push_back(tmr);
                started.erase(curr);
                ntfr->signalTimer(tmr);
            } else if (tmr->targetTime() < tmNext)
                tmNext = tmr->targetTime();
        }
//        smsc_log_debug(logger, "TmWT: sleeping up to %lu:%lu.", tmNext.tv_sec, tmNext.tv_usec);
        //unlocks mutex, awakes either by timeout or by event signaled by StartTimer()
        _sync.wait(tmNext);
    } /* eow */
    if (started.size()) {
        smsc_log_debug(logger, "TmWT: aborting %u timers.", started.size());
        ntfr->signalTimers(started, StopWatch::tmrAborted);
        started.clear();
    }
    _sync.Unlock();
    ntfr->Stop();
    return tmwStopped;
}

void TimeWatcher::getTime(struct timeval & abs_time)
{
    gettimeofday(&abs_time, 0);
}

TimerID TimeWatcher::createTimer(TimerListenerITF * listener/*= NULL*/,
                                        OPAQUE_OBJ * opaque_obj/* = NULL*/)
{
    MutexGuard tmpGrd(_sync);
    StopWatch * tmr = NULL;
    
    if (pool.size()) {
        tmr = *(pool.begin());
        pool.pop_front();
    } else {
        tmr = new StopWatch(this, ++_lastId);
    }
    tmr->init(listener, opaque_obj);
    assigned.push_back(tmr);
    return tmr;
}

void TimeWatcher::stopTimer(TimerID tmr)
{
    //firstly set Stopped status, because of timer may be already passed to Notifier
    ((StopWatch*)tmr)->setStatus(StopWatch::tmrStopped);
    smsc_log_debug(logger, "TmWT: stopping timer[%u]", tmr->getId());
    MutexGuard tmpGrd(_sync);
    TimersLIST::iterator it = std::find(signaled.begin(), signaled.end(), tmr);
    if (it != signaled.end()) {
        signaled.erase(it);
        pool.push_back((StopWatch*)tmr);
        smsc_log_debug(logger, "TmWT: timer[%u] moved to pool (signaled)", tmr->getId());
    } else {
        it = std::find(started.begin(), started.end(), tmr);
        if (it != started.end()) {
            started.erase(it);
            pool.push_back((StopWatch*)tmr);
            smsc_log_debug(logger, "TmWT: timer[%u] moved to pool (started)", tmr->getId());
        } else
            smsc_log_warn(logger, "TmWT: timer[%u] neither signaled nor started", tmr->getId());
    }
    return;
}

void TimeWatcher::releaseTimer(TimerID tmr)
{
    stopTimer(tmr);
    ((StopWatch*)tmr)->reset();
    return;
}

StopWatch::TMError TimeWatcher::startTimer(TimerID tmr, struct timeval & abs_time)
{
    //normalize timeval first
    if (abs_time.tv_usec > 1000000L) {
        abs_time.tv_sec += abs_time.tv_usec / 1000000L;
        abs_time.tv_usec = abs_time.tv_usec % 1000000L;
    }

    MutexGuard tmpGrd(_sync);
    TimersLIST::iterator it = std::find(assigned.begin(), assigned.end(), tmr);
    if (it == assigned.end()) {
        it = std::find(started.begin(), started.end(), tmr);
        return (it != started.end()) ? 
            StopWatch::errActiveTimer : StopWatch::errBadTimer;
    }
    ((StopWatch*)tmr)->activate(abs_time);
    assigned.erase(it);
    started.push_back((StopWatch*)tmr);
    smsc_log_debug(logger, "TmWT: timer[%u] is activated.", tmr->getId());
    _sync.notify();
    return StopWatch::noErr;
}

StopWatch::TMError TimeWatcher::startTimer(TimerID tmr, long timeout, bool millisecs/* = true*/)
{
    struct timeval tms;
    getTime(tms);
    if (millisecs) {
        tms.tv_sec += timeout/1000L;
        tms.tv_usec += (timeout % 1000L)*1000L;
    } else
        tms.tv_sec += timeout;
    return startTimer(tmr, tms); //normalizes 'tms' microsecs
}

} //sync
} //inman
} //smsc

