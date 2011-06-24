#ifndef __SMSC_MCISME_ADVERT_SENDBANNERIDREGISTRY_HPP__
# define __SMSC_MCISME_ADVERT_SENDBANNERIDREGISTRY_HPP__

# include <sys/time.h>
# include <list>
# include "util/Singleton.hpp"
# include "util/Exception.hpp"
# include "core/buffers/IntHash.hpp"
# include "core/synchronization/Mutex.hpp"
# include "logger/Logger.h"

namespace smsc {
namespace mcisme {

struct ExpiredBannerRequest {
  BannerRequest* origBannerReq;
  uint32_t trnId;
};

class SendBannerIdRegistry : public util::Singleton<SendBannerIdRegistry>
{
public:
  void saveSentBannerInfo(BannerRequest* ban_req)
  {
    smsc_log_debug(_logger, "SendBannerIdRegistry::saveSentBannerInfo: insert BannerRequest=0x%p for id=%d", ban_req, ban_req->getId());
    core::synchronization::MutexGuard synchronize(_lock);
    _knownBannerRequests.Insert(ban_req->getId(), ban_req);
    struct timeval cur_time;
    if (gettimeofday(&cur_time, NULL) < 0)
      throw util::SystemError("SendBannerIdRegistry::saveSentBannerInfo::: call gettimeofday() failed");
    _reqSendTimes.push_back(req_time_info(cur_time.tv_sec * 1000 + cur_time.tv_usec / 1000,
                                          ban_req->getId()));
  }

  BannerRequest* ackReceivedBanner(uint32_t transaction_id)
  {
    core::synchronization::MutexGuard synchronize(_lock);
    return removeRequest(transaction_id);
  }

  // return false if there isn't expired request
  bool getNextExpiredRequest(unsigned expiration_period_inmsecs, ExpiredBannerRequest* expired_ban_req)
  {
    core::synchronization::MutexGuard synchronize(_lock);
    time_t currentTime = time(NULL);
    struct timeval cur_time;
    if (gettimeofday(&cur_time, NULL) < 0)
      throw util::SystemError("SendBannerIdRegistry::getNextExpiredRequest::: call gettimeofday() failed");
    unsigned long long curTimeInMsecs = cur_time.tv_sec * 1000 + cur_time.tv_usec / 1000;
    while (!_reqSendTimes.empty()) {
      time_registry_t::iterator iter = _reqSendTimes.begin();
      if((*iter).reqTimeInMsecs + expiration_period_inmsecs <= currentTime) {
        BannerRequest* bannerReq = removeRequest((*iter).trnId);
        if (bannerReq) {
          smsc_log_debug(_logger, "SendBannerIdRegistry::getNextExpiredRequest: found expired request with id=%d", (*iter).trnId);
          expired_ban_req->origBannerReq = bannerReq;
          expired_ban_req->trnId = (*iter).trnId;
          return true;
        } else
          _reqSendTimes.erase(iter);
      } else
        return false;
    }
    return false;
  }

  BannerRequest* removeRequest(uint32_t transaction_id)
  {
    try {
      BannerRequest* ret = _knownBannerRequests.Get(transaction_id);
      smsc_log_debug(_logger, "SendBannerIdRegistry::removeRequest: returned BannerRequest=0x%p for id=%d", ret, transaction_id);
      _knownBannerRequests.Delete(transaction_id);
      return ret;
    } catch (std::runtime_error& ex) {
      return NULL;
    }
  }

protected:
  SendBannerIdRegistry()
  : _logger(logger::Logger::getInstance("advert"))
  {}
  friend class util::Singleton<SendBannerIdRegistry>;

private:
  logger::Logger* _logger;
  core::synchronization::Mutex _lock;
  core::buffers::IntHash<BannerRequest*> _knownBannerRequests;
  struct req_time_info {
    req_time_info(unsigned long long req_time_in_msecs, uint32_t trn_id)
    : reqTimeInMsecs(req_time_in_msecs), trnId(trn_id)
    {}
    unsigned long long reqTimeInMsecs;
    uint32_t trnId;
  };
  typedef std::list<req_time_info> time_registry_t;
  time_registry_t _reqSendTimes;
};

}}

#endif /* SENDBANNERIDREGISTRY_HPP_ */
