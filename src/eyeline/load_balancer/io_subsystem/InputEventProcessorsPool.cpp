#include "InputEventProcessor.hpp"
#include "InputEventProcessorsPool.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
InputEventProcessorsPool::startup()
{
  smsc_log_info(_logger, "InputEventProcessorsPool::startup::: start input event processors pool [pool name=%s]",
                _eventProcPoolName.c_str());
  setMaxThreads(_maxNumOfEventProcessors);
  for(unsigned i=0; i<_maxNumOfEventProcessors; ++i) {
    startTask(new InputEventProcessor(_messagePublisher, i));
  }
}

void
InputEventProcessorsPool::shutdown()
{
  smsc_log_info(_logger, "InputEventProcessorsPool::shutdown::: shutdown input event processors pool [pool name=%s]",
                  _eventProcPoolName.c_str());
  smsc::core::threads::ThreadPool::shutdown();
}

}}}
