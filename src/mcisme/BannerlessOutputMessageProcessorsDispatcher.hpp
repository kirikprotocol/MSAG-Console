#ifndef __SMSC_MCISME_BANNERLESSOUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__
# define __SMSC_MCISME_BANNERLESSOUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__

# include <core/threads/Thread.hpp>
# include <util/config/ConfigView.h>

# include <mcisme/TaskProcessor.h>
# include <mcisme/OutputMessageProcessorsDispatcher.hpp>

namespace smsc {
namespace mcisme {

class BannerlessOutputMessageProcessorsDispatcher : public OutputMessageProcessorsDispatcher {
public:
  explicit BannerlessOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor);

  virtual void dispatch(const AbntAddr& abnt);
  virtual void markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor) {}
  virtual void deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor) {}
private:
  TaskProcessor& _taskProcessor;
};

}}

#endif
