#ifndef __SUA_LIBSUA_MESSAGES_NSTATEMESSAGE_HPP__
# define __SUA_LIBSUA_MESSAGES_NSTATEMESSAGE_HPP__

# include <sua/libsua/messages/LibsuaMessage.hpp>

namespace libsua {

class N_STATE_IND_Message : public LibsuaMessage {
public:
  N_STATE_IND_Message();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setPointCode(uint16_t pointCode);
  uint16_t getPointCode() const;

  void setSSN(uint8_t ssn);
  uint8_t getSSN() const;

  void setUserStatus(uint8_t userStatus);
  uint8_t getUserStatus() const;

  void setSubsystemMultiplicityInd(uint8_t subsystemMultInd);
  uint8_t getSubsystemMultiplicityInd() const;

  static const uint8_t USER_IN_SERVICE=0;
  static const uint8_t USER_OUT_OF_SERVICE=1;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint16_t _pointCode;
  uint8_t _ssn;
  uint8_t _userStatus;
  uint8_t _subsystemMultiplicityIndicator;

  static const uint32_t _MSG_CODE=0x07;
  enum { SET_SUBSYSTEM_MULTIPLICITY_IND = 0x01 };
};

}

#endif
