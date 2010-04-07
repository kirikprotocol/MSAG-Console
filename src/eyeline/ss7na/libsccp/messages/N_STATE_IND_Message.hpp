#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_NSTATEMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_NSTATEMESSAGE_HPP__

# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class N_STATE_IND_Message : public LibsccpMessage {
public:
  N_STATE_IND_Message();

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* result_buf, size_t result_buf_max_sz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packet_buf, size_t packet_buf_sz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "N_STATE_IND_Message"; }

  void setPointCode(uint32_t point_code) { _pointCode = point_code; }
  uint32_t getPointCode() const { return _pointCode; }

  void setSSN(uint8_t ssn) { _ssn = ssn; }
  uint8_t getSSN() const { return _ssn; }

  void setUserStatus(uint8_t user_status) { _userStatus = user_status; }
  uint8_t getUserStatus() const { return _userStatus; }

  void setSubsystemMultiplicityInd(uint8_t subsystem_mult_ind);
  uint8_t getSubsystemMultiplicityInd() const;

  static const uint8_t USER_IN_SERVICE=0;
  static const uint8_t USER_OUT_OF_SERVICE=1;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint32_t _pointCode;
  uint8_t _ssn;
  uint8_t _userStatus;
  uint8_t _subsystemMultiplicityIndicator;

  static const uint32_t _MSG_CODE=0x07;
  enum { SET_SUBSYSTEM_MULTIPLICITY_IND = 0x01 };
};

}}}

#endif
