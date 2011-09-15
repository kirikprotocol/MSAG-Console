#include <stdio.h>
#include <poll.h>
#include <sys/errno.h>
#include <assert.h>
#include "BannerReader.hpp"
#include "SendBannerIdRegistry.hpp"
#include "mcisme/Exceptions.hpp"

namespace smsc {
namespace mcisme {

void
BannerReader::addClientConnect(const AdvertImplRefPtr& banner_engine_proxy)
{
  core::synchronization::MutexGuard synchronize(_lock);
  smsc_log_debug(_logger, "BannerReader::addClientConnect::: fd=%d", banner_engine_proxy->getSocketFd());
  std::pair<activeBEConns_t::iterator, bool> ins_res=_activeBEConns.insert(std::make_pair(banner_engine_proxy->getSocketFd(),
                                                          banner_engine_proxy));
  if (ins_res.second)
    smsc_log_debug(_logger, "BannerReader::addClientConnect::: fd=%d inserted into activeBEConns map",
                   banner_engine_proxy->getSocketFd());
}

int
BannerReader::Execute()
{
  _isRunning = true;
  smsc_log_info(_logger, "BannerReader has been started");
  while (_isRunning)
  {
    struct pollfd fds[100]; // change it to some reasonable value
    int count = 0;
    _lock.Lock();
    for(activeBEConns_t::const_iterator iter=_activeBEConns.begin(), end_iter=_activeBEConns.end();
        iter != end_iter; ++iter, ++count)
    {
      fds[count].fd = iter->second->getSocketFd();
      fds[count].revents = 0;
      fds[count].events = POLLRDNORM;
    }
    _lock.Unlock();
    fds[count].fd = _readSignalEnd;
    fds[count].revents = 0;
    fds[count].events = POLLRDNORM;
    ++count;

    int st = ::poll(fds, count, _responseTimeoutInMsecs);
    if ( st < 0 ) {
      if ( errno == EINTR )
        smsc_log_error(_logger, "BannerReader::Execute::: poll() was interrupted");
      else {
        smsc_log_error(_logger, "BannerReader::Execute::: call to poll() failed: (%s, errno=%d)",
                       strerror(errno), errno);
      }
      continue;
    } else if ( !st ) {
      handleTimedOutRequests(_responseTimeoutInMsecs);
    } else {
      smsc_log_debug(_logger, "BannerReader::Execute::: %d fds are ready", st);
      handleResponses(st, count, fds);
    }
  }

  return 0;
}

void
BannerReader::handleTimedOutRequests(unsigned req_exp_period)
{
  ExpiredBannerRequest expiredBannerReq;
  while (SendBannerIdRegistry::getInstance().getNextExpiredRequest(req_exp_period, &expiredBannerReq)) {
    activeBEConns_t::iterator iter;
    try {
      core::synchronization::MutexGuard synchronize(_lock);
      iter = _activeBEConns.find(expiredBannerReq.origBannerReq->fd);
      // there might be a case when expired request exists in SendBannerIdRegistry
      // but fd is absent because connection to BE was broken
      if (iter != _activeBEConns.end())
      {
        AdvertImplRefPtr& bannerEngineProxy = iter->second;
        smsc_log_info(_logger, "BannerReader::handleTimedOutRequests::: send rollback request for trnId=%u,bannerId=%u,ownerId=%u,rotatorId=%u,serviceName='%s'",
                      expiredBannerReq.trnId, expiredBannerReq.origBannerReq->bannerId, expiredBannerReq.origBannerReq->ownerId,
                      expiredBannerReq.origBannerReq->rotatorId, expiredBannerReq.origBannerReq->serviceName.c_str());
        try {
          bannerEngineProxy->sendErrorInfo(BannerRequest(expiredBannerReq.trnId, expiredBannerReq.origBannerReq->bannerId,
                                                         expiredBannerReq.origBannerReq->ownerId, expiredBannerReq.origBannerReq->rotatorId,
                                                         expiredBannerReq.origBannerReq->serviceName),
                                                         ERR_ADV_TIMEOUT);
          BannerResponseTrace bannerRespTrace;
          bannerRespTrace.bannerId = -1;
          bannerRespTrace.transactionId = expiredBannerReq.trnId;
          bannerRespTrace.ownerId = expiredBannerReq.origBannerReq->ownerId;
          bannerRespTrace.rotatorId = expiredBannerReq.origBannerReq->rotatorId;
          bannerRespTrace.serviceName = expiredBannerReq.origBannerReq->serviceName;
          BannerInfo*  bannerInfo = new BannerInfo(bannerRespTrace, "", expiredBannerReq.origBannerReq->mcEventOut,
                                                   expiredBannerReq.origBannerReq->charSet);
          expiredBannerReq.origBannerReq->resetMCEventOut(); // in order to avoid destroying of MCEventOut object at BannerRequest's destructor.
          _bannerResponseListener.handleEvent(bannerInfo);
        } catch (NetworkException& ex) {
          smsc_log_error(_logger, "BannerReader::handleTimedOutRequests::: caught NetworkException '%s'", ex.what());
          _reconnector.scheduleBrokenConnectionToReestablishing(bannerEngineProxy);
          _activeBEConns.erase(iter);
        }
      }
      delete expiredBannerReq.origBannerReq;
    } catch(std::exception& ex) {
      smsc_log_error(_logger, "BannerReader::handleTimedOutRequests::: caught exception '%s'", ex.what());
    } catch (...) {
      smsc_log_error(_logger, "BannerReader::handleTimedOutRequests::: caught unexpected exception ...");
    }
  }
}

void
BannerReader::handleResponses(unsigned num_of_ready_fd, unsigned count, struct pollfd* fds)
{
  for(int i=0; i<count && num_of_ready_fd; ++i) {
    if (fds[i].revents & POLLRDNORM) {
      --num_of_ready_fd;
      if (fds[i].fd == _readSignalEnd)
        readSignalledInfo();
      else {
        core::synchronization::MutexGuard synchronize(_lock);
        smsc_log_debug(_logger, "BannerReader::handleResponses::: fd=%d is ready", fds[i].fd);
        activeBEConns_t::iterator iter = _activeBEConns.find(fds[i].fd);
        assert(iter != _activeBEConns.end());
        AdvertImplRefPtr& bannerEngineProxy = iter->second;
        try {
          std::string banner;
          BannerResponseTrace bannerRespTrace;

          banner_read_stat status = bannerEngineProxy->readAdvert(&banner, &bannerRespTrace);
          if (status == CONTINUE_READ_PACKET)
            continue;

          BannerRequest* ackedBannerRequest = NULL;
          ackedBannerRequest = SendBannerIdRegistry::getInstance().ackReceivedBanner(bannerRespTrace.transactionId);

          if (!ackedBannerRequest) {
            // If we got response for unknown banner request then rollback received banner.
            // bannerId == -1 if we received banner response packet that doesn't include banner.
            // Skip such response w/o sending of rollback message.
            if (status == BANNER_OK && bannerRespTrace.bannerId > -1)
              bannerEngineProxy->rollbackBanner(bannerRespTrace.transactionId, bannerRespTrace.bannerId, bannerRespTrace.ownerId,
                                                bannerRespTrace.rotatorId, bannerRespTrace.serviceName);
          } else {
            BannerInfo*  bannerInfo = new BannerInfo(bannerRespTrace, banner, ackedBannerRequest->mcEventOut, ackedBannerRequest->charSet);
            ackedBannerRequest->resetMCEventOut(); // in order to avoid destroying of MCEventOut object at BannerRequest's destructor.
            _bannerResponseListener.handleEvent(bannerInfo);
            delete ackedBannerRequest;
          }
        } catch (NetworkException& ex) {
          smsc_log_error(_logger, "BannerReader::handleResponses::: caught NetworkException '%s'", ex.what());
          _reconnector.scheduleBrokenConnectionToReestablishing(bannerEngineProxy);
          _activeBEConns.erase(iter);
        } catch(std::exception& ex) {
          smsc_log_error(_logger, "BannerReader::handleResponses::: caught exception '%s'", ex.what());
        } catch (...) {
          smsc_log_error(_logger, "BannerReader::handleResponses::: caught unexpected exception ...");
        }
      }
    }
  }
}

void
BannerReader::stop()
{
  _isRunning=false;
  Kill(SIGTERM);
}

void
BannerReader::readSignalledInfo()
{
  uint8_t signalledBytes[64];
  smsc_log_debug(_logger, "BannerReader::readSignalledInfo::: fread signal info from fd=%d", _readSignalEnd);
  if (read(_readSignalEnd, signalledBytes, sizeof(signalledBytes)) < 0)
    smsc_log_error(_logger, "BannerReader::readSignalledInfo::: read from signalled pipe is failed: err=%s", strerror(errno));
}

}}
