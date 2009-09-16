#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGRTMPL_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGRTMPL_HPP__

# include "eyeline/load_balancer/io_subsystem/IOProcessorMgr.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

template<class IO_PROCESSOR>
class IOProcessorMgrTmpl : public IOProcessorMgr {
public:
  IOProcessorMgrTmpl(unsigned io_proc_mgr_id,
                     const std::string& listening_host,
                     in_port_t listening_port,
                     SwitchCircuitController* switch_circuit_controller,
                     unsigned max_newconn_event_handlers,
                     unsigned max_new_conn_events_queue_sz)
  : IOProcessorMgr(io_proc_mgr_id, listening_host, listening_port,
                   switch_circuit_controller, max_newconn_event_handlers,
                   max_new_conn_events_queue_sz)
  {}

protected:
  virtual IOProcessorRefPtr createNewIOProcessor() {
    const IOParameters& ioParams = getParameters();
    IOProcessor* newIOProcessor = new IO_PROCESSOR(_ioProcId++, getId(),
                                                   *_switchCircuitCtrl,
                                                   ioParams.maxEventsQueueSz,
                                                   ioParams.reconnectAttemptPeriod,
                                                   ioParams.maxOutPacketsQueueSz,
                                                   ioParams.maxNumOfEventProcessors,
                                                   ioParams.maxSocketsPerIOprocessor);
    smsc_log_debug(_logger, "IOProcessorMgrTmpl::createNewIOProcessor::: new IOProcessor was created, id of new IOProcessor=%d",
                   newIOProcessor->getId());
    newIOProcessor->startup();
    return registerIOProcessor(newIOProcessor);
  }
};

}}}

#endif
