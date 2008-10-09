#ifndef __SMSC_MCISME_BANNEROUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__
# define __SMSC_MCISME_BANNEROUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__

# include <set>
# include <logger/Logger.h>
# include <core/threads/Thread.hpp>
# include <core/synchronization/EventMonitor.hpp>
# include <util/config/ConfigView.h>
# include <mcisme/TaskProcessor.h>
# include <mcisme/OutputMessageProcessorsDispatcher.hpp>

namespace smsc {
namespace mcisme {

class BannerOutputMessageProcessorsDispatcher : public OutputMessageProcessorsDispatcher {
public:
  explicit BannerOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor,
                                                   util::config::ConfigView* advertCfg);
  virtual void dispatch(const AbntAddr& abnt);
  virtual void markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor);
  virtual void deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor);
  virtual void shutdown();
private:
  unsigned _proxyCount;
  typedef std::set<OutputMessageProcessor*> msg_processors_t;
  msg_processors_t _freeMsgProcessors;
  msg_processors_t _usedMsgProcessors;
  core::synchronization::EventMonitor _dispatchMonitor;
  logger::Logger* _logger;
};

}}

#endif
