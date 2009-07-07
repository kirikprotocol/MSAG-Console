#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_TIMEOUTEVENT_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_TIMEOUTEVENT_HPP__

# include "logger/Logger.h"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/IOEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class TimeoutEvent : public IOEvent {
public:
  TimeoutEvent(IOProcessor& io_processor, const LinkId& link_id)
  : _ioProcessor(io_processor), _linkId(link_id),
    _logger(smsc::logger::Logger::getInstance("io_subsystem"))
  {}

  virtual void handle() = 0;
protected:
  IOProcessor& _ioProcessor;
  LinkId _linkId;
  smsc::logger::Logger* _logger;
};

}}}

#endif
