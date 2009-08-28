#ifndef __SMSC_MCISME_BANNERLESSOUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__
# define __SMSC_MCISME_BANNERLESSOUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__

# include <core/threads/Thread.hpp>
# include <util/config/ConfigView.h>
# include <logger/Logger.h>

# include <mcisme/TaskProcessor.h>
# include <mcisme/OutputMessageProcessorsDispatcher.hpp>

namespace smsc {
namespace mcisme {

class BannerlessOutputMessageProcessorsDispatcher : public OutputMessageProcessorsDispatcher {
public:
  explicit BannerlessOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor);

  virtual void dispatchSendMissedCallNotification(const AbntAddr& abnt);
  virtual void dispatchSendAbntOnlineNotifications(const sms_info* pInfo, const AbonentProfile& abntProfile);
  virtual void dispatchBERollbackRequest(const BannerResponseTrace& bannerRespTrace) {}
  virtual void markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor) {}
  virtual void deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor) {}
private:
  TaskProcessor& _taskProcessor;
  logger::Logger* _logger;
};

}}

#endif
