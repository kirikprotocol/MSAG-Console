#include "IOEventProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

int
IOEventProcessor::Execute() {
  try {
    while (!isStopping) {
      IOEvent* event = _cMgr.getEvent();
      try {
        event->handle(); // message processing being made inside method handle()
      } catch (smsc::util::Exception& ex) {
        smsc_log_error(_logger, "IOEventProcessor::Execute::: caught exception [%s]", ex.what());
      }
    }
    smsc_log_info(_logger, "IOEventProcessor::Execute::: leave it");
    return 0;
  } catch (std::exception& ex) {
    smsc_log_info(_logger, "IOEventProcessor::Execute::: caught exception [%s]", ex.what());
  } catch (...) {
    smsc_log_info(_logger, "IOEventProcessor::Execute::: caught unexpected exception [...]");
  }
  return 1;
}

const char*
IOEventProcessor::taskName()
{
  return "IOEventProcessor";
}

}}}}
