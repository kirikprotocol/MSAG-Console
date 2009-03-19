#ifndef __EYELINE_SUA_LIBSUA_MESSAGES_NCOORDMESSAGE_HPP__
# define __EYELINE_SUA_LIBSUA_MESSAGES_NCOORDMESSAGE_HPP__

# include <eyeline/sua/libsua/messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class N_COORD_IND_Message : public LibsuaMessage {
public:
  N_COORD_IND_Message();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

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

}}}

#endif
