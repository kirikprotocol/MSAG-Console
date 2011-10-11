#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "core/timers/TimerNotifier.hpp"

namespace smsc {
namespace core {
namespace timers {

using smsc::core::synchronization::MutexGuard;

/* ************************************************************************** *
 * class TimerNotifier implementation:
 * ************************************************************************** */
const TimeSlice TimerNotifier::k_dfltShutdownTmo(150, TimeSlice::tuMSecs);

TimerNotifier::TimerNotifier(const char * use_ident, Logger * use_log/* = NULL*/)
  : mIniThreads(1)
  , mLogger(use_log ? use_log : Logger::getInstance(TIMERS_MONITOR_DFLT_LOGGER))
{
  snprintf(mLogId, sizeof(mLogId)-1, "TmNtfr[%s]", use_ident);
}

TimerNotifier::~TimerNotifier()
{
  if (isRunning())
    stop();
}

void TimerNotifier::init(uint16_t ini_threads, uint16_t max_threads/* = 0*/)
{
  setMaxThreads(max_threads);
  if (max_threads && (ini_threads > max_threads))
    ini_threads = max_threads;
  if (ini_threads > mIniThreads)
    mIniThreads = ini_threads;
  mTaskPool.reserve(mIniThreads);
}

void TimerNotifier::stop(const TimeSlice * use_tmo/* = NULL*/)
{
  if (!use_tmo)
    use_tmo = &k_dfltShutdownTmo;
  shutdown(use_tmo->Units(), use_tmo->Value());
}

// ------------------------------------------
// -- TimerRefereeIface interface methods
// ------------------------------------------
//Returns false if event cann't be processed by referee.
//Starts a threaded task that processes query event.
bool TimerNotifier::onTimerEvent(const TimerRef & p_tmr)
{
  if (p_tmr.empty()) {
    smsc_log_warn(mLogger, "%s: onTimerEvent() for empty TimerRef", mLogId);
    return true;
  }
  NTFTaskRef  pTask = mTaskPool.allcObj();
  if (!pTask.empty()) {
    if (pTask->init(pTask, p_tmr, mLogId, mLogger) && startTask(pTask.get())) {
      smsc_log_debug(mLogger, "%s: activated %s for Timer[%s]",
                     mLogId, pTask->taskName(), p_tmr->getStatStr());
      return true;
    }
    smsc_log_fatal(mLogger, "%s: failed to activate task %s for Timer[%s]",
                   mLogId, pTask->taskName(), p_tmr->getStatStr());
    pTask->onRelease();
  } else {
    smsc_log_error(mLogger, "%s: task pool is exhausted: %u of %u", mLogId,
                   (unsigned)mTaskPool.usage(), (unsigned)mTaskPool.capacity());
  }
  return false;
}

/* ************************************************************************** *
 * class TimerNotifier::NotificationTask implementation:
 * ************************************************************************** */
bool TimerNotifier::NotificationTask::init(const NTFTaskGuard & task_grd,
                                           const TimerRef & p_tmr, const char * log_id,
                                           Logger * use_log/* = NULL*/)
{
  if ((mTmrRef = p_tmr).empty())
    return false;

  isStopping = isReleased = false;
  if (!(mLogger = use_log))
    mLogger = Logger::getInstance(TIMERS_MONITOR_DFLT_LOGGER);
  mThisGrd = task_grd;
  snprintf(mLogId, sizeof(mLogId)-1, "%s[%u]", log_id, this->getUIdx());
  return true;
}

// -------------------------------------------
// -- ThreadedTask interface methods
// -------------------------------------------
int TimerNotifier::NotificationTask::Execute(void)
{
  if (mTmrRef.empty())
    return -1;

  TimerEventResult_e rval = TimerListenerIface::evtOk;
  do {
    smsc_log_debug(mLogger, "%s: signalling Timer[%s] ..", mLogId, mTmrRef->getStatStr());
    {
      MutexGuard  grd(*mTmrRef.get());
      rval = mTmrRef->notify();
    }
    if (rval == TimerListenerIface::evtResignal) {
      smsc_log_debug(mLogger, "%s: Timer[%s] is to resignal ..", mLogId, mTmrRef->getStatStr());
      struct timespec tv = {0, 2000000}; //2 ms
      nanosleep(&tv, 0);
    }
  } while ((rval == TimerListenerIface::evtResignal) && !isStopping);
  if (rval == TimerListenerIface::evtResignal) {
    smsc_log_warn(mLogger, "%s: Timer[%s] signalling is cancelled", mLogId, mTmrRef->getStatStr());
  }
  return 0;
}

void TimerNotifier::NotificationTask::onRelease(void)
{
  mTmrRef.release();
  isReleased = true;
  mThisGrd.release();
}

} //timers
} //core
} //smsc

