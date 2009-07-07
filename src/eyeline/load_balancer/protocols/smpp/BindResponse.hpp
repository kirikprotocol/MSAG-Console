#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDRESPONSE_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDRESPONSE_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/OptionalParameter.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindResponse : public SMPPMessage {
public:
  explicit BindResponse(uint32_t commandId);

  virtual size_t serialize(io_subsystem::Packet* packet) const;

  virtual size_t deserialize(const io_subsystem::Packet* packet);

  const char* getSystemId() const;
  void setSystemId(const char* systemId);

  uint8_t getScInterfaceVersion();
  void setScInterfaceVersion(uint8_t scInterfaceVersion);

  virtual std::string toString() const;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

private:
  char _systemId[16];
  bool _isSetSystemId;

  OptionalParameter<uint8_t, 0x0210> _scInterfaceVersion;
};

}}}}

#endif
