#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGR_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGR_HPP__

# include <netinet/in.h>
# include <map>
# include <string>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "eyeline/corex/io/network/TCPServerSocket.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/io_subsystem/IOParameters.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchCircuitController.hpp"
# include "eyeline/load_balancer/io_subsystem/InputEventProcessorsPool.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessorMgr : public smsc::core::threads::Thread {
public:
  IOProcessorMgr(unsigned io_proc_mgr_id,
                 const std::string& listening_host,
                 in_port_t listening_port,
                 SwitchCircuitController* switch_circuit_controller,
                 unsigned max_newconn_event_handlers,
                 unsigned max_new_conn_events_queue_sz);
  virtual ~IOProcessorMgr();

  virtual int Execute();
  virtual unsigned getId() const { return _ioProcMgrId; }

  void startup();
  void shutdown();
  void setParameters(const IOParameters& ioParameters);
  IOProcessor* getVacantIOProcessor();
protected:
  bool checkIfNeedCreateIOProcessor() const;

  IOProcessor* registerIOProcessor(IOProcessor* io_processor);
  virtual IOProcessor* createNewIOProcessor() = 0;
  const IOParameters& getParameters() const;

  unsigned _ioProcId;
  SwitchCircuitController* _switchCircuitCtrl;
  smsc::logger::Logger* _logger;
private:
  unsigned _ioProcMgrId;
  bool _shutdownInProgress;

  corex::io::network::TCPServerSocket* _listeningIface;
  IOProcessor* _currentVacantIOProcessor;
  MessagePublisher _newConnEventsPublisher;
  InputEventProcessorsPool _newConnEventsProcessor;
  typedef std::map<unsigned, IOProcessor*> registered_ioprocs_t;
  registered_ioprocs_t _ioProcessors;

  IOParameters _ioParameters;
  bool _ioParametersWasSet;
};

}}}

#endif
