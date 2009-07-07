#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_FASTPARSABLESMPPMESSAGE_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_FASTPARSABLESMPPMESSAGE_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class FastParsableSmppMessage : public SMPPMessage {
public:
  FastParsableSmppMessage(uint32_t commandId);

  virtual size_t serialize(io_subsystem::Packet* packet) const;
  virtual size_t deserialize(const io_subsystem::Packet* packet);

  virtual std::string toString() const;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

  virtual void generateSerializationException() const = 0;
private:
  const uint8_t* _rawMessageBody;
  uint32_t _rawMessageBodyLen;
};

}}}}

#endif
