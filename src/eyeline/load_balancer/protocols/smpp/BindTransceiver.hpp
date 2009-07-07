#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDTRANSCEIVER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDTRANSCEIVER_HPP__

# include "eyeline/load_balancer/protocols/smpp/BindRequest.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindTransceiverResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindTransceiver : public BindRequest {
public:
  BindTransceiver()
    : BindRequest(_MSGCODE) {}

  virtual BindTransceiverResp* makeResponse(uint32_t command_status) const {
    BindTransceiverResp* bindTransceiverResp = new BindTransceiverResp();
    bindTransceiverResp->setCommandStatus(command_status);
    bindTransceiverResp->setSequenceNumber(getSequenceNumber());
    bindTransceiverResp->setSystemId(getSystemId());
    return bindTransceiverResp;
  }

  static const uint32_t _MSGCODE = 0x00000009;
};

}}}}

#endif
