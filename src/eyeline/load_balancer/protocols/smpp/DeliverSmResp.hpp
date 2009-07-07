#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DELIVERSMRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DELIVERSMRESP_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/FastParsableSmppMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class DeliverSmResp : public FastParsableSmppMessage {
public:
  DeliverSmResp()
    : FastParsableSmppMessage(_MSGCODE)
  {}

  static const uint32_t _MSGCODE = 0x80000005;

protected:
  virtual void generateSerializationException() const {
    throw utilx::SerializationException("DeliverSmResp::serialize::: message body wasn't set");
  }
};

}}}}

#endif
