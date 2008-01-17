#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/common/TimeWatcher.hpp"

namespace smsc {
namespace core {
namespace timers {

#define TIMEOUT_STEP 50 //millisecs

/* ************************************************************************** *
 * class TimeWatcherAC::SWNotifier implementation:
 * ************************************************************************** */
bool TimeWatcherAC::SWNotifier::isRunning(void)
{
    MutexGuard grd(_sync);
    return _running;
}

bool TimeWatcherAC::SWNotifier::Start(void)
{
    if (isRunning())
        return true;
    smsc_log_debug(logger, "%s: starting ..", logId());
    Thread::Start();
    {
        MutexGuard grd(_sync);
        if (_running)
            return true;
        _sync.wait(TIMEOUT_STEP);
        if (!_running) {
            smsc_log_fatal(logger, "%s: unable to start", logId());
            return false;
        }
    }
    return true;
}

void TimeWatcherAC::SWNotifier::Stop(void)
{
    {
        MutexGuard grd(_sync);
        if (!_running)
            return;
        _running = false;
        unsigned cnt = (unsigned)timers.size();
        smsc_log_debug(logger, "%s: stopping, %u timers pending ..", logId(), cnt);
        _sync.notify();
    }
    WaitFor();
    smsc_log_debug(logger, "%s: stopped", logId());
}

int TimeWatcherAC::SWNotifier::Execute(void)
{
//    smsc_log_debug(logger, "%s: started.", logId());
    _sync.Lock();
    _running = true;
    _sync.notify();
    while (_running || !timers.empty()) {
        if (!timers.empty()) {
            do {
                TimerHdl & tmr = *timers.begin();
                _sync.Unlock();
                TimeWatcherITF::SignalResult rval = TimeWatcherITF::evtOk;
                try {
                    smsc_log_debug(logger, "%s: signaling timer[%s]", logId(), tmr.IdStr());
                    rval = watcher->SignalTimer(tmr.Id());
                } catch (const std::exception & exc) {
                    smsc_log_error(logger, "%s: timer[%s] listener exception: %s", logId(), tmr.IdStr(), exc.what());
                } catch (...) {
                    smsc_log_error(logger, "%s: timer[%s] listener unknown exception", logId(), tmr.IdStr());
                }
                _sync.Lock();
                if (rval == TimeWatcherITF::evtOk)
                    timers.pop_front(); //Unrefs timer
            } while (!timers.empty());
        }
        _sync.wait(TIMEOUT_STEP); //unlocks and waits
    }
    _sync.Unlock();
    return 0;
}

void TimeWatcherAC::SWNotifier::signalTimer(TimerHdl & sw_id)
{
    MutexGuard grd(_sync);
    timers.push_back(sw_id);
    _sync.notify();
}

void TimeWatcherAC::SWNotifier::signalTimers(TimersLIST & tmm)
{
    MutexGuard grd(_sync);
    timers.insert(timers.end(), tmm.begin(), tmm.end());
    _sync.notify();
}

/* ************************************************************************** *
 * class TimeWatcherAC implementation:
 * ************************************************************************** */
TimeWatcherAC::TimeWatcherAC(const char * use_id, SWQueueITF & use_queue,
                             uint32_t init_tmrs/* = 0*/, Logger * uselog/* = NULL*/)
    : _running(false), _lastId(0), swQueue(use_queue)
    , _ident(use_id), _idStr("TmWT[")
{
    _idStr += use_id; _idStr += ']';
    logger = uselog ? uselog : Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
    ntfr = new TimeWatcherAC::SWNotifier(use_id, this, logger);
    swReg.reserve(init_tmrs ? init_tmrs : 100);
}

TimeWatcherAC::~TimeWatcherAC()
{
    Stop();
    WaitFor();
    MutexGuard tmpGrd(_sync);
    delete ntfr;
    for (SWRegistry::iterator it = swReg.begin(); it != swReg.end(); ++it)
        delete *it;
}

void TimeWatcherAC::Reserve(uint32_t num_tmrs)
{
    MutexGuard grd(_sync);
    swReg.reserve(num_tmrs); //reallocate only if num_tmrs > size()
}

bool TimeWatcherAC::isRunning(void)
{
    MutexGuard grd(_sync);
    return _running;
}

bool TimeWatcherAC::Start(void)
{
    if (isRunning())
        return true;
    smsc_log_debug(logger, "%s: starting ..", logId());
    Thread::Start();
    {
        MutexGuard grd(_sync);
        if (_running)
            return true;
        _sync.wait(TIMEOUT_STEP);
        if (!_running) {
            smsc_log_fatal(logger, "%s: unable to start", logId());
            return false;
        }
    }
    return true;
}

void TimeWatcherAC::Stop(void)
{
    {
        MutexGuard grd(_sync);
        if (!_running)
            return;
        smsc_log_debug(logger, "%s: stopping.", logId());
        _running = false;
        _sync.notify();
    }
    ntfr->WaitFor();
    smsc_log_debug(logger, "%s: stopped.", logId());
}

TimerHdl TimeWatcherAC::_createTimer(TimerListenerITF * listener,
                                  const SWTgtTime & tgt_time,
                                  OPAQUE_OBJ * opaque_obj/* = NULL*/)
{
    TimeWatcherAC::StopWatch * pSw = NULL;
    {
        MutexGuard tmp(_sync);
        if (!swPool.empty()) {
            pSw = swPool.front();
            swPool.pop_front();
        } else {
            uint32_t    swId = ++_lastId;
            if (swId)
                swReg.push_back(pSw = new TimeWatcherAC::StopWatch(swId, this, _ident));
            else {
                --_lastId;
                smsc_log_fatal(logger, "%s: too much timers allocated!", logId());
            }
        }
        if (pSw && !pSw->init(listener, tgt_time, opaque_obj)) {
            smsc_log_fatal(logger, "%s: internal inconsistency"
                                   " (timer[%s] init failed)!", logId(), pSw->IdStr());
            pSw = NULL;
        }
    }
    return pSw ? TimerHdl(pSw->Id(), this) : TimerHdl();
}

// -- -------------------------
// -- Thread interface methods
// -- -------------------------
int TimeWatcherAC::Execute(void)
{
    if (!ntfr->Start())
        return tmwUnexpected;

//    smsc_log_debug(logger, "%s: started.", logId());
    _sync.Lock();
    _running = true;
    _sync.notify();
    while (_running) {
        struct timeval tms, tmNext;
        getTime(tms);
        if (!swQueue.empty()) {
            //determine expired timers, pass them to Notifier,
            //determine next time to awake
            do {
                const SWQueueITF::SWIdTime & swt = swQueue.front();
                if (tms < swt.swTms) {
                    tmNext = swt.swTms;
                    break;
                }
                //signal expired timer
                TimeWatcherAC::StopWatch * pSw = swReg[swt.swId - 1];
                swQueue.pop_front();
                _sync.Unlock();

                smsc_log_debug(logger, "%s: timer[%s] is expired", logId(), pSw->IdStr());
                if (pSw->setToSignal())
                    ntfr->signalTimer(TimerHdl(pSw->Id(), this));
                _sync.Lock();
            } while (!swQueue.empty());
        }
        if (swQueue.empty()) {
            tmNext = tms;
            tmNext.tv_usec += 1000L*TIMEOUT_STEP;
            if (tmNext.tv_usec > 1000000L) {
                tmNext.tv_sec++;
                tmNext.tv_usec = tmNext.tv_usec % 1000000L;
            }
        }
        _sync.wait(tmNext);
    } /* eow */

    TimersLIST  swList;
    if (!swQueue.empty()) {
        smsc_log_warn(logger, "%s: aborting remaining timers ..", logId());
        do {
            const SWQueueITF::SWIdTime & swt = swQueue.front();
            swList.push_back(TimerHdl(swt.swId, this));
            swQueue.pop_front();
        } while (!swQueue.empty());
    }
    _sync.Unlock();

    if (!swList.empty())
        ntfr->signalTimers(swList);
    ntfr->Stop();
    return tmwStopped;
}

// -- -------------------------------- --
// -- TimeWatcherITF interface methods
// -- -------------------------------- --
const char * TimeWatcherAC::IdStr(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    return pSw ? pSw->IdStr() : "unknown";
}

TMError TimeWatcherAC::StartTimer(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    if (!pSw)
        return TimeWatcherITF::errBadTimer;

    struct timeval cur_time;
    getTime(cur_time);
    struct timeval tgt_time = pSw->tgtTime(cur_time);
    if (cur_time >= tgt_time)
        return TimeWatcherITF::errBadTimeVal;
    return activateTimer(pSw, tgt_time);
}
//
TMError TimeWatcherAC::StopTimer(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    if (pSw) {
        MutexGuard grd(*pSw);
        if (pSw->State() == TimeWatcherAC::StopWatch::swActive)
            cancelTimer(tmr_id);
        pSw->stop();
        return TimeWatcherITF::errOk;
    }
    return TimeWatcherITF::errBadTimer;
}

TMError TimeWatcherAC::RefTimer(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    if (pSw) {
        MutexGuard grd(*pSw);
//        smsc_log_debug(logger, "%s: timer[%s] Ref()", logId(), pSw->IdStr());
        return pSw->Ref() ? TimeWatcherITF::errOk
                    : TimeWatcherITF::errTimerState;
    }
    return TimeWatcherITF::errBadTimer;
}

TMError TimeWatcherAC::UnRefTimer(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    if (pSw) {
        MutexGuard grd(*pSw);
//        smsc_log_debug(logger, "%s: timer[%s] UnRef()", logId(), pSw->IdStr());
        if (pSw->UnRef() && pSw->release())
            releaseTimer(pSw);
        return TimeWatcherITF::errOk;
    }
    return TimeWatcherITF::errBadTimer;
}

//
TimeWatcherITF::SignalResult TimeWatcherAC::SignalTimer(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    return pSw ? pSw->notify() : TimeWatcherITF::evtOk;
}

/* ************************************************************************** *
 * class TimeWatchersRegistry implementation:
 * ************************************************************************** */
TimeWatchersRegistry::~TimeWatchersRegistry()
{
    if (twMain.get())
        twMain->Stop();
    twMain.reset();

    for (TWRegistry::iterator it = twReg.begin(); it != twReg.end(); ++it) {
        it->second->Stop();
        delete it->second;
    }
}

void TimeWatchersRegistry::StopAll(void)
{
    if (twMain.get()) {
//        smsc_log_debug(logger, "TWReg: stoppping %s", twMain->logId());
        twMain->Stop();
    }

    for (TWRegistry::iterator it = twReg.begin(); it != twReg.end(); ++it) {
//        smsc_log_debug(logger, "TWReg: stoppping %s", it->second->logId());
        it->second->Stop();
    }
}

//Returns generic TimeWatcher
TimeWatcher * TimeWatchersRegistry::getTimeWatcher(uint32_t num_tmrs/* = 0*/)
{
    MutexGuard  grd(_sync);
    if (!twMain.get()) {
        twMain.reset(new TimeWatcher("0", num_tmrs, logger));
        smsc_log_info(logger, "TWReg: starting generic %s", twMain->logId());
        twMain->Start();
    } else if (num_tmrs)
        twMain->Reserve(num_tmrs);
    return twMain.get();
}

//Returns TimeWatcherUNI adjusted for given timeout
TimeWatcherTMO * TimeWatchersRegistry::getTmoTimeWatcher(long timeout,
                            bool unit_mlsecs/* = false*/, uint32_t num_tmrs/* = 0*/)
{
    TimeWatcherTMO * pTw = NULL;
    if (timeout) {
        MutexGuard  grd(_sync);
        SWTgtTime   tmo(timeout, unit_mlsecs);

        TWRegistry::iterator it = twReg.find(tmo);
        if (it == twReg.end()) {
            char buf[sizeof(uint32_t)*3 + 2];
            snprintf(buf, sizeof(buf)-1, "%u", ++_lastId);
            pTw = new TimeWatcherTMO(buf, num_tmrs, timeout, unit_mlsecs, logger);
            smsc_log_info(logger, "TWReg: starting %s, tmo: %ld %ssecs", pTw->logId(),
                          timeout, unit_mlsecs ? "ml" : "");
            pTw->Start();
            twReg.insert(TWRegistry::value_type(tmo, pTw));
        } else {
            pTw = it->second;
            pTw->Reserve(num_tmrs);
        }
    }
    return pTw;
}

} //timers
} //core
} //smsc

