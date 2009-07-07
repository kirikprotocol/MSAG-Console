#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_INPUTEVENTPROCESSORSPOOL_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_INPUTEVENTPROCESSORSPOOL_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadPool.hpp"
# include "eyeline/load_balancer/io_subsystem/MessagePublisher.hpp"
# include "eyeline/load_balancer/io_subsystem/InputEventProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class InputEventProcessorsPool : public smsc::core::threads::ThreadPool {
public:
  InputEventProcessorsPool(const std::string& event_proc_pool_name,
                           unsigned max_num_of_event_processors,
                           MessagePublisher& message_publisher)
  :  _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _eventProcPoolName(event_proc_pool_name),
    _maxNumOfEventProcessors(max_num_of_event_processors),
    _messagePublisher(message_publisher)
  {}

  void startup();
  void shutdown();
private:
  smsc::logger::Logger* _logger;
  std::string _eventProcPoolName;
  unsigned _maxNumOfEventProcessors;
  MessagePublisher& _messagePublisher;
};

}}}

#endif
