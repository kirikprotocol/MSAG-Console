#include "core/synchronization/MutexGuard.hpp"
#include "Binder.hpp"
#include "Reconnector.hpp"
#include "IOProcessor.hpp"
#include "SetOfNotBindedConnections.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

Reconnector::Reconnector(IOProcessor& io_processor,
                         int reconnect_attempt_period)
  : _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _ioProcessor(io_processor), _shutdownInProgress(false),
    _reconnectAttemptPeriod(reconnect_attempt_period)
{}

int
Reconnector::Execute()
{
  smsc_log_info(_logger, "Reconnector::Execute::: thread stared");
  while (!_shutdownInProgress) {
    try {
      bool sleepInfinitiy = false;
      if ( _failedSetsInProcessing.empty() )
        sleepInfinitiy = true;
      else {
        while ( !_failedSetsInProcessing.empty() ) {
          if ( _shutdownInProgress )
            return 0;
          failed_conn_info& nextFailedConnSetInfo = _failedSetsInProcessing.front();
          smsc_log_info(_logger, "Reconnector::Execute:::process next group of links for linkset with id='%s', scheduledProcessingTime=%lu, current time=%lu",
                        nextFailedConnSetInfo.failedConns->getLinkSetId().toString().c_str(),
                        nextFailedConnSetInfo.scheduledProcessingTime, time(NULL));
          std::set<LinkId>::iterator iter =
            _canceledLinksets.find(nextFailedConnSetInfo.failedConns->getLinkSetId());

          if ( iter != _canceledLinksets.end() ) {
            delete nextFailedConnSetInfo.failedConns;
            _failedSetsInProcessing.pop_front();
            _canceledLinksets.erase(iter);
          } else {
            if ( nextFailedConnSetInfo.scheduledProcessingTime > time(NULL) )
              break;

            tryReconnect(nextFailedConnSetInfo);
          }
        }
      }

      waitForPeriodAndGetNewFailedConnections(sleepInfinitiy);
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "Reconnector::Execute::: caught exception [%s]", ex.what());
    }
  }
  return 0;
}

void
Reconnector::tryReconnect(failed_conn_info& next_failed_connset_info)
{
  SetOfNotBindedConnections* establishedAndNotBindedConnections =
    next_failed_connset_info.failedConns->reestablishConnections();

  if ( establishedAndNotBindedConnections ) {
    _ioProcessor.getBinder().addSetOfNotBindedConnections(establishedAndNotBindedConnections);
    _ioProcessor.getBinder().rebind(establishedAndNotBindedConnections->getLinkSetId());
  }

  if ( next_failed_connset_info.failedConns->isEmpty() ) {
    delete next_failed_connset_info.failedConns;
    _failedSetsInProcessing.pop_front();
  } else {
    next_failed_connset_info.scheduledProcessingTime = time(0) + _reconnectAttemptPeriod;
    _failedSetsInProcessing.splice(_failedSetsInProcessing.end(),
                                   _failedSetsInProcessing,
                                   _failedSetsInProcessing.begin());
  }
}

void
Reconnector::addFailedConnections(SetOfFailedConnections* failed_conns_set)
{
  smsc::core::synchronization::MutexGuard synchronize(_scheduledSetsLock);
  _scheduledSets.push_back(failed_conn_info(failed_conns_set));
  _scheduledSetsLock.notify();
}

void
Reconnector::cancelReconnectionAttempts(const LinkId& link_set_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_scheduledSetsLock);
  _scheduledCanceledLinksets.insert(link_set_id);
}

void
Reconnector::waitForPeriodAndGetNewFailedConnections(bool sleep_infinitiy)
{
  smsc::core::synchronization::MutexGuard synchronize(_scheduledSetsLock);
  if ( sleep_infinitiy )
    _scheduledSetsLock.wait();
  else
    _scheduledSetsLock.wait(_reconnectAttemptPeriod * 1000);

  if ( !_scheduledSets.empty() )
    _failedSetsInProcessing.splice(_failedSetsInProcessing.end(), _scheduledSets);

  if ( !_scheduledCanceledLinksets.empty() ) {
    if ( !_failedSetsInProcessing.empty() )
      _canceledLinksets.insert(_scheduledCanceledLinksets.begin(), _scheduledCanceledLinksets.end());
    _scheduledCanceledLinksets.clear();
  }
}

void
Reconnector::shutdown()
{
  _shutdownInProgress = true;
  smsc::core::synchronization::MutexGuard synchronize(_scheduledSetsLock);
  // to wake up Reconnector's thread if it is sleeping on scheduled_sets_lock Event monitor
  _scheduledSetsLock.notify();
}

}}}
