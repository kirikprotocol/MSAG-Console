#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SUBMITSMRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SUBMITSMRESP_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/FastParsableSmppMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SubmitSmResp : public FastParsableSmppMessage {
public:
  SubmitSmResp()
    : FastParsableSmppMessage(_MSGCODE)
  {}

  static const uint32_t _MSGCODE = 0x80000004;

protected:
  virtual void generateSerializationException() const {
    throw utilx::SerializationException("SubmitSmResp::serialize::: message body wasn't set");
  }
};

}}}}

#endif
