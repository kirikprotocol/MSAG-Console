#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_INPUTEVENTPROCESSOR_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_INPUTEVENTPROCESSOR_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadedTask.hpp"
# include "eyeline/load_balancer/io_subsystem/MessagePublisher.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class InputEventProcessor : public smsc::core::threads::ThreadedTask {
public:
  InputEventProcessor(MessagePublisher& message_publisher, unsigned proc_num);

  virtual int Execute();
  virtual const char* taskName() { return _taskName; };
  void stop();
private:
  MessagePublisher& _messagePublisher;
  smsc::logger::Logger* _logger;
  char _taskName[32];
};

}}}

#endif
