#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_CANCELSM_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_CANCELSM_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class CancelSm : public SMPPMessage {
public:
  CancelSm();

  virtual size_t serialize(io_subsystem::Packet* packet) const;
  virtual size_t deserialize(const io_subsystem::Packet* packet);

  virtual std::string toString() const;

  uint64_t getMessageId() const;

  static const uint32_t _MSGCODE = 0x00000008;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

private:
  const uint8_t* _rawMessageBody;
  uint32_t _rawMessageBodyLen, _messageIdCStringLen;

  bool _isSetMessageId;
  uint64_t _messageId;
};

}}}}

#endif

