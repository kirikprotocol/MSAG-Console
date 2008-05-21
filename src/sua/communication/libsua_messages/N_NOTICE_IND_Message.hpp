#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_NNOTICEINDMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_NNOTICEINDMESSAGE_HPP__

# include <sys/types.h>
# include <string>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>
# include <sua/communication/sua_messages/CLDRMessage.hpp>
# include <sua/communication/TP.hpp>

namespace libsua_messages {

class N_NOTICE_IND_Message : public LibsuaMessage {
public:
  N_NOTICE_IND_Message();
  N_NOTICE_IND_Message(const N_UNITDATA_REQ_Message& originalMessage, uint8_t reasonForReturn);
  N_NOTICE_IND_Message(const sua_messages::CLDRMessage& message);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setCalledAddress(const uint8_t* address, uint8_t addressLen);
  void setCalledAddress(const sua_messages::SCCPAddress& address);
  variable_data_t getCalledAddress() const;

  void setCallingAddress(const uint8_t* address, uint8_t addressLen);
  void setCallingAddress(const sua_messages::SCCPAddress& address);
  variable_data_t getCallingAddress() const;

  void setReasonForReturn(uint8_t hopCounter);
  uint8_t getReasonForReturn() const;

  void setUserData(const uint8_t* data, uint8_t dataLen);
  variable_data_t getUserData() const;

  void setImportance(uint8_t importance);
  uint8_t getImportance() const;

  typedef enum { NO_TRANSLATION_FOR_THIS_SPECIFIC_ADDRESS = 0x01, UNQUALIFIED = 0x07 } reason_for_return_t;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint8_t _calledAddrLen;
  uint8_t _calledAddr[255];
  uint8_t _callingAddrLen;
  uint8_t _callingAddr[255];
  uint8_t _reasonForReturn;
  uint16_t _userDataLen;
  uint8_t _userData[communication::TP::MAX_PACKET_SIZE];
  uint8_t _importance;

  static const uint32_t _MSG_CODE=0x05;
  enum { SET_IMPORTANCE = 0x01 };
};

}

#endif
