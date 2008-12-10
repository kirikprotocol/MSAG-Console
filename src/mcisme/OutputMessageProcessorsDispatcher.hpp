#ifndef __SMSC_MCISME_OUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__
# define __SMSC_MCISME_OUTPUTMESSAGEPROCESSORSDISPATCHER_HPP__

# include <mcisme/AbntAddr.hpp>
# include <mcisme/TaskProcessor.h>
# include <mcisme/Profiler.h>
# include <mcisme/advert/Advertising.h>

namespace smsc {
namespace mcisme {

class OutputMessageProcessor;
struct sms_info;

class OutputMessageProcessorsDispatcher {
public:
  virtual ~OutputMessageProcessorsDispatcher() {}
  virtual void dispatchSendMissedCallNotification(const AbntAddr& abnt) = 0;
  virtual void dispatchSendAbntOnlineNotifications(const sms_info* pInfo, const AbonentProfile& abntProfile) = 0;
  virtual void dispatchBERollbackRequest(const BannerResponseTrace& bannerRespTrace) = 0;

  virtual void markMessageProcessorAsFree(OutputMessageProcessor* freeMessageProcessor) = 0;
  virtual void deleteMessageProcessor(OutputMessageProcessor* terminatedMessageProcessor) = 0;
  virtual void shutdown() {}
};

}}

#endif
