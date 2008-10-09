#include "BannerlessOutputMessageProcessorsDispatcher.hpp"

namespace smsc {
namespace mcisme {

BannerlessOutputMessageProcessorsDispatcher::BannerlessOutputMessageProcessorsDispatcher(TaskProcessor& taskProcessor)
  : _taskProcessor(taskProcessor) {}

void
BannerlessOutputMessageProcessorsDispatcher::dispatch(const AbntAddr& abnt)
{
  _taskProcessor.ProcessAbntEvents(abnt);
}

}}
