#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_BINDCONFIRMMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_BINDCONFIRMMESSAGE_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class BindConfirmMessage : public LibsccpMessage {
public:
  typedef enum { BIND_OK = 0, UNKNOWN_APP_ID_VALUE = 1, UNSUPPORTED_PROTOCOL_VERSION = 2, COMPONENT_IS_INACTIVE = 3, APP_ALREADY_ACTIVE = 4,  SYSTEM_MALFUNCTION = 255 } status_t;

  BindConfirmMessage();
  explicit BindConfirmMessage(status_t status);

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "BIND_CONFIRM"; }

  uint32_t getStatus() const { return _status; }
  void setStatus(status_t status) { _status = status; }

  const sccp::SCCPAddress & getSCCPAddress(void) const { return _sccpAddr; }
  void setSCCPAddress(const sccp::SCCPAddress & sccp_addr) { _sccpAddr = sccp_addr; }

protected:
  virtual uint32_t getLength() const;

private:
  uint32_t          _status;
  sccp::SCCPAddress _sccpAddr; //TODO: implement serialization/deserialization!!!

  static const uint32_t _MSG_CODE=0x81;

};

}}}

#endif
