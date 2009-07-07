#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_UNBIND_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_UNBIND_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/UnbindResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class Unbind : public SMPPMessage {
public:
  Unbind()
    : SMPPMessage(_MSGCODE) {}

  static const uint32_t _MSGCODE = 0x00000006;

  UnbindResp makeResponse(uint32_t command_status) const {
    UnbindResp unbindResp;
    unbindResp.setCommandStatus(command_status);
    unbindResp.setSequenceNumber(getSequenceNumber());
    return unbindResp;
  }
protected:
  virtual uint32_t calculateCommandBodyLength() const { return 0; }
};

}}}}

#endif

