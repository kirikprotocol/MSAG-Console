#ifndef __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_BINDCONFIRMMESSAGE_HPP__
# define __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_BINDCONFIRMMESSAGE_HPP__

# include <string>
# include <eyeline/sua/communication/libsua_messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace communication {
namespace libsua_messages {

class BindConfirmMessage : public LibsuaMessage {
public:
  typedef enum { BIND_OK = 0, UNKNOWN_APP_ID_VALUE = 1, UNSUPPORTED_PROTOCOL_VERSION = 2, COMPONENT_IS_INACTIVE = 3, SYSTEM_MALFUNCTION = 255 } status_t;

  BindConfirmMessage();

  explicit BindConfirmMessage(status_t status);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  uint32_t getStatus() const;
  void setStatus(status_t status);

protected:
  virtual uint32_t getLength() const;

private:
  uint32_t _status;

  static const uint32_t _MSG_CODE=0x81;

};

}}}}

#endif
