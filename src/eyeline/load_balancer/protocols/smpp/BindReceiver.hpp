#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDRECEIVER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDRECEIVER_HPP__

# include "eyeline/load_balancer/protocols/smpp/BindRequest.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindReceiverResp.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindReceiver : public BindRequest {
public:
  BindReceiver()
    : BindRequest(_MSGCODE) {}

  virtual BindReceiverResp* makeResponse(uint32_t command_status) const {
    BindReceiverResp* bindReceiverResp = new BindReceiverResp();
    bindReceiverResp->setCommandStatus(command_status);
    bindReceiverResp->setSequenceNumber(getSequenceNumber());
    bindReceiverResp->setSystemId(getSystemId());
    return bindReceiverResp;
  }

  virtual std::string getBindType() const { return "RX"; }

  static const uint32_t _MSGCODE = 0x00000001;
};

}}}}

#endif
