#include <utility>

#include "TimeoutMonitor.hpp"
#include "util/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"

namespace eyeline {
namespace utilx {

TimeoutMonitor::TimeoutMonitor(unsigned timeout_id_seed)
  : _logger(smsc::logger::Logger::getInstance("timeoutmon")), _isRunning(true),
    _shutdownInProgress(false), _stopped(false), _maxTimeoutValue(0),
    _timeoutIdSourceGenerator(1), _knownTimeouts(NULL)
{
  _timeoutIdSourceGenerator = (timeout_id_seed << 28) | 1;
}

TimeoutMonitor::~TimeoutMonitor()
{
  delete [] _knownTimeouts;
}

void
TimeoutMonitor::initialize(unsigned int max_timeout_value)
{
  _maxTimeoutValue = max_timeout_value;
  _knownTimeouts = new TimeoutsEntryInfo[max_timeout_value];
  Start();
}

int
TimeoutMonitor::Execute()
{
  while (_isRunning) {
    used_timeouts_t usedTimeouts;
    {
      smsc::core::synchronization::MutexGuard synchronize(_usedTimeoutsLock);
      usedTimeouts = _usedTimeouts;
    }
    long sleepInterval=10000000;
    bool sleepIntervalIsValid = false;
    for (used_timeouts_t::iterator iter = usedTimeouts.begin(), end_iter = usedTimeouts.end();
         iter != end_iter; ++iter) {
      TimeoutsEntryInfo& timeoutEntryInf = _knownTimeouts[*iter];

      smsc::core::synchronization::MutexGuard synchronize(timeoutEntryInf.entryLock);
      if ( !timeoutEntryInf.activeTimeouts )
        continue;

      for ( active_timeouts_t::reverse_iterator sIter = timeoutEntryInf.activeTimeouts->rbegin(), eIter = timeoutEntryInf.activeTimeouts->rend();
            sIter != eIter; ) {
        struct timeval currentTime;
        gettimeofday(&currentTime,0);

        const struct timeval& timerExpirationValue = sIter->timeToExpire;
        if ( currentTime.tv_sec < timerExpirationValue.tv_sec ||
             (currentTime.tv_sec == timerExpirationValue.tv_sec && currentTime.tv_usec < timerExpirationValue.tv_usec) ) {
          sleepInterval = std::min((timerExpirationValue.tv_sec - currentTime.tv_sec) * 1000 + timerExpirationValue.tv_usec - timerExpirationValue.tv_sec, sleepInterval);
          sleepIntervalIsValid = true;
          break;
        }
        timeout_id_t expiredTimeoutId = sIter->timeoutId;
        unregisterTimeoutId(expiredTimeoutId);

        try {
          smsc_log_debug(_logger, "TimeoutMonitor::Execute::: timeout_handler=0x%p,expiredTimeoutId=%u",
                         sIter->timeoutHandler, expiredTimeoutId);
          sIter->timeoutHandler->handle();
        } catch (...) {}

        delete sIter->timeoutHandler;
        active_timeouts_t::iterator iterToRemovingElement = sIter.base(); sIter++;
        timeoutEntryInf.activeTimeouts->erase(--iterToRemovingElement);
      }
      if ( timeoutEntryInf.activeTimeouts->empty() ) {
        delete timeoutEntryInf.activeTimeouts; timeoutEntryInf.activeTimeouts = NULL;
        smsc::core::synchronization::MutexGuard anotherSynchronize(_usedTimeoutsLock);
        _usedTimeouts.erase(timeoutEntryInf.iterToUsedTimeouts);
      }
    }
    {
      smsc::core::synchronization::MutexGuard synchronize(_usedTimeoutsLock);
      if ( _shutdownInProgress && _usedTimeouts.empty() )
        break;
    }
    if ( !sleepIntervalIsValid )
      sleepInterval = 0;
    waitForNewScheduledTimeouts(static_cast<int>(sleepInterval));
  }
  _stopped = true;
  return 0;
}

void
TimeoutMonitor::waitForNewScheduledTimeouts(int sleep_interval)
{
  if ( sleep_interval )
    _timeoutScheduleEvent.wait(sleep_interval);
  else
    _timeoutScheduleEvent.wait();
}

void
TimeoutMonitor::shutdown()
{
  _shutdownInProgress = true;
  smsc::core::synchronization::MutexGuard synchronize(_usedTimeoutsLock);
  if ( _usedTimeouts.empty() )
    _timeoutScheduleEvent.notify();
}

TimeoutMonitor::timeout_id_t
TimeoutMonitor::schedule(unsigned int timeout_value,
                         TimeoutHandler* timeout_handler)
{
  if ( timeout_value > _maxTimeoutValue ) {
    smsc_log_error(_logger, "TimeoutMonitor::schedule::: invalid timeout value=%d, max possible value is %d seconds",
                   timeout_value, _maxTimeoutValue);
    throw smsc::util::Exception("TimeoutMonitor::schedule::: invalid timeout value");
  }

  TimeoutsEntryInfo& timeoutEntryInf = _knownTimeouts[timeout_value];
  smsc::core::synchronization::MutexGuard synchronize(timeoutEntryInf.entryLock);
  if ( !_knownTimeouts[timeout_value].activeTimeouts ) {
    _knownTimeouts[timeout_value].activeTimeouts = new active_timeouts_t();

    smsc::core::synchronization::MutexGuard anotherSynchronize(_usedTimeoutsLock);
    _knownTimeouts[timeout_value].iterToUsedTimeouts = _usedTimeouts.insert(_usedTimeouts.end(),
                                                                            timeout_value);
  }
  active_timeouts_t* activeTimeouts = _knownTimeouts[timeout_value].activeTimeouts;

  timeout_id_t timeoutId = generateTimeoutId();
  active_timeouts_t::iterator ins_iter = activeTimeouts->insert(activeTimeouts->end(),
                                                                TimeoutInfo(timeoutId,
                                                                            timeout_value,
                                                                            timeout_handler)
                                                                );
  registerTimeoutId(timeoutId, timeout_value, ins_iter);
  smsc_log_debug(_logger, "TimeoutMonitor::schedule::: timeout_handler=0x%p,timeoutId=%u",
                 timeout_handler, timeoutId);
  _timeoutScheduleEvent.notify();
  return timeoutId;
}

void
TimeoutMonitor::registerTimeoutId(timeout_id_t timeout_id,
                                  unsigned int timeout_value,
                                  active_timeouts_t::iterator ins_iter)
{
  smsc::core::synchronization::MutexGuard synchronize(_timeoutRefsRegistryLock);
  _timeoutRefsRgistry.Insert(timeout_id, timeout_ref_t(timeout_value, ins_iter));
}

void
TimeoutMonitor::unregisterTimeoutId(timeout_id_t timeout_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_timeoutRefsRegistryLock);
  _timeoutRefsRgistry.Delete(timeout_id);
}

bool
TimeoutMonitor::cancel(timeout_id_t timeout_id)
{
  timeout_ref_t timeoutRef;
  {
    smsc::core::synchronization::MutexGuard synchronize(_timeoutRefsRegistryLock);
    if ( !_timeoutRefsRgistry.Get(timeout_id, timeoutRef) )
      return false;
    _timeoutRefsRgistry.Delete(timeout_id);
  }

  TimeoutsEntryInfo& timeoutEntryInf = _knownTimeouts[timeoutRef.timeoutValue];
  smsc::core::synchronization::MutexGuard synchronize(timeoutEntryInf.entryLock);
  if ( timeoutEntryInf.activeTimeouts ) {
    delete (*timeoutRef.iterToActiveTimeout).timeoutHandler;
    timeoutEntryInf.activeTimeouts->erase(timeoutRef.iterToActiveTimeout);
    if ( timeoutEntryInf.activeTimeouts->empty() ) {
      delete timeoutEntryInf.activeTimeouts; timeoutEntryInf.activeTimeouts = NULL;
      smsc::core::synchronization::MutexGuard anotherSynchronize(_usedTimeoutsLock);
      _usedTimeouts.erase(timeoutEntryInf.iterToUsedTimeouts);
    }
  }
  return true;
}

void
TimeoutMonitor::restart(unsigned int timeout_value, timeout_id_t timeout_id)
{
  // TODO:: implement it!
}

TimeoutMonitor::timeout_id_t
TimeoutMonitor::generateTimeoutId()
{
  return _timeoutIdSourceGenerator++;
}

}}

