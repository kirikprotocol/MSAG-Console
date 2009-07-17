#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SUBMITSMRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SUBMITSMRESP_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class SubmitSmResp : public SMPPMessage {
public:
  SubmitSmResp()
  : SMPPMessage(_MSGCODE), _rawMessageBody(NULL), _rawMessageBodyLen(0)
  {}

  virtual size_t serialize(io_subsystem::Packet* packet) const;
  virtual size_t deserialize(const io_subsystem::Packet* packet);

  virtual std::string toString() const;

  static const uint32_t _MSGCODE = 0x80000004;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

private:
  const uint8_t* _rawMessageBody;
  uint32_t _rawMessageBodyLen;
};

}}}}

#endif
