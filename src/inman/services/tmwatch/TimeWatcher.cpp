#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/tmwatch/TimeWatcher.hpp"

namespace smsc {
namespace core {
namespace timers {

//#define TIMEOUT_STEP 50 //millisecs
static TimeSlice    TIMEOUT_STEP(50, TimeSlice::tuMSecs);

/* ************************************************************************** *
 * class TimeWatcherAC::StopWatch implementation:
 * ************************************************************************** */
// -- ******************************************************************** --
// -- NOTE: all next FSM switching methods require StopWatch being locked !!!
// -- ******************************************************************** --
//switches FSM to swIsToSignal
bool TimeWatcherAC::StopWatch::setToSignal(void)
{
    if (_swState != swActive)
        return false;
    _swState = swIsToSignal;
    return true;
}
//switches FSM to swInited, is called only either while timer creation or
//while reusing already released timer
bool TimeWatcherAC::StopWatch::init(TimerListenerITF * listener, const SWTgtTime & tgt_time,
          const OPAQUE_OBJ * opaque_obj/* = NULL*/)
{
    if (_swState != swIdle)
        return false;
    _eventHdl = listener;
    if (opaque_obj)
        _opaqueObj = *opaque_obj;
    else
        _opaqueObj.clear();

    _swState = swInited;
//    _nextState = swIsToSignal;
    ++_usage;
    _tgtTime = tgt_time;
    mkIdent();
    return true;
}
//switches FSM to swActive
bool TimeWatcherAC::StopWatch::activate(void)
{
    if (_swState != swInited)
        return false;
    _swState = swActive;
    return true;
}
//tries to switche FSM to swInited, returns true if succeded,
//false if stopwatch is currently signaling
bool TimeWatcherAC::StopWatch::stop(void)
{
    if (_swState == swIsToSignal) { //NOTE: refCount cann't be zero at this stage
        _nextState = swInited;
        return false;
    }
    _swState = swInited;
    _nextState = swIsToSignal;
    return true;
}

//tries to switche FSM to swIdle, returns true if succeded,
//false if stopwatch is currently signaling
bool TimeWatcherAC::StopWatch::release(void)
{
    if (_swState == swIsToSignal) {
        _nextState = swIdle;
        return false;
    }
    _swState = swIdle;
    _nextState = swIsToSignal;
    return true;
}

TimeWatcherITF::SignalResult TimeWatcherAC::StopWatch::notify(void)
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


/* ************************************************************************** *
 * class TimeWatcherAC::SWNotifier implementation:
 * ************************************************************************** */
bool TimeWatcherAC::SWNotifier::isRunning(void) const
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

void TimeWatcherAC::SWNotifier::Stop(bool do_wait/* = false*/)
{
    {
        MutexGuard grd(_sync);
        if (_running) {
            _running = false;
            unsigned cnt = (unsigned)_timers.size();
            smsc_log_debug(logger, "%s: stopping, %u timers pending ..", logId(), cnt);
            _sync.notify();
        }
    }
    if (do_wait)
        Thread::WaitFor();
}

int TimeWatcherAC::SWNotifier::Execute(void)
{
//    smsc_log_debug(logger, "%s: started.", logId());
    _sync.Lock();
    _running = true;
    _sync.notify();
    while (_running || !_timers.empty()) {
        if (!_timers.empty()) {
            do {
                TimerHdl & tmr = *_timers.begin();
                _sync.Unlock();
                TimeWatcherITF::SignalResult rval = TimeWatcherITF::evtOk;
                try {
                    smsc_log_debug(logger, "%s: signaling timer[%s]", logId(), tmr.IdStr());
                    rval = _watcher->SignalTimer(tmr.Id());
                } catch (const std::exception & exc) {
                    smsc_log_error(logger, "%s: timer[%s] listener exception: %s", logId(), tmr.IdStr(), exc.what());
                } catch (...) {
                    smsc_log_error(logger, "%s: timer[%s] listener unknown exception", logId(), tmr.IdStr());
                }
                _sync.Lock();
                if (rval == TimeWatcherITF::evtOk)
                    _timers.pop_front(); //Unrefs timer
            } while (!_timers.empty());
        }
        _sync.wait(TIMEOUT_STEP); //unlocks and waits
    }
    _sync.Unlock();
    smsc_log_debug(logger, "%s: stopped", logId());
    return 0;
}

//void TimeWatcherAC::SWNotifier::signalTimer(const TimerHdl & sw_id)
void TimeWatcherAC::SWNotifier::signalTimer(uint32_t sw_id)
{
    MutexGuard grd(_sync);
    TimerHdl  tHdl(sw_id, _watcher);
    if (tHdl.Id()) {
      _timers.push_back(tHdl);
      _sync.notify();
    }
}

/* ************************************************************************** *
 * class TimeWatcherAC implementation:
 * ************************************************************************** */
TimeWatcherAC::TimeWatcherAC(const char * use_id, SWQueueITF & use_queue,
                             uint32_t init_tmrs/* = 0*/, Logger * uselog/* = NULL*/)
    : _running(false), _ident(use_id), _idStr("TmWT[")
    , _lastId(0), _swStore(use_queue)
{
    _idStr += use_id; _idStr += ']';
    logger = uselog ? uselog : Logger::getInstance(TIMEWATCHER_DFLT_LOGGER);
    _ntfr = new TimeWatcherAC::SWNotifier(use_id, this, logger);
    _swStore.reserve(init_tmrs ? init_tmrs : 100);
}

TimeWatcherAC::~TimeWatcherAC()
{
    Stop(true);
    MutexGuard tmpGrd(_sync);
    delete _ntfr;
    _swStore.clear();
}

void TimeWatcherAC::Reserve(uint32_t num_tmrs)
{
    MutexGuard grd(_sync);
    _swStore.reserve(num_tmrs); //reallocate only if num_tmrs > size()
}

bool TimeWatcherAC::isRunning(void) const
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

void TimeWatcherAC::Stop(bool do_wait/* = false*/)
{
    {
        MutexGuard grd(_sync);
        if (_running) {
            smsc_log_debug(logger, "%s: stopping.", logId());
            _running = false;
            _sync.notify();
        }
    }
    if (do_wait)
        Thread::WaitFor();
}

TimerHdl TimeWatcherAC::_createTimer(TimerListenerITF * listener,
                                  const SWTgtTime & tgt_time,
                                  OPAQUE_OBJ * opaque_obj/* = NULL*/)
{
    TimeWatcherAC::StopWatch * pSw = NULL;
    {
        MutexGuard tmp(_sync);
        if (!(pSw = _swStore.getIdleTimer())) {
          uint32_t    swId = ++_lastId;
          if (swId)
            _swStore.addTimer(pSw = new TimeWatcherAC::StopWatch(swId, this, _ident));
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
    if (!_ntfr->Start())
        return tmwUnexpected;

//    smsc_log_debug(logger, "%s: started.", logId());
    _sync.Lock();
    _running = true;
    _sync.notify();
    while (_running) {
        struct timespec tms, tmNext;
        getTime(tms);
        if (!_swStore.Queue().empty()) {
            //determine expired timers, pass them to Notifier,
            //determine next time to awake
            do {
                const SWQueueITF::SWIdTime & swt = _swStore.Queue().front();
                if (tms < swt.swTms) {
                    tmNext = swt.swTms;
                    break;
                }
                //signal expired timer
                uint32_t sw_id = 0;
                {
                  SWGuard pSw(_swStore, swt.swId);
                  smsc_log_debug(logger, "%s: timer[%s] is expired", logId(), pSw->IdStr());
                  if (pSw->setToSignal())
                    sw_id = swt.swId;
                  else  //internal inconsistency
                    smsc_log_error(logger, "%s: timer[%s] is expired, but its state: %u",
                                   logId(), pSw->IdStr(), pSw->State());
                }
                _swStore.Queue().pop_front();
                _sync.Unlock();

                if (sw_id)
                    _ntfr->signalTimer(sw_id);

                _sync.Lock();
            } while (!_swStore.Queue().empty());
        }
        if (_swStore.Queue().empty())
            tmNext = TIMEOUT_STEP.adjust2Nano(&tms);
        _sync.wait(tmNext);
    } /* eow */

    if (!_swStore.Queue().empty()) {
        smsc_log_warn(logger, "%s: aborting %u remaining timers ..", logId(),
                      _swStore.Queue().size());
        do {
            const SWQueueITF::SWIdTime & swt = _swStore.Queue().front();
            //try to signal aborted timer
            uint32_t sw_id = 0;
            {
              SWGuard pSw(_swStore, swt.swId);
              smsc_log_debug(logger, "%s: timer[%s] is being aborted", logId(), pSw->IdStr());
              if (pSw->setToSignal())
                sw_id = swt.swId;
              else  //internal inconsistency
                smsc_log_debug(logger, "%s: timer[%s] is being aborted, but its state: %u",
                               logId(), pSw->IdStr(), pSw->State());
            }
            _swStore.Queue().pop_front();
            _sync.Unlock();

            if (sw_id)
                _ntfr->signalTimer(sw_id);

            _sync.Lock();
        } while (!_swStore.Queue().empty());
    }
    _sync.Unlock();
    
    _ntfr->Stop(true); //wait notifier completion
    smsc_log_debug(logger, "%s: stopped.", logId());
    return tmwStopped;
}

// -- -------------------------------- --
// -- TimeWatcherITF interface methods
// -- -------------------------------- --
const char * TimeWatcherAC::IdStr(uint32_t tmr_id) const
{
  MutexGuard grd(_sync);
  SWGuard pSw(_swStore, tmr_id);
  return pSw.get() ? pSw->IdStr() : "unknown";
}
//
TMError TimeWatcherAC::StartTimer(uint32_t tmr_id)
{
    MutexGuard grd(_sync);
    if (!_running)
      return TimeWatcherITF::errWatcherState;

    SWGuard gsw(_swStore, tmr_id);
    if (!gsw.get())
      return TimeWatcherITF::errBadTimer;

    struct timespec cur_time;
    getTime(cur_time);
    struct timespec tgt_time = gsw->tgtTime(&cur_time);
    if (!(cur_time < tgt_time))
      return TimeWatcherITF::errBadTimeVal;

    if (_swStore.activateTimer(gsw.get(), tgt_time)) {
      _sync.notify();
      return TimeWatcherITF::errOk;
    }
    smsc_log_warn(logger, "%s: timer[%s] activation failed", logId(), gsw->IdStr());
    return TimeWatcherITF::errTimerState;
}
//
TMError TimeWatcherAC::StopTimer(uint32_t tmr_id)
{
    MutexGuard grd(_sync);
    SWGuard pSw(_swStore, tmr_id);
    if (!pSw.get())
      return TimeWatcherITF::errBadTimer;

    if (pSw->State() == TimeWatcherAC::StopWatch::swActive)
      _swStore.cancelTimer(tmr_id);
    pSw->stop();
    return TimeWatcherITF::errOk;
}
//
TMError TimeWatcherAC::RefTimer(uint32_t tmr_id)
{
    MutexGuard grd(_sync);
    SWGuard pSw(_swStore, tmr_id);
    if (!pSw.get())
      return TimeWatcherITF::errBadTimer;

    return pSw->Ref() ? TimeWatcherITF::errOk
                      : TimeWatcherITF::errTimerState;
}
//
TMError TimeWatcherAC::UnRefTimer(uint32_t tmr_id)
{
    MutexGuard grd(_sync);
    SWGuard pSw(_swStore, tmr_id);
    if (!pSw.get())
      return TimeWatcherITF::errBadTimer;

    if (pSw->UnRef() && pSw->release()) {
      _swStore.releaseTimer(pSw.get());
      smsc_log_debug(logger, "%s: timer[%s] moved to pool", logId(), pSw->IdStr());
    }
    return TimeWatcherITF::errOk;
}

//NOTE:StopWatch was already scheduled for signaling (possibly later cancelled)
TimeWatcherITF::SignalResult TimeWatcherAC::SignalTimer(uint32_t tmr_id)
{
    TimeWatcherAC::StopWatch * pSw = getTimer(tmr_id);
    return pSw ? pSw->notify() : TimeWatcherITF::evtOk;
}

/* ************************************************************************** *
 * class TimeWatchersRegistry implementation:
 * ************************************************************************** */
bool TimeWatchersRegistry::StartAll(void)
{
    bool failed = false;
    if (twMain.get())
        failed |= !twMain->Start();

    for (TWRegistry::const_iterator it = twReg.begin(); it != twReg.end(); ++it)
        failed |= !it->second->Start();

    return !failed;
}

void TimeWatchersRegistry::StopAll(bool do_wait/* = false*/)
{
    if (twMain.get())
        twMain->Stop(do_wait);

    for (TWRegistry::const_iterator it = twReg.begin(); it != twReg.end(); ++it)
        it->second->Stop(do_wait);
}

//Returns generic TimeWatcher
TimeWatcher * TimeWatchersRegistry::getTimeWatcher(uint32_t num_tmrs/* = 0*/,
                                                   bool do_start/* = true*/)
{
    if (!twMain.get()) {
        twMain.reset(new TimeWatcher("0", num_tmrs, logger));
    } else if (num_tmrs)
        twMain->Reserve(num_tmrs);

    if (!twMain->isRunning() && do_start) {
        smsc_log_info(logger, "TWReg: starting generic %s", twMain->logId());
        twMain->Start();
    }
    return twMain.get();
}

//Returns TimeWatcherUNI adjusted for given timeout
TimeWatcherTMO * TimeWatchersRegistry::getTmoTimeWatcher(
                            const TimeSlice & use_tmo,
                            uint32_t num_tmrs/* = 0*/, bool do_start/* = true*/)
{
    TimeWatcherTMO * pTw = NULL;
    {
        SWTgtTime   tmo(use_tmo);
        if (!(pTw = twReg.find(tmo))) {
            char buf[sizeof(uint32_t)*3 + 2];
            snprintf(buf, sizeof(buf)-1, "%u", ++_lastId);
            pTw = new TimeWatcherTMO(buf, num_tmrs, use_tmo, logger);
            twReg.insert(tmo, pTw);
        } else if (num_tmrs)
            pTw->Reserve(num_tmrs);

        if (!pTw->isRunning() && do_start) {
            smsc_log_info(logger, "TWReg: starting %s, tmo: %ld %s", pTw->logId(),
                          use_tmo.Value(), use_tmo.nmUnit());
            pTw->Start();
        }
    }
    return pTw;
}

} //timers
} //core
} //smsc

