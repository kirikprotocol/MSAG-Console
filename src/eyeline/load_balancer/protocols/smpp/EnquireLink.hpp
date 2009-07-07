#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_ENQUIRELINK_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_ENQUIRELINK_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/EnquireLinkResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class EnquireLink : public SMPPMessage {
public:
  EnquireLink()
    : SMPPMessage(_MSGCODE) {}

  virtual uint32_t calculateCommandBodyLength() const { return 0; }

  EnquireLinkResp makeResponse(uint32_t command_status) const {
    EnquireLinkResp enquireLinkResp;
    enquireLinkResp.setCommandStatus(command_status);
    enquireLinkResp.setSequenceNumber(getSequenceNumber());
    return enquireLinkResp;
  }

  static const uint32_t _MSGCODE = 0x00000015;
};

}}}}

#endif
