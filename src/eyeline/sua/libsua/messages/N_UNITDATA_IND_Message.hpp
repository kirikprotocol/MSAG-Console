#ifndef __EYELINE_SUA_LIBSUA_MESSAGES_NUNITDATAINDMESSAGE_HPP__
# define __EYELINE_SUA_LIBSUA_MESSAGES_NUNITDATAINDMESSAGE_HPP__

# include <eyeline/sua/libsua/messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class N_UNITDATA_IND_Message : public LibsuaMessage {
public:
  N_UNITDATA_IND_Message();
  virtual ~N_UNITDATA_IND_Message();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setSequenceControl(uint32_t sequenceControl);
  uint32_t getSequenceControl() const;

  void setCalledAddress(const uint8_t* address, uint8_t addressLen);
  variable_data_t getCalledAddress() const;

  void setCallingAddress(const uint8_t* address, uint8_t addressLen);
  variable_data_t getCallingAddress() const;

  void setUserData(const uint8_t* data, uint16_t dataLen);
  variable_data_t getUserData() const;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint32_t _sequenceControl;
  uint8_t _calledAddrLen;
  uint8_t* _calledAddr;
  uint8_t _callingAddrLen;
  uint8_t* _callingAddr;
  uint16_t _userDataLen;
  uint8_t* _userData;

  static const uint32_t _MSG_CODE=0x04;
  enum { SET_SEQUENCE_CONTROL = 0x01 };
};

}}}

#endif
