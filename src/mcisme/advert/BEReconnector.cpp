#include "BEReconnector.hpp"
#include <exception>
#include <core/synchronization/MutexGuard.hpp>

namespace smsc {
namespace mcisme {

BEReconnector::BEReconnector(int connect_timeout, int reconnection_attempt_period)
  : _logger(logger::Logger::getInstance("advert")), _connectTimeout(connect_timeout),
    _isStopped(false), _reconnectionAttemptPeriodInMSec(reconnection_attempt_period*1000),
    _restoredConnNotifier(NULL)
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
  while ( !_setOfUnregsteredBrokenConnections.empty() ) {
    AdvertImplRefPtr& brokenConn = _setOfUnregsteredBrokenConnections.front();
    if ( _setOfBrokenConnections.find(brokenConn.Get()) == _setOfBrokenConnections.end() )
      _setOfBrokenConnections.insert(std::make_pair(brokenConn.Get(), brokenConn));
    _setOfUnregsteredBrokenConnections.pop_front();
  }
}

int
BEReconnector::Execute()
{
  smsc_log_info(_logger, "BEReconnector has been run");
  while (!_isStopped) {
    try {
      for (connections_set_t::iterator iter = _setOfBrokenConnections.begin(), end_iter = _setOfBrokenConnections.end();
           iter != end_iter;) {
        AdvertisingImpl* brokenConnectionToBE = iter->first;
        if ( brokenConnectionToBE->reinit(_connectTimeout) ) {
          smsc_log_info(_logger, "BEReconnector::Execute::: connection  to [%s] established successful", brokenConnectionToBE->toString().c_str());
          AdvertImplRefPtr restoredConnection=iter->second;
          _setOfBrokenConnections.erase(iter++);
          if (_restoredConnNotifier)
            _restoredConnNotifier->connectionRestored(restoredConnection);
        } else
          iter++;
      }

      bool needSleepInfinity = true;
      if ( !_setOfBrokenConnections.empty() )
        needSleepInfinity = false;

      waitForPeriodAndGetNewBrokenConnections(needSleepInfinity);
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "BEReconnector::Execute::: caught unexpected exception [%s]", ex.what());
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
BEReconnector::scheduleBrokenConnectionToReestablishing(AdvertImplRefPtr& brokenConnectionToBE)
{
  core::synchronization::MutexGuard synchronize(_monitor);
  _setOfUnregsteredBrokenConnections.push_back(brokenConnectionToBE);
  _monitor.notify();
}

}}
