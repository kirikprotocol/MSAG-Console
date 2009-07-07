#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_UNBINDRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_UNBINDRESP_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class UnbindResp : public SMPPMessage {
public:
  UnbindResp()
    : SMPPMessage(_MSGCODE) {}

  static const uint32_t _MSGCODE = 0x80000006;

protected:
  virtual uint32_t calculateCommandBodyLength() const { return 0; }
};

}}}}

#endif

