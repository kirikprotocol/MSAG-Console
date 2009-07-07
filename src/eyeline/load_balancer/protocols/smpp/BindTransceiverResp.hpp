#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDTRANSCEIVERRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDTRANSCEIVERRESP_HPP__

# include "eyeline/load_balancer/protocols/smpp/BindResponse.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindTransceiverResp : public BindResponse {
public:
  BindTransceiverResp()
    : BindResponse(_MSGCODE) {}

  static const uint32_t _MSGCODE = 0x80000009;
};

}}}}

#endif
