#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPPROTOCOLSTATECONTROLLER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMPPPROTOCOLSTATECONTROLLER_HPP__

# include "eyeline/utilx/prot_fsm/ProtocolStateController.hpp"
# include "eyeline/utilx/prot_fsm/TCPIndicationPrimitive.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

typedef utilx::prot_fsm::ProtocolStateController<SMPPMessage, utilx::prot_fsm::TCPIndicationPrimitive> SMPPProtocolStateController;
typedef utilx::prot_fsm::ProtocolState<SMPPMessage, utilx::prot_fsm::TCPIndicationPrimitive> SMPPProtocolState;

}}}}

#endif
