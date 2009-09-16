#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_ACCEPTNEWCONNECTIONEVENT_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_ACCEPTNEWCONNECTIONEVENT_HPP__

# include "eyeline/corex/io/network/TCPSocket.hpp"
# include "eyeline/load_balancer/io_subsystem/IOEvent.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchCircuitController.hpp"
# include "eyeline/load_balancer/io_subsystem/types.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class AcceptNewConnectionEvent : public IOEvent {
public:
  AcceptNewConnectionEvent(IOProcessorRefPtr io_processor,
                           SwitchCircuitController& switch_circuit_ctrl,
                           corex::io::network::TCPSocket* new_socket)
  : _ioProcessor(io_processor), _switchCircuitCtrl(switch_circuit_ctrl),
    _newSocket(new_socket)
  {}

  virtual void handle();
private:
  IOProcessorRefPtr _ioProcessor;
  SwitchCircuitController& _switchCircuitCtrl;
  corex::io::network::TCPSocket* _newSocket;
};

}}}

#endif
