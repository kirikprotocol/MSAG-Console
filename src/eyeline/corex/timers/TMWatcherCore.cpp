#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/corex/timers/TMWatcherCore.hpp"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::core::synchronization::MutexGuard;

static TimeSlice    _DFLT_SHUTDOWN_TIMEOUT(100, TimeSlice::tuMSecs);
static TimeSlice    _DFLT_SLEEP_TIMEOUT(100, TimeSlice::tuMSecs);

/* ************************************************************************** *
 * class TimeWatcherCore implementation:
 * ************************************************************************** */
TimeWatcherCore::TimeWatcherCore(const TimeWatcherIdent & use_id,
                                 ExpirationQueueIface & use_queue,
                                 Logger * uselog/* = NULL*/)
  : _runState(twrsStopped), _shdTmo(_DFLT_SHUTDOWN_TIMEOUT.Value(), _DFLT_SHUTDOWN_TIMEOUT.Units())
  ,  _ident(use_id), _logId("TmWT[")
  , _logger(uselog ? uselog : Logger::getInstance(TIMEWATCHER_DFLT_LOGGER))
  , _swStore(_ident), _swNotifier(_ident, _swStore, _logger), _expQueue(use_queue)
{
  _logId += use_id; _logId += ']';
}

TimeWatcherCore::~TimeWatcherCore()
{
  Stop(_DFLT_SHUTDOWN_TIMEOUT.Units(), _DFLT_SHUTDOWN_TIMEOUT.Value());
}

//
void TimeWatcherCore::initTimers(uint32_t num_tmrs)
{
  _swStore.reserve(num_tmrs);
}
//
void TimeWatcherCore::initThreads(unsigned ini_threads, unsigned max_threads/* = 0*/)
{
  _swNotifier.Init(ini_threads, max_threads);
}


bool TimeWatcherCore::isRunning(void) const
{
  MutexGuard grd(_sync);
  return _runState != twrsStopped;
}

bool TimeWatcherCore::Start(void)
{
  {
    MutexGuard grd(_sync);
    if (_runState == twrsRunning)
      return true;
    if (_runState == twrsStopping) {
      smsc_log_warn(_logger, "%s::Start(): inconsistent state \'twrsStopping\'", logId());
      return false;
    }
  }
  smsc_log_debug(_logger, "%s: starting ..", logId());
  _swNotifier.Start();
  Thread::Start();
  {
    MutexGuard grd(_sync);
    if (_runState == twrsRunning)
      return true;
    _sync.wait(_DFLT_SLEEP_TIMEOUT);
    if (_runState != twrsRunning) {
      smsc_log_fatal(_logger, "%s: unable to start", logId());
      return false;
    }
  }
  return true;
}

void TimeWatcherCore::StopNotify(void)
{
  MutexGuard grd(_sync);
  if (_runState == twrsRunning) {
    smsc_log_debug(_logger, "%s: stopping.", logId());
    _runState = twrsStopping;
    _sync.notify();
  }
}

void TimeWatcherCore::Stop(TimeSlice::UnitType_e time_unit, long use_tmo)
{
  StopNotify();
  {
    MutexGuard grd(_sync);
    _shdTmo  = TimeSlice(use_tmo, time_unit);
  }
  Thread::WaitFor();
}


// -- -------------------------
// -- Thread interface methods
// -- -------------------------
int TimeWatcherCore::Execute(void)
{
//    smsc_log_debug(_logger, "%s: started.", logId());
  _sync.Lock();
  _runState = twrsRunning;
  _sync.notify();
  while (_runState == twrsRunning) {
    uint32_t        swId = 0;
    struct timespec curTms;

    TimeSlice::getRealTime(curTms);
    while (_expQueue.popExpired(curTms, swId)) {
      ReverseMutexGuard rgrd(_sync);
      {
        StopWatchGuard  pSw;
        _swStore.getTimer(swId, pSw);
        if (pSw.get()) {
          smsc_log_debug(_logger, "%s: timer[%s] is expired", logId(), pSw->IdStr());

          if (pSw->setToSignal()) {
            pSw->Ref();
            _swNotifier.signalTimer(swId);
          } else //internal inconsistency!!!
            smsc_log_error(_logger, "%s: timer[%s] is expired, but its state: %u",
                           logId(), pSw->IdStr(), pSw->State());
        }
      }
    }
    //determine next time to awake
    struct timespec nextTms;
    if (!_expQueue.nextTime(nextTms))
      nextTms = _DFLT_SLEEP_TIMEOUT.adjust2Nano();
    _sync.wait(nextTms);
  } /* eow */

  if (!_expQueue.empty()) {
    smsc_log_warn(_logger, "%s: aborting %u remaining timers ..", logId(), _expQueue.size());
    uint32_t        swId = 0;
    struct timespec curTms;

    _expQueue.lastTime(curTms);
    while (_expQueue.popExpired(curTms, swId)) {
      ReverseMutexGuard rgrd(_sync);
      {
        StopWatchGuard pSw;
        _swStore.getTimer(swId, pSw);
        if (pSw.get()) {
          smsc_log_debug(_logger, "%s: timer[%s] is being aborted", logId(), pSw->IdStr());

          if (pSw->setToSignal())
            _swNotifier.signalTimer(swId);
          else //internal inconsistency!!!
            smsc_log_error(_logger, "%s: timer[%s] is being aborted, but its state: %u",
                           logId(), pSw->IdStr(), pSw->State());
        }
      }

    }
  }
  _sync.Unlock();
  _swNotifier.Stop(_shdTmo.Units(), _shdTmo.Value()); //wait notifier completion
  _runState = twrsStopped;
  smsc_log_debug(_logger, "%s: stopped.", logId());
  return tmwStopped;
}



TimeWatcherCore::TimerHdlComposer
  TimeWatcherCore::_createTimer(TimerListenerIface * listener,
                                  const ExpirationTime & tgt_time,
                                  const OpaqueArg * opaque_obj/* = NULL*/)
{
  StopWatchGuard pSw;
  _swStore.allcTimer(pSw);
  if (!pSw.get()) {
    smsc_log_error(_logger, "%s: unable to allocate timer", logId());
    return TimerHdlComposer();
  }
  if (!pSw->init(listener, tgt_time, opaque_obj)) {
    smsc_log_error(_logger, "%s: internal inconsistency"
                            " (timer[%s] init failed)!", logId(), pSw->IdStr());
    return TimerHdlComposer();
  }
  return TimerHdlComposer(pSw->Id(), this);
}

// -- -------------------------------- --
// -- TimeWatcherIface interface methods
// -- -------------------------------- --
const char * TimeWatcherCore::getTimerIdent(uint32_t tmr_id) const
{
  StopWatchGuard pSw;
  _swStore.getTimer(tmr_id, pSw);
  return pSw.get() ? pSw->IdStr() : "unknown";
}

//
TimerError_e TimeWatcherCore::startTimer(uint32_t tmr_id)
{
  StopWatchGuard pSw;
  _swStore.getTimer(tmr_id, pSw);
  if (!pSw.get())
    return TimeWatcherIface::errBadTimer;

  struct timespec curTms;
  TimeSlice::getRealTime(curTms);

  struct timespec tgt_time = pSw->tgtTime().adjust2TMS(&curTms);
  if (!(curTms < tgt_time))
    return TimeWatcherIface::errBadTimeVal;

  MutexGuard grd(_sync);
  {
    if (_runState != twrsRunning)
      return TimeWatcherIface::errWatcherState;
  
    if (pSw->activate() && _expQueue.insert(tmr_id, pSw->tgtTime())) {
      pSw->Ref();
      _sync.notify();
      return TimeWatcherIface::errOk;
    }
  }
  smsc_log_warn(_logger, "%s: timer[%s] activation failed", logId(), pSw->IdStr());
  return TimeWatcherIface::errTimerState;
}

//
TimerError_e TimeWatcherCore::stopTimer(uint32_t tmr_id)
{
  StopWatchGuard pSw;
  _swStore.getTimer(tmr_id, pSw);
  if (!pSw.get())
    return TimeWatcherIface::errBadTimer;

  if (pSw->State() == StopWatchFSM::swActive) {
    MutexGuard grd(_sync);
    _expQueue.erase(tmr_id, pSw->tgtTime());
    pSw->UnRef();
  }
  if (!pSw->stop()) {
    smsc_log_debug(_logger, "%s: stopping timer[%s] in state \'swSignalling\'", logId(), pSw->IdStr());
    return TimeWatcherIface::errTimerState;
  }
  return TimeWatcherIface::errOk;
}

//
TimerError_e TimeWatcherCore::refTimer(uint32_t tmr_id)
{
  return _swStore.refTimer(tmr_id) ? TimeWatcherIface::errOk 
                                  : TimeWatcherIface::errBadTimer;
}

//
TimerError_e TimeWatcherCore::unRefTimer(uint32_t tmr_id)
{
  return _swStore.unrefTimer(tmr_id) ? TimeWatcherIface::errOk 
                                  : TimeWatcherIface::errBadTimer;
}

} //timers
} //corex
} //eyeline

