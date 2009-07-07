#include <stdio.h>
#include <memory>
#include <exception>

#include "InputEventProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

InputEventProcessor::InputEventProcessor(MessagePublisher& messagePublisher, unsigned procNum)
  : _messagePublisher(messagePublisher), _logger(smsc::logger::Logger::getInstance("ioEventProc"))
{
  snprintf(_taskName, sizeof(_taskName), "ioEventProc_%u", procNum);
}

int
InputEventProcessor::Execute()
{
  while (!isStopping) {
    try {
      std::auto_ptr<IOEvent> ioEvent(_messagePublisher.getIOEvent());
      ioEvent->handle();
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
