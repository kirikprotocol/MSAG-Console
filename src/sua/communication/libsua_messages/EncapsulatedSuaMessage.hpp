#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_SUAENCAPSULATEDMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_SUAENCAPSULATEDMESSAGE_HPP__

# include <string>
# include <logger/Logger.h>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/communication/TP.hpp>
# include <sua/communication/sua_messages/SUAMessage.hpp>

namespace libsua_messages {

class EncapsulatedSuaMessage : public LibsuaMessage {
public:
  EncapsulatedSuaMessage(uint32_t msgCode=0);

  EncapsulatedSuaMessage(const sua_messages::SUAMessage* suaMessage);

  EncapsulatedSuaMessage(uint32_t msgCode, const uint8_t* suaMessageBody, uint32_t suaMessageBodySz);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  virtual const sua_messages::SUAMessage* getContainedSuaMessage() const;

  static const uint32_t _MIN_MSG_CODE=0x01, _MAX_MSG_CODE=0x0A;

  static const uint32_t ENCAPSULATED_SUA_ERROR_MESSAGE_CODE = 0x01;
  static const uint32_t ENCAPSULATED_SUA_NOTIFY_MESSAGE_CODE = 0x02;
  static const uint32_t ENCAPSULATED_SUA_CLDT_MESSAGE_CODE = 0x09;

private:
  uint32_t produceSuaMsgCode(uint32_t libsuaMsgCode);
  uint32_t produceLibsuaMsgCode(uint32_t suaMsgCode);
  void setEncapsulatedSuaMessage(const sua_messages::SUAMessage* suaMessage);

  communication::TP _encapsulatedMessageTP;

  smsc::logger::Logger *_logger;
  const sua_messages::SUAMessage* _suaMessage;
  uint8_t* _encapsulatedMessagePtr;
  uint32_t _encapsulatedMessageSz;
};

} // namespace sua_user_communication

#include <sua/utilx/PreallocatedMemoryManager.hpp>

namespace utilx {

template<> inline
alloc_mem_desc_t*
PreallocatedMemoryManager::getMemory<PreallocatedMemoryManager::MEM_FOR_ENCAPSULATED_MESSAGE>() {
  static TSD_Init_Helper<MEM_FOR_ENCAPSULATED_MESSAGE> tsd_init;
  alloc_mem_desc_t* res = allocateMemory(communication::Message::MAX_MESSAGE_SIZE, tsd_init._tsd_memory_key);
  return res;
}

}

#endif
