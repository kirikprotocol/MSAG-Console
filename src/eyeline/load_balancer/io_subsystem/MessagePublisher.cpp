#include "MessagePublisher.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

MessagePublisher::MessagePublisher(unsigned max_events_queue_sz)
  :_eventsQueue(max_events_queue_sz)
{}

void
MessagePublisher::publish(IOEvent* new_io_event)
{
  putEventToQueue(new_io_event);
}

IOEvent*
MessagePublisher::getIOEvent()
{
  return getEventFromQueue();
}

void
MessagePublisher::putEventToQueue(IOEvent* new_event)
{
  smsc::core::synchronization::MutexGuard synchronize(_eventsMonitor);
  _eventsQueue.enqueue(new_event);
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
