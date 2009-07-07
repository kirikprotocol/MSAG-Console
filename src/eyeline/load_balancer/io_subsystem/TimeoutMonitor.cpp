#include <time.h>
#include <memory>

#include "TimeoutMonitor.hpp"
#include "core/synchronization/MutexGuard.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

int
TimeoutMonitor::Execute()
{
  smsc_log_debug(_logger, "TimeoutMonitor::Execute::: thread is running");

  while (!_shutdownInProgress) {
    int sleepPeriod=0;
    try {
    while ( true ) {
      std::auto_ptr<TimeoutEvent> eventToFire;
      {
        smsc::core::synchronization::MutexGuard synchronize(_lock);
        if ( _timeoutExpirationTimes.empty() )
          break;

        timeouts_sched_time_t::iterator iter = _timeoutExpirationTimes.begin();
        time_t currentTime = time(0);
        if ( (*iter).timeToExpire > currentTime ) {
          sleepPeriod = static_cast<int>(((*iter).timeToExpire - currentTime) * 1000);
          break;
        }
        smsc_log_debug(_logger, "TimeoutMonitor::Execute::: timeout with id='%s' has been expired",
                       (*iter).timeoutId.c_str());
        eventToFire.reset((*iter).eventToFire);
        _registeredTimeouts.erase((*iter).timeoutId);
        _timeoutExpirationTimes.erase(iter);
      }
      eventToFire->handle();
    }
    if ( sleepPeriod )
      _newTimeoutEvent.Wait(sleepPeriod);
    else
      _newTimeoutEvent.Wait();
    } catch (std::exception& ex) {
      smsc_log_debug(_logger, "TimeoutMonitor::Execute::: caught unexpected exception [%s]",
                     ex.what());
    }
  }
  return 0;
}

void
TimeoutMonitor::scheduleTimeout(timeout_id_t timeout_id,
                                unsigned int timeout_value,
                                TimeoutEvent* timeout_event)
{
  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    if ( _registeredTimeouts.find(timeout_id) != _registeredTimeouts.end() )
      throw smsc::util::Exception("TimeoutMonitor::scheduleTimeout::: timeout with id=[%s] already registered",
                                  timeout_id.c_str());

    time_t timeToExpire = time(0) + timeout_value;
    char strBuf[128];
    smsc_log_debug(_logger, "TimeoutMonitor::scheduleTimeout::: try schedule timeout event on '%s', tineout_id='%s'",
                   ctime_r(&timeToExpire, strBuf), timeout_id.c_str());

    std::pair<timeouts_sched_time_t::iterator, bool> ins_res =
      _timeoutExpirationTimes.insert(TimeoutEntry(timeToExpire, timeout_event, timeout_id));
    if ( !ins_res.second )
      throw smsc::util::Exception("TimeoutMonitor::scheduleTimeout::: can't schedule timeout with id=[%s]",
                                  timeout_id.c_str());

    try {
      _registeredTimeouts.insert(std::make_pair(timeout_id, ins_res.first));
    } catch (...) {
      _timeoutExpirationTimes.erase(ins_res.first);
      throw;
    }

    smsc_log_debug(_logger, "TimeoutMonitor::scheduleTimeout::: timeout event with tineout_id='%s' has been scheduled",
                   timeout_id.c_str());
  } catch (...) {
    delete timeout_event;
    throw;
  }
  _newTimeoutEvent.Signal();
}

void
TimeoutMonitor::cancelTimeout(timeout_id_t timeout_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  smsc_log_debug(_logger, "TimeoutMonitor::cancelTimeout::: try cancel timeout event with tineout_id='%s'",
                 timeout_id.c_str());

  registered_timeouts_t::iterator iter = _registeredTimeouts.find(timeout_id);
  if ( iter == _registeredTimeouts.end() )
    return;

  try {
    delete iter->second->eventToFire;
    _timeoutExpirationTimes.erase(iter->second);
  } catch (...) {}
  _registeredTimeouts.erase(iter);
  smsc_log_debug(_logger, "TimeoutMonitor::cancelTimeout::: timeout event with tineout_id='%s' has been canceled",
                 timeout_id.c_str());
}

}}}
