#include <stdio.h>
#include <memory>
#include <exception>

#include "InputEventProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

InputEventProcessor::InputEventProcessor(MessagePublisher& message_publisher,
                                         unsigned proc_num)
  : _messagePublisher(message_publisher), _logger(smsc::logger::Logger::getInstance("ioEventProc"))
{
  snprintf(_taskName, sizeof(_taskName), "ioEventProc_%u", proc_num);
}

void InputEventProcessor::stop()
{
  ThreadedTask::stop();
  _messagePublisher.interrupt();
}

int
InputEventProcessor::Execute()
{
  while (!isStopping) {
    try {
      std::auto_ptr<IOEvent> ioEvent(_messagePublisher.getIOEvent());
      if(ioEvent.get())ioEvent->handle();
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "InputEventProcessor::Execute::: caught exception '%s'", ex.what());
    } catch (...) {
      smsc_log_error(_logger, "InputEventProcessor::Execute::: caught unexpected exception ...'");
      return 1;
    }
  }

  return 0;
}

}}}
