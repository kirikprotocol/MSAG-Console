#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DATASMRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_DATASMRESP_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/FastParsableSmppMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/codec_utility.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class DataSmResp : public FastParsableSmppMessage {
public:
  DataSmResp()
    : FastParsableSmppMessage(_MSGCODE), _isMessageIdWasSetEmptyExplicitly(false)
  {}

  virtual size_t serialize(io_subsystem::Packet* packet) const {
    if ( _isMessageIdWasSetEmptyExplicitly ) {
      SMPPMessage::serialize(packet);
      return addCOctetString(packet, NULL, 0);
    } else
      return FastParsableSmppMessage::serialize(packet);
  }

  void setEmptyMessageId() {
    _isMessageIdWasSetEmptyExplicitly = true;
  }

  static const uint32_t _MSGCODE = 0x80000103;

protected:
  virtual void generateSerializationException() const {
    throw utilx::SerializationException("DataSmResp::serialize::: message body wasn't set");
  }
private:
  bool _isMessageIdWasSetEmptyExplicitly;
};

}}}}

#endif

