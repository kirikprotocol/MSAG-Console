#include <memory>
#include "BannerlessOutputMessageProcessorsDispatcher.hpp"

namespace smsc {
namespace mcisme {

BannerlessOutputMessageProcessorsDispatcher::BannerlessOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor)
  : _taskProcessor(taskProcessor),
    _logger(logger::Logger::getInstance("bannless"))  {}

void
BannerlessOutputMessageProcessorsDispatcher::dispatchSendMissedCallNotification(const AbntAddr& abnt)
{
  _taskProcessor.ProcessAbntEvents(abnt);
}

void
BannerlessOutputMessageProcessorsDispatcher::dispatchSendAbntOnlineNotifications(const sms_info* pInfo, const AbonentProfile& abntProfile)
{
  std::auto_ptr<const sms_info> autoPtrSmsInfo(pInfo);
  try {
    _taskProcessor.SendAbntOnlineNotifications(autoPtrSmsInfo.get(), abntProfile);
    _taskProcessor.commitMissedCallEvents(autoPtrSmsInfo->abnt, autoPtrSmsInfo->events,
                                          abntProfile);
  } catch(std::exception& ex) {
    smsc_log_error(_logger, "BannerlessOutputMessageProcessorsDispatcher::dispatchSendAbntOnlineNotifications::: caught exception '%s'",
                   ex.what());
  }
}

}}
