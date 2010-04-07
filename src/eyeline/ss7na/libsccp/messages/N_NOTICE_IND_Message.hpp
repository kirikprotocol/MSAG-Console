#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_NNOTICEINDMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_NNOTICEINDMESSAGE_HPP__

# include "eyeline/utilx/types.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class N_NOTICE_IND_Message : public LibsccpMessage {
public:
  N_NOTICE_IND_Message();
  N_NOTICE_IND_Message(const N_UNITDATA_REQ_Message& message,
                       common::return_cause_value_t reason_for_return);

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* result_buf, size_t result_buf_max_sz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packet_buf, size_t packet_buf_sz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "N_NOTICE_IND_Message"; }

  void setCalledAddress(const uint8_t* address, uint8_t address_len);
  void setCalledAddress(const sccp::SCCPAddress& address);
  utilx::variable_data_t getCalledAddress() const;

  void setCallingAddress(const uint8_t* address, uint8_t address_len);
  void setCallingAddress(const sccp::SCCPAddress& address);
  utilx::variable_data_t getCallingAddress() const;

  void setReasonForReturn(common::return_cause_value_t reason_for_return) { _reasonForReturn = uint8_t(reason_for_return); }
  void setReasonForReturn(uint8_t reason_for_return) { _reasonForReturn = reason_for_return; }
  uint8_t getReasonForReturn() const { return _reasonForReturn; }

  void setUserData(const uint8_t* data, uint16_t data_len);
  utilx::variable_data_t getUserData() const;

  void setImportance(uint8_t importance);
  uint8_t getImportance() const;

  typedef enum { NO_TRANSLATION_FOR_THIS_SPECIFIC_ADDRESS = 0x01, UNQUALIFIED = 0x07 } reason_for_return_t;

protected:
  virtual uint32_t getLength() const;

  uint8_t _fieldsMask;
  uint8_t _calledAddrLen;
  uint8_t _calledAddr[255];
  uint8_t _callingAddrLen;
  uint8_t _callingAddr[255];
  uint8_t _reasonForReturn;
  uint16_t _userDataLen;
  uint8_t _userData[common::TP::MAX_PACKET_SIZE];
  uint8_t _importance;

  static const unsigned FIXED_MSG_PART_SZ = _MSGCODE_SZ + sizeof(uint8_t) /*_fieldsMask*/ +
        sizeof(uint8_t) /*_returnOption*/ + sizeof(uint8_t) /*_calledAddrLen*/ +
        sizeof(uint8_t) /*_callingAddrLen*/ + sizeof(uint16_t) /*_userDataLen*/;

  static const uint32_t _MSG_CODE=0x05;
  enum { SET_IMPORTANCE = 0x01 };
};

}}}

#endif
