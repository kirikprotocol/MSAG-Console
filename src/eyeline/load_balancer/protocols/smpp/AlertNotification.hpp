#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_ALERTNOTIFICATION_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_ALERTNOTIFICATION_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/FastParsableSmppMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class AlertNotification : public FastParsableSmppMessage {
public:
  AlertNotification()
    : FastParsableSmppMessage(_MSGCODE)
  {}

  static const uint32_t _MSGCODE = 0x00000102;

protected:
  virtual void generateSerializationException() const {
    throw utilx::SerializationException("AlertNotification::serialize::: message body wasn't set");
  }
};

}}}}

#endif
