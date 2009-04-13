#include "BEReconnector.hpp"
#include <exception>
#include <core/synchronization/MutexGuard.hpp>

namespace smsc {
namespace mcisme {

BEReconnector::BEReconnector(int connectTimeout, int reconnectionAttemptPeriod)
  : _logger(logger::Logger::getInstance("advert")), _connectTimeout(connectTimeout),
    _isStopped(false), _reconnectionAttemptPeriodInMSec(reconnectionAttemptPeriod*1000)
{}

void
BEReconnector::waitForPeriodAndGetNewBrokenConnections(bool needSleepInfinity)
{
  core::synchronization::MutexGuard synchronize(_monitor);
  while ( _setOfUnregsteredBrokenConnections.empty() ) {
    if ( needSleepInfinity )
      _monitor.wait();
    else {
      _monitor.wait(_reconnectionAttemptPeriodInMSec);
      break;
    }
  }
  if ( !_setOfUnregsteredBrokenConnections.empty() )
    _setOfBrokenConnections.splice(_setOfBrokenConnections.end(), _setOfUnregsteredBrokenConnections);
}

int
BEReconnector::Execute()
{
  smsc_log_info(_logger, "BEReconnector has been run");
  while (!_isStopped) {
    try {
      for (connections_set_t::iterator iter = _setOfBrokenConnections.begin(), end_iter = _setOfBrokenConnections.end();
           iter != end_iter;) {
        core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& brokenConnectionToBE = *iter;
        if ( brokenConnectionToBE->reinit(_connectTimeout) ) {
          smsc_log_info(_logger, "BEReconnector::Execute::: connection  to [%s] established successful", brokenConnectionToBE->toString().c_str());
          _setOfBrokenConnections.erase(iter++);
        } else
          iter++;
      }

      bool needSleepInfinity = true;
      if ( !_setOfBrokenConnections.empty() )
        needSleepInfinity = false;

      waitForPeriodAndGetNewBrokenConnections(needSleepInfinity);
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "BEReconnector::Execute::: catched unexpected exception [%s]. Thread terminated.", ex.what());
    }
  }
  smsc_log_info(_logger, "BEReconnector has finished");
  return 0;
}

void
BEReconnector::stop()
{
  _isStopped = true;
}

void
BEReconnector::scheduleBrokenConnectionToReestablishing(core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& brokenConnectionToBE)
{
  core::synchronization::MutexGuard synchronize(_monitor);
  _setOfUnregsteredBrokenConnections.push_back(brokenConnectionToBE);
  _monitor.notify();
}

}}
