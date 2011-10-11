#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "core/timers/TimersMonitorCore.hpp"

namespace smsc {
namespace core {
namespace timers {

using smsc::core::threads::Thread;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::ReverseMutexGuard;
/* ************************************************************************** *
 * class TimerHdl implementation:
 * ************************************************************************** */
TimerHdl::Error_e TimerHdl::resultToErr(TimerFSM::ActivationResult_e res_val)
{
  switch (res_val) {
  case TimerFSM::actOk:       return errOk;
  case TimerFSM::actBadTime:  return errBadTimeVal;
  case TimerFSM::actFsmState: return errTimerState;
  case TimerFSM::actRfrState: return errMonitorState;
  }
  return errBadTimer;
}
//Returns timer statistics string or "NULL" in case of uninitialized timer.
const char * TimerHdl::getStatStr(void) const
{
  if (!mSwRef.empty()) {
    MutexGuard swGrd(*mSwRef.get());
    return mSwRef->getStatStr();
  }
  return "NULL";
}
//Starts timer monitoring.
TimerHdl::Error_e TimerHdl::start(void) const
{
  if (!mSwRef.empty()) {
    MutexGuard swGrd(*mSwRef.get());
    return resultToErr(mSwRef->activate(mSwRef));
  }
  return errBadTimer;
}
//Stops timer expiration monitoring. If timer's expiration time is of timeout form,
//timer may be started again.
//NOTE: 'errTimerState' is returned if timer is currently signalling
TimerHdl::Error_e TimerHdl::stop(void) const
{
  if (!mSwRef.empty()) {
    MutexGuard swGrd(*mSwRef.get());
    return mSwRef->stop() ? errOk : errTimerState;
  }
  return errOk;
}

/* ************************************************************************** *
 * class TimersMonitorCore implementation:
 * ************************************************************************** */
const TimeSlice TimersMonitorCore::k_dfltSleepTmo(400, TimeSlice::tuMSecs);

TimersMonitorCore::TimersMonitorCore(const char * id_pfx,
                                   ExpirationQueueIface & use_queue,
                                   Logger * use_log/* = NULL*/)
  : mRunState(monStopped), mpShtdTmo(0), mIdent("TmWT["), mExpQueue(use_queue)
  , mLogger(use_log ? use_log : Logger::getInstance(TIMERS_MONITOR_DFLT_LOGGER))
  , mSwNtfr(id_pfx, mLogger)
{
  strncpy(mIdPfx, id_pfx, sizeof(mIdPfx) - 1);
  mIdPfx[sizeof(mIdPfx) - 1] = 0;
  mIdent += id_pfx; mIdent += ']';
}

TimersMonitorCore::~TimersMonitorCore()
{
  stop();
  TimersPool::size_type swUsed = mSwStore.usage();
  if (swUsed) {
    smsc_log_warn(mLogger, "%s %lu timers still allocated upon destruction",
                  logId(), (long)swUsed);
  }
}

//Reserves resources for fiven number of TimerFSM objects.
void TimersMonitorCore::reserveTimers(TimerUId num_tmrs)
{
  MutexGuard grd(mSync);
  mSwStore.reserve(num_tmrs);
}

bool TimersMonitorCore::isState(RunState_e tgt_state) const
{
  MutexGuard grd(mSync);
  return mRunState == tgt_state;
}

bool TimersMonitorCore::start(void)
{
  {
    MutexGuard grd(mSync);
    if (mRunState == monRunning)
      return true;
    if (mRunState == monStopping) {
      smsc_log_warn(mLogger, "%s::Start(): inconsistent state \'monStopping\'", logId());
      return false;
    }
  }
  smsc_log_debug(mLogger, "%s: starting ..", logId());
  mSwNtfr.start();
  Thread::Start();
  {
    MutexGuard grd(mSync);
    int rc = Thread::getRetCode();
    if (rc) {
      smsc_log_fatal(mLogger, "%s: monitoring thread start is failed: %s (%d)",
                      logId(), strerror(rc), rc);
      return false;
    }
    if (mRunState == monRunning)
      return true;
    mSync.wait(k_dfltSleepTmo);
    if (mRunState != monRunning) {
      smsc_log_fatal(mLogger, "%s: monitoring thread start is timed out", logId());
      return false;
    }
  }
  return true;
}

void TimersMonitorCore::stopNotify(void)
{
  MutexGuard grd(mSync);
  if (mRunState == monRunning) {
    smsc_log_debug(mLogger, "%s: stopping.", logId());
    mRunState = monStopping;
    mSync.notify();
  }
}

void TimersMonitorCore::stop(const TimeSlice * use_tmo/* = NULL*/)
{
  {
    MutexGuard grd(mSync);
    if (mRunState == monStopped)
      return;
    if (mRunState == monRunning) {
      smsc_log_debug(mLogger, "%s: stopping.", logId());
      mRunState = monStopping;
      mpShtdTmo = use_tmo;
      mSync.notify();
    }
  }
  Thread::WaitFor();
}

// -- -------------------------
// -- Thread interface methods
// -- -------------------------
int TimersMonitorCore::Execute(void)
{
  MutexGuard grd(mSync);
  mRunState = monRunning;
  Thread::setRetCode(0);
  mSync.notify();

  while (mRunState == monRunning) {
    TimerRef        swRef;
    struct timespec curTms;

    TimeSlice::getRealTime(curTms);
    while (mExpQueue.popExpired(curTms, swRef)) {
      ReverseMutexGuard rGrd(mSync);
      {
        MutexGuard swGrd(*swRef.get());
        if (swRef->setToSignal()) {
          smsc_log_debug(mLogger, "%s: Timer[%s] is expired", logId(), swRef->getStatStr());
          mSwNtfr.onTimerEvent(swRef);
        } else //internal inconsistency!!!
          smsc_log_fatal(mLogger, "%s: Timer[%s] is expired, having inconsistent state: %s",
                         logId(), swRef->getStatStr(), swRef->nmState());
      }
    }
    //determine next time to awake
    struct timespec nextTms;
    if (!mExpQueue.nextTime(nextTms))
      nextTms = k_dfltSleepTmo.adjust2Nano();
//    smsc_log_debug(mLogger, "%s: sleep until {sec:%lu, nsec:%lu}", logId(), nextTms.tv_sec, nextTms.tv_nsec);
    mSync.wait(nextTms);
  } /* eow */

  if (!mExpQueue.empty()) {
    smsc_log_warn(mLogger, "%s: aborting %u remaining timers ..", logId(), mExpQueue.size());
    TimerRef        swRef;
    struct timespec curTms;

    mExpQueue.lastTime(curTms);
    while (mExpQueue.popExpired(curTms, swRef)) {
      ReverseMutexGuard rGrd(mSync);
      {
        MutexGuard swGrd(*swRef.get());
        if (swRef->setToSignal()) {
          smsc_log_debug(mLogger, "%s: Timer[%s] is being aborted", logId(), swRef->getStatStr());
          mSwNtfr.onTimerEvent(swRef);
        } else //internal inconsistency!!!
          smsc_log_error(mLogger, "%s: Timer[%s] is being aborted, having inconsistent state: %s",
                         logId(), swRef->getStatStr(), swRef->nmState());
      }
    }
  }
  {
    ReverseMutexGuard rgrd(mSync);
    mSwNtfr.stop(mpShtdTmo); //wait notifier's threads completion
  }
  smsc_log_debug(mLogger, "%s: stopped.", logId());
  mRunState = monStopped;
  mSync.notify();
  return 0;
}


// ----------------------------------------------
// -- TimerMonitorIface interface methods
// ----------------------------------------------
//Returns handle of previously allocated timer.
TimerHdl  TimersMonitorCore::getTimer(TimerUId sw_uid) /*throw()*/
{
  TimerRef pSw = mSwStore.hasObj(sw_uid);
  return pSw.empty() ? TimerHdl() : newTimerHdl(pSw);
}

TimerHdl TimersMonitorCore::allcTimer(TimerListenerIface & tmr_lsr,
                                       const ExpirationTime & tgt_time) /*throw()*/
{
  TimerRef pSw = mSwStore.allcObj();
  if (pSw.empty()) {
    smsc_log_error(mLogger, "%s: timers pool is exhausted, %u used of %u",
                   logId(), mSwStore.usage(), mSwStore.capacity());
    return TimerHdl();
  }
  {
    MutexGuard tGrd(*pSw.get());
    if (!pSw->init(*this, mIdPfx, tmr_lsr, tgt_time)) { //internal inconsistency
      smsc_log_fatal(mLogger, "%s: Timer[%s] init failed", logId(), pSw->getStatStr());
      return TimerHdl();
    }
  }
  return newTimerHdl(pSw);
}


// ----------------------------------------------
// -- TimerRefereeIface interface methods
// ----------------------------------------------
//Inserts given TimerLink to pool of monitored timers.
TimerFSM::ActivationResult_e
  TimersMonitorCore::enqueue(TimerLink & sw_link) /*throw()*/
{
  MutexGuard monGrd(mSync);
  if (mRunState != monRunning) {
    smsc_log_debug(mLogger, "%s: Timer[%s] activation denied: stopping ..",
                  logId(), sw_link->getStatStr());
    return TimerFSM::actRfrState;
  }
  if (!mExpQueue.insert(sw_link)) {
    smsc_log_warn(mLogger, "%s: Timer[%s] activation denied: unsupported expiration time",
                  logId(), sw_link->getStatStr());
    return TimerFSM::actBadTime;
  }
  mSync.notify();
  return TimerFSM::actOk;
}

//NOTE: It's a caller responsibility to ensure that given TimerLink was
//      inserted previously !!!
void TimersMonitorCore::unlink(TimerLink & sw_link) /*throw()*/
{
  MutexGuard monGrd(mSync);
  mExpQueue.unlink(sw_link);
}

} //timers
} //core
} //smsc

