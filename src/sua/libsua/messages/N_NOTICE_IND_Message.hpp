#ifndef __SUA_LIBSUA_MESSAGES_NNOTICEINDMESSAGE_HPP__
# define __SUA_LIBSUA_MESSAGES_NNOTICEINDMESSAGE_HPP__

# include <sua/libsua/messages/LibsuaMessage.hpp>

namespace libsua {

class N_NOTICE_IND_Message : public LibsuaMessage {
public:
  N_NOTICE_IND_Message();

  virtual ~N_NOTICE_IND_Message();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setCalledAddress(const uint8_t* address, uint8_t addressLen);
  variable_data_t getCalledAddress() const;

  void setCallingAddress(const uint8_t* address, uint8_t addressLen);
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
  uint8_t* _calledAddr;
  uint8_t _callingAddrLen;
  uint8_t* _callingAddr;
  uint8_t _reasonForReturn;
  uint16_t _userDataLen;
  uint8_t* _userData;
  uint8_t _importance;

  static const uint32_t _MSG_CODE=0x05;
  enum { SET_IMPORTANCE = 0x01 };
};

}

#endif
