#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEPUBLISHER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_MESSAGEPUBLISHER_HPP__

# include <sys/types.h>

# include "core/synchronization/EventMonitor.hpp"
# include "eyeline/utilx/FakeMutex.hpp"
# include "eyeline/utilx/ArrayBasedQueue.hpp"
# include "eyeline/load_balancer/io_subsystem/IOEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class MessagePublisher {
public:
  MessagePublisher(unsigned maxEventsQueueSz);

  void publish(IOEvent* newIoEvent);
  IOEvent* getIOEvent();
protected:
  void putEventToQueue(IOEvent* newEvent);
  IOEvent* getEventFromQueue();
private:
  smsc::core::synchronization::EventMonitor _eventsMonitor;
  utilx::ArrayBasedQueue<IOEvent*, utilx::FakeMutex> _eventsQueue;
};

}}}

#endif
