#include <core/synchronization/MutexGuard.hpp>
#include <util/Exception.hpp>
#include "OutputMessageProcessor.hpp"
#include "BannerOutputMessageProcessorsDispatcher.hpp"

namespace smsc {
namespace mcisme {

BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher(TaskProcessor& task_processor,
                                                                                 util::config::ConfigView* advert_cfg)
  : _logger(logger::Logger::getInstance("outputprc"))
{
  if (!advert_cfg->getBool("useAdvert"))
    throw smsc::util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: Fatal! Advertising.useAdvert = false");

  try {
    _proxyCount = advert_cfg->getInt("proxyCount");
    if ( _proxyCount == 0 )
      throw smsc::util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: Fatal! Advertising.proxyCount = 0");
  } catch (util::config::ConfigException& ex) {
    _proxyCount = 1;
  }

  int connectTimeout;
  try {
    connectTimeout = advert_cfg->getInt("connectTimeout");
    if ( connectTimeout < 0 )
      throw util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: invalid Advertising.connectTimeout value < 0");
  } catch (...) {
    connectTimeout = 15;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.connectTimeout> missed. Default value is '15'.");
  }

  int reconnectionAttemptPeriod;
  try {
    reconnectionAttemptPeriod = advert_cfg->getInt("reconnectionAttemptPeriod");
    if ( reconnectionAttemptPeriod < 0 )
      throw util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: invalid Advertising.reconnectionAttemptPeriod value < 0");
  } catch (...) {
    reconnectionAttemptPeriod = 1;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.reconnectionAttemptPeriod> missed. Default value is '1'.");
  }

  int responseHandlersCount;
  try {
    responseHandlersCount = advert_cfg->getInt("responseHandlersCount");
    if ( responseHandlersCount < 0 )
      throw util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: invalid Advertising.responseHandlersCount value < 0");
  } catch (...) {
    responseHandlersCount = 10;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.responseHandlersCount> missed. Default value is '10'.");
  }

  int responseTimeoutInMsecs;
  try {
    responseTimeoutInMsecs = advert_cfg->getInt("timeout");
    if ( responseTimeoutInMsecs < 0 )
      throw util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: invalid Advertising.timeout value < 0");
  } catch (...) {
    responseTimeoutInMsecs = 1000;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.timeout> missed. Default value is 1000 msec");
  }

  _reconnectorThread = new BEReconnector(connectTimeout, reconnectionAttemptPeriod);
  _reconnectorThread->Start();

  _bannerListener = new BannerResponseListener(task_processor, responseHandlersCount);
  _bannerReader = new BannerReader(*_bannerListener, *_reconnectorThread, responseTimeoutInMsecs);
  _reconnectorThread ->setListener(_bannerReader);

  for(unsigned i=0; i < _proxyCount; ++i) {
    try {
      OutputMessageProcessor* outputMsgProc = new OutputMessageProcessor(task_processor, advert_cfg, *this, *_reconnectorThread, *_bannerReader);
      outputMsgProc->Start();
      _freeMsgProcessors.insert(outputMsgProc);
    } catch(util::Exception& ex) {
      smsc_log_error(_logger, "BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: caught exception [%s]", ex.what());
    }
  }
  _bannerReader->Start();
  _bannerListener->start();
}

BannerOutputMessageProcessorsDispatcher::~BannerOutputMessageProcessorsDispatcher()
{
  try {
    shutdown();
  } catch (...) {}
}

OutputMessageProcessor*
BannerOutputMessageProcessorsDispatcher::getFreeProcessor()
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  while ( _freeMsgProcessors.empty() ) {
    if (_dispatchMonitor.wait())
      throw util::SystemError("BannerOutputMessageProcessorsDispatcher::getFreeProcessor::: EventMonitor.wait() failed");
  }
  msg_processors_t::iterator iter = _freeMsgProcessors.begin();
  OutputMessageProcessor* msgProc = *iter;
  _freeMsgProcessors.erase(iter);
  _usedMsgProcessors.insert(msgProc);
  smsc_log_debug(_logger, "BannerOutputMessageProcessorsDispatcher::getFreeProcessor::: got free message processor [%p]", msgProc);

  return msgProc;
}

void
BannerOutputMessageProcessorsDispatcher::dispatchSendMissedCallNotification(const AbntAddr& abnt)
{
  OutputMessageProcessor* msgProc = getFreeProcessor();
  msgProc->assignMessageOutputWork(abnt);
}

void
BannerOutputMessageProcessorsDispatcher::dispatchSendAbntOnlineNotifications(const sms_info* pInfo,
                                                                             const AbonentProfile& abntProfile)
{
  OutputMessageProcessor* msgProc = getFreeProcessor();
  msgProc->assignMessageOutputWork(pInfo, abntProfile);
}

void
BannerOutputMessageProcessorsDispatcher::dispatchBERollbackRequest(const BannerResponseTrace& bannerRespTrace)
{
  if( bannerRespTrace.bannerId == -1 ) return;
  OutputMessageProcessor* msgProc = getFreeProcessor();
  msgProc->assignMessageOutputWork(bannerRespTrace);
}

void
BannerOutputMessageProcessorsDispatcher::markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor)
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  smsc_log_debug(_logger, "BannerOutputMessageProcessorsDispatcher::markMessageProcessorAsFree::: try mark message processor [%p] as free", freeMessageProcessor);
  msg_processors_t::iterator iter = _usedMsgProcessors.find(freeMessageProcessor);
  if ( iter != _usedMsgProcessors.end() ) {
    _usedMsgProcessors.erase(iter);
    _freeMsgProcessors.insert(freeMessageProcessor);
    smsc_log_debug(_logger, "BannerOutputMessageProcessorsDispatcher::markMessageProcessorAsFree::: message processor [%p] has been marked as free", freeMessageProcessor);
    _dispatchMonitor.notify();
  } else
    smsc_log_error(_logger, "BannerOutputMessageProcessorsDispatcher::markMessageProcessorAsFree::: message processor [%p] not found in set of used processors", freeMessageProcessor);
}

void
BannerOutputMessageProcessorsDispatcher::deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor)
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  smsc_log_debug(_logger, "BannerOutputMessageProcessorsDispatcher::deleteMessageProcessor::: delete message processor [%p]", terminatedMessageProcessor);
  msg_processors_t::iterator iter = _usedMsgProcessors.find(terminatedMessageProcessor);
  if ( iter != _usedMsgProcessors.end() )
    _usedMsgProcessors.erase(iter);
  else {
    iter = _freeMsgProcessors.find(terminatedMessageProcessor);
    if ( iter != _freeMsgProcessors.end() )
      _freeMsgProcessors.erase(iter);
  }

  delete terminatedMessageProcessor;
}

void
BannerOutputMessageProcessorsDispatcher::shutdown()
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  msg_processors_t::iterator iter;
  do {
    while ( !_freeMsgProcessors.empty() ) {
      iter = _freeMsgProcessors.begin();
      OutputMessageProcessor* outputMsgProc = *iter;
      outputMsgProc->stop();
      _freeMsgProcessors.erase(iter);
    }

    if ( !_usedMsgProcessors.empty() )
      _dispatchMonitor.wait();
    else
      break;
  } while (true);

  _reconnectorThread->stop();
  _bannerReader->stop();
  _bannerListener->stop();
}

}}
