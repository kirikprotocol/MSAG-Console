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

void
BannerOutputMessageProcessorsDispatcher::dispatch(const AbntAddr& abnt)
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  if ( _freeMsgProcessors.empty() ) {
    if (_dispatchMonitor.wait())
      throw util::SystemError("BannerOutputMessageProcessorsDispatcher::dispatch::: EventMonitor.wait() failed");
  }
  msg_processors_t::iterator iter = _freeMsgProcessors.begin();
  OutputMessageProcessor* msgProc = *iter;
  _freeMsgProcessors.erase(iter);
  _usedMsgProcessors.insert(msgProc);
  msgProc->assignMessageOutputWork(abnt);
}

void
BannerOutputMessageProcessorsDispatcher::markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor)
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  msg_processors_t::iterator iter = _usedMsgProcessors.find(freeMessageProcessor);
  if ( iter != _usedMsgProcessors.end() ) {
    _usedMsgProcessors.erase(iter);
    _freeMsgProcessors.insert(freeMessageProcessor);
    _dispatchMonitor.notify();
  }
}

void
BannerOutputMessageProcessorsDispatcher::deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor)
{
  core::synchronization::MutexGuard synchonize(_dispatchMonitor);
  msg_processors_t::iterator iter = _usedMsgProcessors.find(terminatedMessageProcessor);
  if ( iter != _usedMsgProcessors.end() )
    _usedMsgProcessors.erase(iter);
  else {
    iter = _freeMsgProcessors.find(terminatedMessageProcessor);
    if ( iter != _usedMsgProcessors.end() )
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
      //      delete outputMsgProc;
    }

    if ( !_usedMsgProcessors.empty() )
      _dispatchMonitor.wait();
    else
      break;
  } while (true);
}

}}
