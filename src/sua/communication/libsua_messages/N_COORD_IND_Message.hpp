#ifndef __SUA_COMMUNICATION_LIBSUAMESSAGES_NCOORDMESSAGE_HPP__
# define __SUA_COMMUNICATION_LIBSUAMESSAGES_NCOORDMESSAGE_HPP__

# include <sys/types.h>
# include <string>
# include <sua/communication/libsua_messages/LibsuaMessage.hpp>
# include <sua/communication/TP.hpp>

namespace libsua_messages {

class N_COORD_IND_Message : public LibsuaMessage {
public:
  N_COORD_IND_Message();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setSignalingPointCode(uint16_t pointCode);
  uint16_t getSignalingPointCode() const;

  void setSSN(uint8_t ssn);
  uint8_t getSSN() const;

  void setSubsystemMultiplicityInd(uint8_t subsystemMultInd);
  uint8_t getSubsystemMultiplicityInd() const;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint16_t _pointCode;
  uint8_t _ssn;
  uint8_t _subsystemMultiplicityIndicator;

  static const uint32_t _MSG_CODE=0x08;
  enum { SET_SUBSYSTEM_MULTIPLICITY_IND = 0x01 };
};

}

#endif
