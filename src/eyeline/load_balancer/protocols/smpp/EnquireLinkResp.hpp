#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_ENQUIRELINKRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_ENQUIRELINKRESP_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class EnquireLinkResp : public SMPPMessage {
public:
  EnquireLinkResp()
    : SMPPMessage(_MSGCODE) {}

  virtual uint32_t calculateCommandBodyLength() const { return 0; }

  static const uint32_t _MSGCODE = 0x80000015;
};

}}}}

#endif
