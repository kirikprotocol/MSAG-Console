#include "Reconnector.hpp"
#include "eyeline/ss7na/common/sig/SIGConnect.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

void
Reconnector::schedule(const LinkPtr& broken_link)
{
  smsc_log_debug(_logger, "Reconnector::schedule::: broken_link='%s'", broken_link->getLinkId().getValue().c_str());
  if ( _connMgr->existLink(broken_link->getLinkId()) )
    return;
  {
    smsc::core::synchronization::MutexGuard synchronize(_alreadyScheduledLinkIdsLock);
    if ( _idsForAlreadyScheduledLinks.find(broken_link->getLinkId()) !=
         _idsForAlreadyScheduledLinks.end() )
      return;
    _idsForAlreadyScheduledLinks.insert(broken_link->getLinkId());
  }
  try {
    smsc::core::synchronization::MutexGuard synchronize(_scheduledLinksLock);
    _scheduledLinks.push_back(broken_link_info(broken_link));
    _scheduledLinksLock.notify();
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "Reconnector::schedule::: caught exception '%s'", ex.what());
    smsc::core::synchronization::MutexGuard synchronize(_alreadyScheduledLinkIdsLock);
    _idsForAlreadyScheduledLinks.erase(broken_link->getLinkId());
    throw;
  }
}

int
Reconnector::Execute()
{
  smsc_log_debug(_logger, "Reconnector::Execute::: thread stared, _connMgr=0x%p, _reconnectAttemptPeriod=%u",
                 _connMgr, _reconnectAttemptPeriod);
  while (!_shutdownInProgress) {
    try {
      bool sleepInfinitiy = false;
      if ( !_notReconnectedLinks.empty() )
        _brokenLinksInProcessing.splice(_brokenLinksInProcessing.end(),
                                        _notReconnectedLinks, _notReconnectedLinks.begin());

      if ( _brokenLinksInProcessing.empty() )
        sleepInfinitiy = true;
      else {
        while ( !_brokenLinksInProcessing.empty() ) {
          if ( _shutdownInProgress )
            return 0;

          broken_link_info nextBrokenLinkInfo = _brokenLinksInProcessing.front();
          _brokenLinksInProcessing.pop_front();
          smsc_log_debug(_logger, "Reconnector::Execute:::process next broken link with id='%s', scheduledProcessingTime=%lu, current time=%lu",
                         nextBrokenLinkInfo.brokenLink->getLinkId().getValue().c_str(),
                         nextBrokenLinkInfo.scheduledProcessingTime, time(NULL));
          if ( nextBrokenLinkInfo.scheduledProcessingTime > time(NULL) )
            break;

          tryReconnect(nextBrokenLinkInfo);
        }
      }

      waitForPeriodAndGetNewBrokenLinks(sleepInfinitiy);
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "Reconnector::Execute::: caught exception [%s]",
                     ex.what());
    }
  }
  return 0;
}

void
Reconnector::tryReconnect(broken_link_info& broken_link_info)
{
  const LinkPtr& brokenLink = broken_link_info.brokenLink;
  sig::SIGConnect* sigLink = static_cast<sig::SIGConnect*>(brokenLink.Get());
  try {
    sigLink->reinit();
    sigLink->sctpEstablish();

    _connMgr->addLink(brokenLink->getLinkId(), brokenLink);
    sigLink->up();

    smsc::core::synchronization::MutexGuard synchronize(_alreadyScheduledLinkIdsLock);
    _idsForAlreadyScheduledLinks.erase(brokenLink->getLinkId());

  } catch(corex::io::ConnectionFailedException& ex) {
    smsc_log_error(_logger, "Reconnector::tryReconnect::: caught exception [%s] when processing link with id='%s'",
                   sigLink->getLinkId().getValue().c_str());
    moveFailedLinkToNotReconnectedList(broken_link_info);
  }
}


void
Reconnector::waitForPeriodAndGetNewBrokenLinks(bool sleep_infinitiy)
{
  smsc::core::synchronization::MutexGuard synchronize(_scheduledLinksLock);
  if ( sleep_infinitiy )
    _scheduledLinksLock.wait();
  else
    _scheduledLinksLock.wait(_reconnectAttemptPeriod * 1000);

  if ( !_scheduledLinks.empty() )
    _brokenLinksInProcessing.splice(_brokenLinksInProcessing.end(), _scheduledLinks);
}

void
Reconnector::shutdown()
{
  _shutdownInProgress = true;
  smsc::core::synchronization::MutexGuard synchronize(_scheduledLinksLock);
  // to wake up Reconnector's thread if it is sleeping on Event monitor
  _scheduledLinksLock.notify();
}

void
Reconnector::moveFailedLinkToNotReconnectedList(const broken_link_info& failed_link_info)
{
  _notReconnectedLinks.push_back(failed_link_info);
}

}}}}
