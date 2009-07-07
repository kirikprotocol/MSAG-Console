#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SUBMITMULTIRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SUBMITMULTIRESP_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/FastParsableSmppMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SubmitMultiResp : public FastParsableSmppMessage {
public:
  SubmitMultiResp()
    : FastParsableSmppMessage(_MSGCODE)
  {}

  static const uint32_t _MSGCODE = 0x80000021;

protected:
  virtual void generateSerializationException() const {
    throw utilx::SerializationException("SubmitMultiResp::serialize::: message body wasn't set");
  }
};

}}}}

#endif
