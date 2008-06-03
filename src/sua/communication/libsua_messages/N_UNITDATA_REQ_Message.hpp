#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_NUNITDATAREQMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_NUNITDATAREQMESSAGE_HPP__

# include <sys/types.h>
# include <string>
# include <sua/communication/sua_messages/CLDTMessage.hpp>
# include <sua/communication/sua_messages/SCCPAddress.hpp>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/TP.hpp>
# include <core/buffers/TmpBuf.hpp>

namespace libsua_messages {

class N_UNITDATA_REQ_Message : public LibsuaMessage {
public:
  N_UNITDATA_REQ_Message();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setSequenceControl(uint32_t sequenceControl);
  bool isSetSequenceControl() const;
  uint32_t getSequenceControl() const;

  // on_off == true --> return message on error, else discard message
  void setReturnOption(bool on_off);
  bool getReturnOption() const;

  void setImportance(uint8_t importance);
  bool isSetImportance() const;
  uint8_t getImportance() const;

  void setHopCounter(uint8_t hopCounter);
  bool isSetHopCounter() const;
  uint8_t getHopCounter() const;

  void setCalledAddress(const uint8_t* address, uint8_t addressLen);
  void setCalledAddress(const sua_messages::SCCPAddress& address);
  variable_data_t getCalledAddress() const;

  void setCallingAddress(const uint8_t* address, uint8_t addressLen);
  void setCallingAddress(const sua_messages::SCCPAddress& address);
  variable_data_t getCallingAddress() const;

  void setUserData(const uint8_t* data, uint16_t dataLen);
  variable_data_t getUserData() const;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint32_t _sequenceControl;
  uint8_t _returnOption;
  uint8_t _importance;
  uint8_t _hopCounter;
  uint8_t _calledAddrLen;
  uint8_t _calledAddr[255];
  uint8_t _callingAddrLen;
  uint8_t _callingAddr[255];
  uint16_t _userDataLen;
  uint8_t _userData[communication::TP::MAX_PACKET_SIZE];

  static const uint32_t _MSG_CODE=0x03;
  enum { SET_SEQUENCE_CONTROL = 0x01, SET_IMPORTANCE = 0x02, SET_HOP_COUNTER = 0x04 };
};

}

#endif
