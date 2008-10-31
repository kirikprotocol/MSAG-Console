#include <core/synchronization/MutexGuard.hpp>
#include <util/Exception.hpp>
#include "OutputMessageProcessor.hpp"
#include "BannerOutputMessageProcessorsDispatcher.hpp"

namespace smsc {
namespace mcisme {

BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor,
                                                                                 util::config::ConfigView* advertCfg)
  : _logger(logger::Logger::getInstance("outputprc"))
{
  if (!advertCfg->getBool("useAdvert"))
    throw smsc::util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: Fatal! Advertising.useAdvert = false");

  try {
    _proxyCount = advertCfg->getInt("proxyCount");
    if ( _proxyCount == 0 )
      throw smsc::util::Exception("BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: Fatal! Advertising.proxyCount = 0");
  } catch (util::config::ConfigException& ex) {
    _proxyCount = 1;
  }

  for(unsigned i=0; i < _proxyCount; ++i) {
    try {
      OutputMessageProcessor* outputMsgProc = new OutputMessageProcessor(taskProcessor, advertCfg, *this);
      outputMsgProc->Start();
      _freeMsgProcessors.insert(outputMsgProc);
    } catch(util::Exception& ex) {
      smsc_log_error(_logger, "BannerOutputMessageProcessorsDispatcher::BannerOutputMessageProcessorsDispatcher::: catched exception [%s]", ex.what());
    }
  }
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
  if ( _freeMsgProcessors.empty() ) {
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
}

}}
