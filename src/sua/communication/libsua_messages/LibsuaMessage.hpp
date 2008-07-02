#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_LIBSUAMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_LIBSUAMESSAGE_HPP__

# include <sua/communication/Message.hpp>

namespace libsua_messages {

struct variable_data_t {
  variable_data_t(const uint8_t* aData, uint8_t aDataLen)
    : data(aData), dataLen(aDataLen) {}
  const uint8_t* data;
  uint8_t dataLen;
};

class LibsuaMessage : public communication::Message {
public:
  LibsuaMessage(uint32_t msgCode);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual uint32_t getMsgCode() const;

  static unsigned int getMessageIndex(uint32_t msgCode);

  static void registerMessageCode(uint32_t msgCode);

protected:
  virtual uint32_t getLength() const;

  void setLength(uint32_t msgLen);

  enum { MAX_MESSAGE_CODE_VALUE = 0xFF };

  static unsigned int _messagesCodeToMessageIdx[MAX_MESSAGE_CODE_VALUE];

  static const size_t _MSGCODE_SZ = sizeof(uint32_t), _MSGLEN_SZ = sizeof(uint32_t);
private:
  uint32_t _msgLen;
  uint32_t _msgCode;
};

} // namespace sua_user_communication



#endif /* SUA_USER_COMMUNICATION_LIBSUAMESSAGE_HPP_HEADER_INCLUDED_B87B6879 */
