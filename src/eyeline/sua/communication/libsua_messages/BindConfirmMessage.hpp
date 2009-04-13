#ifndef __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_BINDCONFIRMMESSAGE_HPP__
#ident "@(#)$Id$"
# define __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_BINDCONFIRMMESSAGE_HPP__

# include <string>
# include "eyeline/sua/communication/libsua_messages/LibsuaMessage.hpp"

namespace eyeline {
namespace sua {
namespace communication {
namespace libsua_messages {

class BindConfirmMessage : public LibsuaMessage {
public:
  //NOTE: BindConfirmMessage::BindResult_e values biuniquely conform
  //to corresponding SuaApi::ErrorCode_e values !!!
  enum BindResult_e { BIND_OK = 0,
    UNKNOWN_APP_ID_VALUE = 1, UNSUPPORTED_PROTOCOL_VERSION = 2,
    COMPONENT_IS_INACTIVE = 3, SYSTEM_MALFUNCTION = 255
  };
  typedef enum BindResult_e status_t;

  BindConfirmMessage();

  explicit BindConfirmMessage(status_t status);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  status_t getStatus() const { return static_cast<BindResult_e>(_status); }

  void setStatus(status_t status) { _status = status; }

protected:
  virtual uint32_t getLength() const;

private:
  uint32_t _status;

  static const uint32_t _MSG_CODE=0x81;

};

}}}}

#endif /* __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_BINDCONFIRMMESSAGE_HPP__ */

