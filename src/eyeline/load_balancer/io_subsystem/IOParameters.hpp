#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPARAMETERS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPARAMETERS_HPP__

# include <string>

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

struct IOParameters {
  IOParameters()
  : maxEventsQueueSz(0), reconnectAttemptPeriod(0),
    maxOutPacketsQueueSz(0), maxNumOfEventProcessors(0),
    maxSocketsPerIOprocessor(0)
  {}

  IOParameters(unsigned max_events_queue_sz,
               unsigned reconnect_attempt_period,
               unsigned max_out_packets_queue_sz,
               unsigned max_num_of_event_proceccors,
               unsigned max_sockets_per_ioprocessor)
  : maxEventsQueueSz(max_events_queue_sz),
    reconnectAttemptPeriod(reconnect_attempt_period),
    maxOutPacketsQueueSz(max_out_packets_queue_sz),
    maxNumOfEventProcessors(max_num_of_event_proceccors),
    maxSocketsPerIOprocessor(max_sockets_per_ioprocessor)
  {}

  unsigned maxEventsQueueSz;
  unsigned reconnectAttemptPeriod;
  unsigned maxOutPacketsQueueSz;
  unsigned maxNumOfEventProcessors;
  unsigned maxSocketsPerIOprocessor;

  std::string toString() const {
    char strBuf[256];
    snprintf(strBuf, sizeof(strBuf), "maxEventsQueueSz=%d,reconnectAttemptPeriod=%d,maxOutPacketsQueueSz=%d,maxNumOfEventProcessors=%d,maxSocketsPerIOprocessor=%d",
             maxEventsQueueSz,reconnectAttemptPeriod,maxOutPacketsQueueSz,maxNumOfEventProcessors,maxSocketsPerIOprocessor);
    return strBuf;
  }
};

}}}

#endif
