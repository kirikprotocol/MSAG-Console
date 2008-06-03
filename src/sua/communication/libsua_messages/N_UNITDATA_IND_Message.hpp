#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_NUNITDATAINDMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_NUNITDATAINDMESSAGE_HPP__

# include <sys/types.h>
# include <string>
# include <sua/communication/sua_messages/CLDTMessage.hpp>
# include <sua/communication/sua_messages/SCCPAddress.hpp>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>
# include <sua/communication/TP.hpp>
# include <core/buffers/TmpBuf.hpp>

namespace libsua_messages {

class N_UNITDATA_IND_Message : public LibsuaMessage {
public:
  N_UNITDATA_IND_Message();
  N_UNITDATA_IND_Message(const sua_messages::CLDTMessage& cldtMessage);
  N_UNITDATA_IND_Message(const N_UNITDATA_REQ_Message& n_unitdata_req_message);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setSequenceControl(uint32_t sequenceControl);
  uint32_t getSequenceControl() const;

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
  uint8_t _calledAddrLen;
  uint8_t _calledAddr[255];
  uint8_t _callingAddrLen;
  uint8_t _callingAddr[255];
  uint16_t _userDataLen;
  uint8_t _userData[communication::TP::MAX_PACKET_SIZE];

  static const uint32_t _MSG_CODE=0x04;
  enum { SET_SEQUENCE_CONTROL = 0x01 };
};

}

#endif
