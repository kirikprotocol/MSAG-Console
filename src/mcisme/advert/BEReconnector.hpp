#ifndef __MCISME_ADVERT_BERECONNECTOR_HPP__
# define __MCISME_ADVERT_BERECONNECTOR_HPP__

# include <map>
# include <list>
# include "logger/Logger.h"
# include "core/buffers/RefPtr.hpp"
# include "core/threads/Thread.hpp"
# include "core/synchronization/EventMonitor.hpp"
# include "core/synchronization/Mutex.hpp"
# include "mcisme/advert/AdvertisingImpl.h"

namespace smsc {
namespace mcisme {

class RestoredConnectionNotifierIface {
public:
  virtual ~RestoredConnectionNotifierIface() {}
  virtual void connectionRestored(const AdvertImplRefPtr& restored_conn_to_be) = 0;
};

//forward declaration to avoid cyclic dependency between BannerReader.hpp and BEReconnector.hpp
class BannerReader;

class BEReconnector : public core::threads::Thread {
public:
  BEReconnector(int connect_timeout, int reconnection_attempt_period);
  void scheduleBrokenConnectionToReestablishing(AdvertImplRefPtr& broken_connection_to_BE);
  virtual int Execute();
  void stop();
  void setListener(RestoredConnectionNotifierIface* restored_conn_notifier) {
    _restoredConnNotifier = restored_conn_notifier;
  }

private:
  void waitForPeriodAndGetNewBrokenConnections(bool need_sleep_infinity);

  logger::Logger* _logger;
  int _connectTimeout;
  bool _isStopped;
  int _reconnectionAttemptPeriodInMSec;
  typedef std::list<AdvertImplRefPtr> connections_list_t;
  typedef std::map<AdvertisingImpl*, AdvertImplRefPtr> connections_set_t;
  connections_set_t _setOfBrokenConnections;
  connections_list_t _setOfUnregsteredBrokenConnections;
  core::synchronization::EventMonitor _monitor;
  RestoredConnectionNotifierIface* _restoredConnNotifier;
};

}}

#endif
