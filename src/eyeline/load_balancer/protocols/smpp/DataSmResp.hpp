#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DATASMRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DATASMRESP_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class DataSmResp : public FastParsableSmppMessage {
public:
  DataSmResp()
    : FastParsableSmppMessage(_MSGCODE)
  {}

  static const uint32_t _MSGCODE = 0x80000103;

protected:
  virtual void generateSerializationException() const {
    throw utilx::SerializationException("DataSmResp::serialize::: message body wasn't set");
  }
};

}}}}

#endif

