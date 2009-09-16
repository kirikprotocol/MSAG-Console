#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDREQUEST_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_BINDREQUEST_HPP__

# include "eyeline/load_balancer/protocols/smpp/SMPPMessage.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindResponse.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class BindRequest : public SMPPMessage {
public:
  explicit BindRequest(uint32_t commandId);

  virtual size_t serialize(io_subsystem::Packet* packet) const;

  virtual size_t deserialize(const io_subsystem::Packet* packet);

  virtual BindResponse* makeResponse(uint32_t command_status) const = 0;

  const char* getSystemId() const;
  void setSystemId(const char* systemId);

  const char* getPassword() const;
  void setPassword(const char* password);

  const char* getSystemType() const;
  void setSystemType(const char* systemType);

  uint8_t getInterfaceVersion() const;
  void setInterfaceVersion(uint8_t interfaceVersion);

  uint8_t getAddrTon() const;
  void setAddrTon(uint8_t addrTon);

  uint8_t getAddrNpi() const;
  void setAddrNpi(uint8_t addrNpi);

  const char* getAddressRange() const;
  void setAddressRange(const char* addrRange);

  virtual std::string toString() const;

  enum { BIND_FAILED=protocols::smpp::ESME_RBINDFAIL };

  virtual std::string getBindType() const = 0;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

private:
  char _systemId[16];
  bool _isSetSystemId;

  char _password[9];
  bool _isSetPassword;

  char _systemType[13];
  bool _isSetSystemType;

  uint8_t _interfaceVersion;
  bool _isSetInterfaceVersion;

  uint8_t _addrTon;
  bool _isSetAddrTon;

  uint8_t _addrNpi;
  bool _isSetAddrNpi;

  char _addrRange[41];
  bool _isSetAddrRange;
};

}}}}

#endif
