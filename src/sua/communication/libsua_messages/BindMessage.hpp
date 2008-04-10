#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_BINDMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_BINDMESSAGE_HPP__

# include <string>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/LinkId.hpp>

namespace libsua_messages {

class BindMessage : public LibsuaMessage {
public:
  BindMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  std::string getAppId() const;

  void setAppId(const std::string& appid);

private:
  char _appId[32+1];
  static const uint32_t _MSG_CODE=0x41;
};

} // namespace sua_user_communication

#endif /* SUA_USER_COMMUNICATION_BINDMESSAGE_HPP_HEADER_INCLUDED_B87B76B1 */
