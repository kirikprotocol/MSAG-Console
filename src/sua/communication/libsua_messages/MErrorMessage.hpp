#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_MERRORMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_MERRORMESSAGE_HPP__ 1

# include <string>
# include <logger/Logger.h>
# include <sua/communication/TP.hpp>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/LinkId.hpp>

namespace libsua_messages {

class MErrorMessage : public LibsuaMessage {
public:
  MErrorMessage();
  MErrorMessage(uint32_t errCode, const LibsuaMessage* originalMessage);
  MErrorMessage(uint32_t errCode, const communication::TP* originalMessage);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  uint32_t getErrorCode() const;

  const LibsuaMessage* getOriginalMessage() const;

  enum {NO_SESSION_ESTABLISHED=0x01, BIND_PROCESSING_ERROR=0x02, MESSAGE_FORWARDING_ERROR_TO_SGP=0x03};

private:
  smsc::logger::Logger* _logger;
  uint32_t _errorCode;
  const LibsuaMessage* _originalMessage;
  const communication::TP* _originalPacket;
  static const uint32_t _MSG_CODE=0xFF;
};

} // namespace sua_user_communication

#endif
