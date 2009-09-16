#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDTRANSMITTER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDTRANSMITTER_HPP__

# include "eyeline/load_balancer/protocols/smpp/BindRequest.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindTransmitterResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindTransmitter : public BindRequest {
public:
  BindTransmitter()
    : BindRequest(_MSGCODE) {}

  virtual BindTransmitterResp* makeResponse(uint32_t command_status) const {
    BindTransmitterResp* bindTransmitterResp = new BindTransmitterResp();
    bindTransmitterResp->setCommandStatus(command_status);
    bindTransmitterResp->setSequenceNumber(getSequenceNumber());
    bindTransmitterResp->setSystemId(getSystemId());
    return bindTransmitterResp;
  }

  virtual std::string getBindType() const { return "TX"; }

  static const uint32_t _MSGCODE = 0x00000002;
};

}}}}

#endif
