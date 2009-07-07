#include <utility>

#include "eyeline/corex/io/network/TCPServerSocket.hpp"
#include "IOProcessorMgr.hpp"
#include "AcceptNewConnectionEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

IOProcessorMgr::IOProcessorMgr(unsigned io_proc_mgr_id,
                               const std::string& listening_host,
                               in_port_t listening_port,
                               SwitchCircuitController* switch_circuit_controller,
                               unsigned max_newconn_event_handlers,
                               unsigned max_new_conn_events_queue_sz)
: _ioProcMgrId(io_proc_mgr_id), _ioProcId(0),
  _shutdownInProgress(false), _switchCircuitCtrl(switch_circuit_controller),
  _currentVacantIOProcessor(NULL),
  _listeningIface(new corex::io::network::TCPServerSocket(listening_host, listening_port)),
  _newConnEventsPublisher(max_new_conn_events_queue_sz),
  _newConnEventsProcessor("NewConnEventProcPool", max_newconn_event_handlers, _newConnEventsPublisher),
  _ioParametersWasSet(false), _logger(smsc::logger::Logger::getInstance("io_subsystem"))
{
  smsc_log_debug(_logger, "IOProcessorMgr::IOProcessorMgr::: _ioProcMgrId=%d, listen on host='%s', port=%d",
                 _ioProcMgrId, listening_host.c_str(), listening_port);
}

IOProcessorMgr::~IOProcessorMgr()
{
  delete _listeningIface;
}

int
IOProcessorMgr::Execute()
{
  smsc_log_debug(_logger, "IOProcessorMgr::Execute::: thread is running");

  while (!_shutdownInProgress) {
    try {
      corex::io::network::TCPSocket* newSocket = _listeningIface->accept();
      smsc_log_debug(_logger, "IOProcessorMgr::Execute::: accept returned new socket=[%s]",
                     newSocket->toString().c_str());
      IOProcessor* vacantIOProcessor = getVacantIOProcessor();
      const IOParameters& ioParams = getParameters();
      _newConnEventsPublisher.publish(new AcceptNewConnectionEvent(*vacantIOProcessor,
                                                                   *_switchCircuitCtrl,
                                                                   newSocket));
    } catch (std::exception& ex) {
      smsc_log_debug(_logger, "IOProcessorMgr::Execute::: caught unexpected exception=[%s]",
                     ex.what());
    }
  }

  return 0;
}

void
IOProcessorMgr::startup()
{
  _newConnEventsProcessor.startup();
  createNewIOProcessor();
  Start();
}

void
IOProcessorMgr::shutdown()
{
  smsc_log_debug(_logger, "IOProcessorMgr::shutdown::: try shutdown IOProcessorMgr with id=%d",
                 _ioProcMgrId);
  _shutdownInProgress = true;
  _listeningIface->close();
  _newConnEventsProcessor.shutdown();
  while ( !_ioProcessors.empty() ) {
    registered_ioprocs_t::iterator iter = _ioProcessors.begin();
    smsc_log_debug(_logger, "IOProcessorMgr::shutdown::: try shutdown IOProcessor with id=%d",
                     iter->second->getId());
    iter->second->shutdown();
    delete iter->second;
    _ioProcessors.erase(iter);
  }
  smsc_log_debug(_logger, "IOProcessorMgr::shutdown::: IOProcessorMgr with id=%d has been stopped",
                 _ioProcMgrId);
}

void
IOProcessorMgr::setParameters(const IOParameters& ioParameters)
{
  _ioParameters = ioParameters;
  _ioParametersWasSet = true;
}

const IOParameters&
IOProcessorMgr::getParameters() const
{
  if ( _ioParametersWasSet )
    return _ioParameters;
  else
    throw smsc::util::Exception("IOProcessorMgr::getParameters::: IO parameters wasn't set");
}

IOProcessor*
IOProcessorMgr::getVacantIOProcessor()
{
  if ( _currentVacantIOProcessor->canProcessNewSocket() ) {
    smsc_log_debug(_logger, "IOProcessorMgr::getVacantIOProcessor::: return free IOProcessor with id=%d",
                   _currentVacantIOProcessor->getId());
    return _currentVacantIOProcessor;
  } else
    return createNewIOProcessor();
}

IOProcessor*
IOProcessorMgr::registerIOProcessor(IOProcessor* io_processor)
{
  std::pair<registered_ioprocs_t::iterator, bool> res = _ioProcessors.insert(std::make_pair(io_processor->getId(), io_processor));
  if ( !res.second )
    throw smsc::util::Exception("IOProcessorMgr::registerIOProcessor::: IOProcessor with id = [%d] already registered", io_processor->getId());
  _currentVacantIOProcessor = io_processor;

  smsc_log_debug(_logger, "IOProcessorMgr::registerIOProcessor::: new IOProcessor with id=%d has been registered",
                 io_processor->getId());
  return _currentVacantIOProcessor;
}

}}}
