#include <memory>
#include "BannerlessOutputMessageProcessorsDispatcher.hpp"

namespace smsc {
namespace mcisme {

BannerlessOutputMessageProcessorsDispatcher::BannerlessOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor)
  : _taskProcessor(taskProcessor) {}

void
BannerlessOutputMessageProcessorsDispatcher::dispatchSendMissedCallNotification(const AbntAddr& abnt)
{
  _taskProcessor.ProcessAbntEvents(abnt);
}

void
BannerlessOutputMessageProcessorsDispatcher::dispatchSendAbntOnlineNotifications(const sms_info* pInfo, const AbonentProfile& abntProfile)
{
  std::auto_ptr<const sms_info> autoPtrSmsInfo(pInfo);
  _taskProcessor.SendAbntOnlineNotifications(autoPtrSmsInfo.get(), abntProfile);
  _taskProcessor.commitMissedCallEvents(autoPtrSmsInfo.get(), abntProfile);
}

}}
