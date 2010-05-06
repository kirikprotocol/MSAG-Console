#include <utility>

#include "core/synchronization/MutexGuard.hpp"
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
  _logger(smsc::logger::Logger::getInstance("io_subsystem")),
  _listeningIface(new corex::io::network::TCPServerSocket(listening_host, listening_port)),
  _newConnEventsPublisher(max_new_conn_events_queue_sz),
  _newConnEventsProcessor("NewConnEventProcPool", max_newconn_event_handlers, _newConnEventsPublisher),
  _currentVacantIOProcessorIter(_ioProcessors.begin()), _ioParametersWasSet(false)
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
      _newConnEventsPublisher.publish(new AcceptNewConnectionEvent(getVacantIOProcessor(),
                                                                   *_switchCircuitCtrl,
                                                                   newSocket));
    } catch (std::exception& ex) {
      smsc_log_debug(_logger, "IOProcessorMgr::Execute::: caught unexpected exception=[%s]",
                     ex.what());
    }
  }

  _shutdown();
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
}

void
IOProcessorMgr::_shutdown()
{
  _listeningIface->close();
  _newConnEventsProcessor.shutdown();
  smsc::core::synchronization::MutexGuard synchronize(_ioProcessorsLock);
  while ( !_ioProcessors.empty() ) {
    registered_ioprocs_t::iterator iter = _ioProcessors.begin();
    smsc_log_debug(_logger, "IOProcessorMgr::shutdown::: try shutdown IOProcessor with id=%d",
                     iter->second->getId());
    try {
      iter->second->shutdown();
    } catch (...) {}

    _ioProcessors.erase(iter);
  }
  smsc_log_debug(_logger, "IOProcessorMgr::shutdown::: IOProcessorMgr with id=%d has been stopped",
                 _ioProcMgrId);
}

void
IOProcessorMgr::setParameters(const IOParameters& io_parameters)
{
  _ioParameters = io_parameters;
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

IOProcessorRefPtr
IOProcessorMgr::getVacantIOProcessor()
{
  smsc::core::synchronization::MutexGuard synchronize(_ioProcessorsLock);

  if ( _currentVacantIOProcessorIter != _ioProcessors.end() ) {
    if ( _currentVacantIOProcessorIter->second->canProcessNewSocket() ) {
      smsc_log_debug(_logger, "IOProcessorMgr::getVacantIOProcessor::: return free IOProcessor with id=%d",
                     _currentVacantIOProcessorIter->second->getId());
      return _currentVacantIOProcessorIter->second;
    } else {
      if ( ++_currentVacantIOProcessorIter == _ioProcessors.end() )
        _currentVacantIOProcessorIter = _ioProcessors.begin();
      while ( _currentVacantIOProcessorIter != _ioProcessors.end() ) {
        if ( _currentVacantIOProcessorIter->second->canProcessNewSocket() ) {
          smsc_log_debug(_logger, "IOProcessorMgr::getVacantIOProcessor::: return free IOProcessor with id=%d",
                         _currentVacantIOProcessorIter->second->getId());
          return _currentVacantIOProcessorIter->second;
        }
        ++_currentVacantIOProcessorIter;
      }
    }
  }
  return createNewIOProcessor();
}

IOProcessorRefPtr
IOProcessorMgr::getIOProcessor(unsigned io_proc_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_ioProcessorsLock);
  registered_ioprocs_t::iterator iter = _ioProcessors.find(io_proc_id);
  if ( iter == _ioProcessors.end() )
    return IOProcessorRefPtr(NULL);
  return iter->second;
}

IOProcessorRefPtr
IOProcessorMgr::registerIOProcessor(IOProcessor* io_processor)
{
  // because registerIOProcessor called only from createNewIOProcessor() method
  // _ioProcessorsLock has been already acquired, so there we don't try acquire it
  std::pair<registered_ioprocs_t::iterator, bool> res = _ioProcessors.insert(std::make_pair(io_processor->getId(), io_processor));
  if ( !res.second )
    throw smsc::util::Exception("IOProcessorMgr::registerIOProcessor::: IOProcessor with id = [%d] already registered", io_processor->getId());
  //_currentVacantIOProcessor = io_processor;
  _currentVacantIOProcessorIter = res.first;

  smsc_log_debug(_logger, "IOProcessorMgr::registerIOProcessor::: new IOProcessor with id=%d has been registered",
                 io_processor->getId());
  return res.first->second;
}

}}}
