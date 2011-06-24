#ifndef __SMSC_MCISME_ADVERT_BANNERREADER_HPP__
# define __SMSC_MCISME_ADVERT_BANNERREADER_HPP__

# include <unistd.h>
# include <map>

# include "core/threads/Thread.hpp"
# include "core/synchronization/Mutex.hpp"

# include "mcisme/OutputMessageProcessor.hpp"
# include "mcisme/advert/BannerResponseListener.hpp"
# include "mcisme/advert/AdvertisingImpl.h"
# include "mcisme/advert/BEReconnector.hpp"

# include "logger/Logger.h"
# include "util/Exception.hpp"

namespace smsc {
namespace mcisme {

class BannerReader : public core::threads::Thread,
                     public RestoredConnectionNotifierIface {
public:
  explicit BannerReader(BannerResponseListener& banner_listener,
                        BEReconnector& reconnector,
                        unsigned response_timeout_in_msecs)
  : _isRunning(false), _bannerResponseListener(banner_listener),
    _reconnector(reconnector), _responseTimeoutInMsecs(response_timeout_in_msecs),
    _logger(logger::Logger::getInstance("ban_reader"))
  {
    int fds[2];
    if (pipe(fds))
      throw util::SystemError("BannerReader::BannerReader::: pipe() call failed");
    _readSignalEnd = fds[0]; _writeSignalEnd = fds[1];
  }

  virtual void connectionRestored(const AdvertImplRefPtr& restored_conn_to_be) {
    addClientConnect(restored_conn_to_be);
    uint8_t signalledByte=0;
    if (write(_writeSignalEnd, &signalledByte, sizeof(signalledByte)) < 0)
      throw util::SystemError("connectionRestored::: failed write to signalled pipe end");
  }
  void addClientConnect(const AdvertImplRefPtr& banner_engine_proxy);

  virtual int Execute();
  void stop();

private:
  BannerReader(const BannerReader&);
  BannerReader& operator=(const BannerReader&);

  void handleTimedOutRequests(unsigned req_exp_period);
  void handleResponses(unsigned num_of_ready_fds, unsigned count, struct pollfd* fds);
  void readSignalledInfo();

  bool _isRunning;
  BannerResponseListener& _bannerResponseListener;
  BEReconnector& _reconnector;
  typedef std::map<int /* fd number */, AdvertImplRefPtr> activeBEConns_t;
  activeBEConns_t _activeBEConns;
  unsigned _responseTimeoutInMsecs;
  logger::Logger* _logger;
  core::synchronization::Mutex _lock;
  int _readSignalEnd, _writeSignalEnd;
};

}}

#endif
