#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_GENERICNACK_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_GENERICNACK_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class GenericNack : public SMPPMessage {
public:
  GenericNack()
    : SMPPMessage(_MSGCODE) {}

  static const uint32_t _MSGCODE = 0x80000000;

protected:
  virtual uint32_t calculateCommandBodyLength() const { return 0; }
};

}}}}

#endif

