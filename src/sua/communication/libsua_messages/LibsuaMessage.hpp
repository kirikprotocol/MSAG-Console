#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_LIBSUAMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_LIBSUAMESSAGE_HPP__

# include <sua/communication/Message.hpp>
# include <sua/communication/sua_messages/SUAMessage.hpp>

namespace libsua_messages {

class LibsuaMessage : public communication::Message {
public:
  LibsuaMessage(uint32_t msgCode);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual uint32_t getMsgCode() const;

  virtual uint16_t getStreamNo() const;

  virtual const sua_messages::SUAMessage* getContainedSuaMessage() const;

  static unsigned int getMessageIndex(uint32_t msgCode);
  static void registerMessageCode(uint32_t msgCode);

protected:
  virtual uint32_t getLength() const;

  bool isLengthGotFromMessageBuffer() const;

  void setLength(uint32_t msgLen);

  void setEncapsulatedSuaMessage(const sua_messages::SUAMessage* suaMessage);

  enum {MAX_MESSAGE_CODE_VALUE = 0xFF};

  static unsigned int _messagesCodeToMessageIdx[MAX_MESSAGE_CODE_VALUE];

  static const size_t _MSGCODE_SZ = sizeof(uint32_t), _MSGLEN_SZ = sizeof(uint32_t);
private:
  const sua_messages::SUAMessage* _suaMessage;
  uint32_t _msgLen;
  uint32_t _msgCode;
};

} // namespace sua_user_communication



#endif /* SUA_USER_COMMUNICATION_LIBSUAMESSAGE_HPP_HEADER_INCLUDED_B87B6879 */
