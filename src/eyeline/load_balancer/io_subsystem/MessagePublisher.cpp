#include "MessagePublisher.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

MessagePublisher::MessagePublisher(unsigned maxEventsQueueSz)
  :_eventsQueue(maxEventsQueueSz)
{}

void
MessagePublisher::publish(IOEvent* newIoEvent)
{
  putEventToQueue(newIoEvent);
}

IOEvent*
MessagePublisher::getIOEvent()
{
  return getEventFromQueue();
}

void
MessagePublisher::putEventToQueue(IOEvent* newEvent)
{
  smsc::core::synchronization::MutexGuard synchronize(_eventsMonitor);
  _eventsQueue.enqueue(newEvent);
  _eventsMonitor.notify();
}

IOEvent*
MessagePublisher::getEventFromQueue()
{
  smsc::core::synchronization::MutexGuard synchronize(_eventsMonitor);
  while ( _eventsQueue.isEmpty() )
    _eventsMonitor.wait();
  return _eventsQueue.dequeue();
}

}}}
