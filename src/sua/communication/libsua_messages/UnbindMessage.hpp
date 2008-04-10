#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_UNBINDMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_UNBINDMESSAGE_HPP__

# include <string>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>

namespace libsua_messages {

class UnbindMessage : public LibsuaMessage {
public:
  UnbindMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  const char* getMsgCodeTextDescription() const;
private:
  static const uint32_t _MSG_CODE=0x42;
};

} // namespace sua_user_communication

#endif /* SUA_USER_COMMUNICATION_BINDMESSAGE_HPP_HEADER_INCLUDED_B87B76B1 */
