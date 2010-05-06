#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_NUNITDATAREQMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_NUNITDATAREQMESSAGE_HPP__

# include "eyeline/utilx/types.hpp"
# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class N_UNITDATA_REQ_Message : public LibsccpMessage {
public:
  N_UNITDATA_REQ_Message();

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* result_buf, size_t result_buf_max_sz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packet_buf, size_t packet_buf_sz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "N_UNITDATA_REQ_Message"; }

  void setSequenceControl(uint32_t sequence_control);
  bool isSetSequenceControl() const;
  uint32_t getSequenceControl() const;

  // on_off == true --> return message on error, else discard message
  void setReturnOption(bool on_off) { _returnOption = on_off; }
  bool getReturnOption() const { return _returnOption; }

  void setImportance(uint8_t importance);
  bool isSetImportance() const;
  uint8_t getImportance() const;

  void setHopCounter(uint8_t hop_counter);
  bool isSetHopCounter() const;
  uint8_t getHopCounter() const;

  void setCalledAddress(const uint8_t* address, uint8_t address_len);
  utilx::variable_data_t getCalledAddress() const;

  void setCallingAddress(const uint8_t* address, uint8_t address_len);
  utilx::variable_data_t getCallingAddress() const;

  void setUserData(const uint8_t* data, uint16_t data_len);
  utilx::variable_data_t getUserData() const;

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

  static const unsigned FIXED_MSG_PART_SZ = _MSGCODE_SZ + sizeof(uint8_t) /*_fieldsMask*/ +
      sizeof(uint8_t) /*_returnOption*/ + sizeof(uint8_t) /*_calledAddrLen*/ +
      sizeof(uint8_t) /*_callingAddrLen*/ + sizeof(uint16_t) /*_userDataLen*/;

  uint8_t _userDataBuf[common::TP::MAX_PACKET_SIZE];
  const uint8_t* _userData;

  static const uint32_t _MSG_CODE=0x03;
  enum { SET_SEQUENCE_CONTROL = 0x01, SET_IMPORTANCE = 0x02, SET_HOP_COUNTER = 0x04 };
};

}}}

#endif
