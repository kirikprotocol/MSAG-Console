#ifndef __MCISME_ADVERT_BERECONNECTOR_HPP__
# define __MCISME_ADVERT_BERECONNECTOR_HPP__

# include <list>
# include <logger/Logger.h>
# include <core/buffers/RefPtr.hpp>
# include <core/threads/Thread.hpp>
# include <core/synchronization/EventMonitor.hpp>
# include <core/synchronization/Mutex.hpp>
# include <mcisme/advert/AdvertisingImpl.h>

namespace smsc {
namespace mcisme {

class BEReconnector : public core::threads::Thread {
public:
  BEReconnector(int connectTimeout, int reconnectionAttemptPeriod);
  void scheduleBrokenConnectionToReestablishing(core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex>& brokenConnectionToBE);
  virtual int Execute();
  void stop();
private:
  void waitForPeriodAndGetNewBrokenConnections(bool needSleepInfinity);

  logger::Logger* _logger;
  int _connectTimeout;
  bool _isStopped;
  int _reconnectionAttemptPeriodInMSec;
  typedef std::list<core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex> > connections_set_t;
  connections_set_t _setOfBrokenConnections;
  connections_set_t _setOfUnregsteredBrokenConnections;
  core::synchronization::EventMonitor _monitor;
};

}}

#endif
