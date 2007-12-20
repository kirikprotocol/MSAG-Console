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

StopWatch::TMError StopWatch::start(const struct timeval & abs_time)
{
    MutexGuard  tmp(_sync);
    StopWatch::TMError  rval = errNoErr;

    if (_tmrState > tmrInited)
        return errActiveTimer;

    _tms = abs_time;
    //normalize timeval first
    if (_tms.tv_usec > 1000000L) {
        _tms.tv_sec += _tms.tv_usec / 1000000L;
        _tms.tv_usec = _tms.tv_usec % 1000000L;
    }
    if (!(rval = _owner->activateTimer(this)))
        _tmrState = tmrActive;
    return rval;
}

//timeout is either in millisecs or in microsecs
StopWatch::TMError StopWatch::start(long timeout, bool millisecs/* = true*/)
{
    struct timeval tms;
    _owner->getTime(tms);
    if (millisecs) {
        tms.tv_sec += timeout/1000L;
        tms.tv_usec += (timeout % 1000L)*1000L;
    } else
        tms.tv_sec += timeout;
    return start(tms);
}

void StopWatch::stop(void)
{
    _sync.Lock();
    _result = tmrStopped;           //change status so signal() doesn't call listener
    if (_tmrState == tmrIsToSignal) { //timer is passed to Notifier and awaits signalling
        _sync.Unlock();
        _sigEvent.Wait();       //wait for signal() completion
        _sync.Lock();
    }
    if (_multiRun && (_tmrState != tmrInited))
        deactivate(false);
    else if (_tmrState != tmrIdle)
        deactivate(true);
    _sync.Unlock();
}

void StopWatch::release(void)
{
    _sync.Lock();
    _result = tmrStopped;           //change status so signal() doesn't call listener
    if (_tmrState == tmrIsToSignal) { //timer is passed to Notifier and awaits signalling
        _sync.Unlock();
        _sigEvent.Wait();       //wait for signal() completion
        _sync.Lock();
    }
    if (_tmrState != tmrIdle)
        deactivate(true);
    _sync.Unlock();
}

/* ---------------------------------------------------------------------------------- *
 * StopWatch Protected methods:
 * ---------------------------------------------------------------------------------- */
void StopWatch::init(TimerListenerITF * listener, OPAQUE_OBJ * opaque_obj/* = NULL*/,
                     bool multi_run/* = false*/)
{
    MutexGuard  tmp(_sync);
    _multiRun = multi_run;
    _cb_event = listener;
    _tmrState = tmrInited;
    _result = tmrNone;
    if (opaque_obj)
        _opaqueObj = *opaque_obj;
    else
        _opaqueObj.kind = OPAQUE_OBJ::objNone;

}

void StopWatch::signalStatus(SWStatus status)
{
    MutexGuard  tmp(_sync);
    if (status > _result)
        _result = status;
    _tmrState = tmrIsToSignal;
}

int StopWatch::signal(void)
{
//    smsc_log_debug(logger, "Timer[%u]: signaling at state %u, argKind: %d", _id, _tmrState, _opaqueObj.kind);
    MutexGuard  tmp(_sync);
    if (_tmrState == tmrIsToSignal) {
        if ((_result != tmrStopped) &&_cb_event) {
            int rval = _cb_event->onTimerEvent(this,
                        (_opaqueObj.kind != OPAQUE_OBJ::objNone) ? &_opaqueObj : NULL);
            if (rval)
                return rval;
        }
        _sigEvent.Signal();
    }
    _tmrState = tmrSignaled;
    deactivate(false); //stop timer
    return 0;
}

/* ---------------------------------------------------------------------------------- *
 * StopWatch Private methods:
 * ---------------------------------------------------------------------------------- */
void StopWatch::deactivate(bool doRelease)
{
    if (_multiRun && !doRelease) {
        _owner->deactivateTimer(this);
        _tmrState = tmrInited;
    } else {
        _owner->dischargeTimer(this);
        reset();
    }
}

void StopWatch::reset(void)
{
    _multiRun = false;
    _cb_event = NULL;
    _opaqueObj.kind = OPAQUE_OBJ::objNone;
    _tmrState = tmrIdle;
    _result = tmrNone;
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
    while (_running || !timers.empty()) {
        if (!timers.empty()) {
            do {
                StopWatch * tmr = *(timers.begin());
                timers.pop_front();
                _sync.Unlock();
                int rval = 0;
                try {
                    smsc_log_debug(logger, "TmNtfr: signaling timer[%u]", tmr->getId());
                    rval = tmr->signal(); //also stops timer
                } catch (std::exception& exc) {
                    smsc_log_error(logger, "TmNtfr: timer[%u] listener exception: %s", tmr->getId(), exc.what());
                } catch (...) {
                    smsc_log_error(logger, "TmNtfr: timer[%u] listener unknown exception", tmr->getId());
                }
                _sync.Lock();
                if (rval) //listener requested resignalling
                    timers.push_back(tmr);
            } while (!timers.empty());
        }
        _sync.wait(TIMEOUT_STEP); //unlocks and waits
    }
    _sync.Unlock();
    stopped.Signal();
    return 0;
}

void TimeNotifier::signalTimer(StopWatch* tmr)
{
    MutexGuard grd(_sync);
    tmr->signalStatus(StopWatch::tmrExpired);
    timers.push_back(tmr);
    _sync.notify();
}

void TimeNotifier::signalTimers(TimersLIST& tmm, StopWatch::SWStatus status)
{
    MutexGuard grd(_sync);
    for (TimersLIST::iterator it = tmm.begin(); it != tmm.end(); it++) {
        (*it)->signalStatus(status);
    }
    timers.insert(timers.end(), tmm.begin(), tmm.end());
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
    MutexGuard tmpGrd(_sync);
    delete ntfr;

    TimersLIST::iterator it;
    for (it = pool.begin(); it != pool.end(); it++)
        delete *it;
    pool.clear();
    for (it = assigned.begin(); it != assigned.end(); it++)
        delete *it;
    assigned.clear();
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
        //determine next time to awake
        struct timeval tms, tmNext;
        getTime(tms);
//        smsc_log_debug(logger, "TmWT: time %lu:%lu.", tms.tv_sec, tms.tv_usec);
        tmNext = tms;
        tmNext.tv_usec += 1000L*TIMEOUT_STEP;
        if (tmNext.tv_usec > 1000000L) {
            tmNext.tv_sec++;
            tmNext.tv_usec = tmNext.tv_usec % 1000000L;
        }

        TimersLIST  expired;
        _sync.Lock();
        for (TimersLIST::iterator it = started.begin(); it != started.end(); ) {
            StopWatch * tmr = *it;
            TimersLIST::iterator curr = it;
            it++;
            if (tms >= tmr->targetTime()) {
                smsc_log_debug(logger, "TmWT: timer[%u] is expired.", tmr->getId());
                expired.push_back(tmr);
                started.erase(curr);
            } else if (tmr->targetTime() < tmNext)
                tmNext = tmr->targetTime();
        }
        signaled.insert(signaled.end(), expired.begin(), expired.end());
        _sync.Unlock();
        ntfr->signalTimers(expired, StopWatch::tmrExpired);
        _sync.Lock();
        //awakes either by timeout or by event signaled by StartTimer()
//        smsc_log_debug(logger, "TmWT: sleeping up to %lu:%lu.", tmNext.tv_sec, tmNext.tv_usec);
        _sync.wait(tmNext);
    } /* eow */
    if (started.size()) {
        smsc_log_debug(logger, "TmWT: aborting %u timers.", started.size());
        signaled.insert(signaled.end(), started.begin(), started.end());
        started.clear();
        ntfr->signalTimers(started, StopWatch::tmrAborted);
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
                                    OPAQUE_OBJ * opaque_obj/* = NULL*/,
                                    bool multi_run/* = false*/)
{
    MutexGuard tmpGrd(_sync);
    StopWatch * tmr = NULL;
    
    if (!pool.empty()) {
        tmr = pool.front();
        pool.pop_front();
    } else {
        if (!(++_lastId))
            return tmr;
        tmr = new StopWatch(this, _lastId);
    }
    tmr->init(listener, opaque_obj, multi_run);
    assigned.push_back(tmr);
    return tmr;
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * ---------------------------------------------------------------------------------- */
StopWatch::TMError TimeWatcher::activateTimer(TimerID tmr)
{
    MutexGuard tmpGrd(_sync);
    TimersLIST::iterator it = std::find(assigned.begin(), assigned.end(), tmr);
    if (it == assigned.end()) {
        it = std::find(started.begin(), started.end(), tmr);
        return (it != started.end()) ? 
            StopWatch::errActiveTimer : StopWatch::errBadTimer;
    }
    assigned.erase(it);
    started.push_back((StopWatch*)tmr);
    smsc_log_debug(logger, "TmWT: timer[%u] is activated.", tmr->getId());
    _sync.notify();
    return StopWatch::errNoErr;
}

void TimeWatcher::deactivateTimer(TimerID tmr)
{
    MutexGuard tmpGrd(_sync);
    smsc_log_debug(logger, "TmWT: stopping timer[%u]", tmr->getId());
    TimersLIST::iterator it = std::find(signaled.begin(), signaled.end(), tmr);
    if (it != signaled.end()) {
        signaled.erase(it);
        assigned.push_back((StopWatch*)tmr);
        smsc_log_debug(logger, "TmWT: timer[%u] moved to assigned (was signaled)", tmr->getId());
        return;
    }
    it = std::find(started.begin(), started.end(), tmr);
    if (it != started.end()) {
        started.erase(it);
        assigned.push_back((StopWatch*)tmr);
        smsc_log_debug(logger, "TmWT: timer[%u] moved to assigned (was started)", tmr->getId());
        return;
    }
    smsc_log_error(logger, "TmWT: timer[%u] was neither started nor signaled!", tmr->getId());
    return;
}

void TimeWatcher::dischargeTimer(TimerID tmr)
{
    MutexGuard tmpGrd(_sync);
    TimersLIST::iterator it;
    if ((it = std::find(signaled.begin(), signaled.end(), tmr)) != signaled.end()) {
        signaled.erase(it);
        pool.push_back((StopWatch*)tmr);
        smsc_log_debug(logger, "TmWT: timer[%u] is released to pool (was signaled)", tmr->getId());
        return;
    }
    if ((it = std::find(started.begin(), started.end(), tmr)) != started.end()) {
        started.erase(it);
        pool.push_back((StopWatch*)tmr);
        smsc_log_debug(logger, "TmWT: timer[%u] is released to pool (was started)", tmr->getId());
        return;
    }
    if ((it = std::find(assigned.begin(), assigned.end(), tmr)) != assigned.end()) {
        assigned.erase(it);
        pool.push_back((StopWatch*)tmr);
        smsc_log_debug(logger, "TmWT: timer[%u] is released to pool (was assigned)", tmr->getId());
        return;
    }
    smsc_log_warn(logger, "TmWT: releasing unassigned timer[%u]!", tmr->getId());
    return;
}

} //sync
} //inman
} //smsc

