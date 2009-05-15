#include <utility>

#include "TimeoutMonitor.hpp"
#include "util/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/tcap/provd/TDialogueServiceData.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TimeoutMonitor::TimeoutMonitor(unsigned maxTimeoutValue)
  : _logger(smsc::logger::Logger::getInstance("tmtmon")),
    _isRunning(true), _maxTimeoutValue(maxTimeoutValue),
    _timeoutIdSourceGenerator(1)
{
  _knownTimeouts = new TimeoutsEntryInfo[maxTimeoutValue];
}

TimeoutMonitor::~TimeoutMonitor()
{
  delete [] _knownTimeouts;
}

#if 0

#endif

int
TimeoutMonitor::Execute()
{
  while (_isRunning) {
    used_timeouts_t usedTimeouts;
    {
      smsc::core::synchronization::MutexGuard synchronize(_usedTimeoutsLock);
      usedTimeouts = _usedTimeouts;
    }
    long sleepInterval=0;
    for (used_timeouts_t::iterator iter = usedTimeouts.begin(), end_iter = usedTimeouts.end();
         iter != end_iter; ++iter) {
      TimeoutsEntryInfo& timeoutEntryInf = _knownTimeouts[*iter];

      smsc::core::synchronization::MutexGuard synchronize(timeoutEntryInf.entryLock);
      if ( !timeoutEntryInf.activeTimeouts )
        continue;

      for ( active_timeouts_t::reverse_iterator sIter = timeoutEntryInf.activeTimeouts->rbegin(), eIter = timeoutEntryInf.activeTimeouts->rend();
            sIter != eIter;) {
        struct timeval currentTime;
        gettimeofday(&currentTime,0);

        const struct timeval& timerExpirationValue = sIter->timeToExpire;
        if ( currentTime.tv_sec < timerExpirationValue.tv_sec ||
             (currentTime.tv_sec == timerExpirationValue.tv_sec && currentTime.tv_usec < timerExpirationValue.tv_usec) ) {
          sleepInterval = std::min((timerExpirationValue.tv_sec - currentTime.tv_sec) * 1000 + timerExpirationValue.tv_usec - timerExpirationValue.tv_sec, sleepInterval);
          break;
        }
        timeout_id_t expiredTimeoutId = sIter->timeoutId;
        unregisterTimeoutId(expiredTimeoutId);

        sIter->timeoutHandler->handle();

        active_timeouts_t::iterator iterToRemovingElement = sIter.base(); sIter++;
        timeoutEntryInf.activeTimeouts->erase(iterToRemovingElement);
      }
    }
    waitForNewScheduledTimeouts(static_cast<int>(sleepInterval));
  }
  return 0;
}

void
TimeoutMonitor::waitForNewScheduledTimeouts(int sleepInterval)
{
  _timeoutScheduleEvent.wait(sleepInterval);
}

TimeoutMonitor::timeout_id_t
TimeoutMonitor::schedule(unsigned int timeoutValue,
                         TimeoutHandler* timeoutHandler)
{
  if ( timeoutValue > _maxTimeoutValue ) {
    smsc_log_error(_logger, "TimeoutMonitor::schedule::: invalid timeout value=%d, max possible value is %d seconds", timeoutValue, _maxTimeoutValue);
    throw smsc::util::Exception("TimeoutMonitor::schedule::: invalid timeout value");
  }

  TimeoutsEntryInfo& timeoutEntryInf = _knownTimeouts[timeoutValue];
  smsc::core::synchronization::MutexGuard synchronize(timeoutEntryInf.entryLock);
  if ( !_knownTimeouts[timeoutValue].activeTimeouts ) {
    _knownTimeouts[timeoutValue].activeTimeouts = new active_timeouts_t();

    smsc::core::synchronization::MutexGuard anotherSynchronize(_usedTimeoutsLock);
    _knownTimeouts[timeoutValue].iterToUsedTimeouts = _usedTimeouts.insert(_usedTimeouts.end(), timeoutValue);
  }
  active_timeouts_t* activeTimeouts = _knownTimeouts[timeoutValue].activeTimeouts;

  timeout_id_t timeoutId = generateTimeoutId();
  active_timeouts_t::iterator ins_iter = activeTimeouts->insert(activeTimeouts->end(),
                                                                TimeoutInfo(timeoutId,
                                                                            timeoutValue,
                                                                            timeoutHandler)
                                                                );
  registerTimeoutId(timeoutId, timeoutValue, ins_iter);

  _timeoutScheduleEvent.notify();
  return timeoutId;
}

void
TimeoutMonitor::registerTimeoutId(timeout_id_t timeoutId,
                                  unsigned int timeoutValue,
                                  active_timeouts_t::iterator ins_iter)
{
  smsc::core::synchronization::MutexGuard synchronize(_timeoutRefsRegistryLock);
  _timeoutRefsRgistry.Insert(timeoutId, timeout_ref_t(timeoutValue, ins_iter));
}

void
TimeoutMonitor::unregisterTimeoutId(timeout_id_t timeoutId)
{
  smsc::core::synchronization::MutexGuard synchronize(_timeoutRefsRegistryLock);
  _timeoutRefsRgistry.Delete(timeoutId);
}

void
TimeoutMonitor::cancel(timeout_id_t timeoutId)
{
  timeout_ref_t timeoutRef;
  {
    smsc::core::synchronization::MutexGuard synchronize(_timeoutRefsRegistryLock);
    if ( !_timeoutRefsRgistry.Get(timeoutId, timeoutRef) )
      return;
    _timeoutRefsRgistry.Delete(timeoutId);
  }

  TimeoutsEntryInfo& timeoutEntryInf = _knownTimeouts[timeoutRef.timeoutValue];
  smsc::core::synchronization::MutexGuard synchronize(timeoutEntryInf.entryLock);
  if ( timeoutEntryInf.activeTimeouts ) {
    timeoutEntryInf.activeTimeouts->erase(timeoutRef.iterToActiveTimeout);
    if ( timeoutEntryInf.activeTimeouts->empty() ) {
      delete timeoutEntryInf.activeTimeouts; timeoutEntryInf.activeTimeouts = NULL;
      smsc::core::synchronization::MutexGuard anotherSynchronize(_usedTimeoutsLock);
      _usedTimeouts.erase(timeoutEntryInf.iterToUsedTimeouts);
    }
  }
}

void
TimeoutMonitor::restart(unsigned int timeoutValue, timeout_id_t timeoutId)
{
}

TimeoutMonitor::timeout_id_t
TimeoutMonitor::generateTimeoutId()
{
  return _timeoutIdSourceGenerator++;
}

}}}

