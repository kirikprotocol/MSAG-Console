#ifndef __EYELINE_SUA_LIBSUA_MESSAGES_NPCSTATEMESSAGE_HPP__
# define __EYELINE_SUA_LIBSUA_MESSAGES_NPCSTATEMESSAGE_HPP__

# include <eyeline/sua/libsua/messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class N_PCSTATE_IND_Message : public LibsuaMessage {
public:
  N_PCSTATE_IND_Message();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setAffectedSignalingPoint(uint16_t pointCode);
  uint16_t getAffectedSignalingPoint() const;

  void setSignalingPointStatus(uint8_t signalingPointStatus);
  uint8_t getSignalingPointStatus() const;

  void setRemoteSCCPStatus(uint8_t remoteSCCPStatus);
  uint8_t getRemoteSCCPStatus() const;

  static const uint8_t SIGNALING_POINT_INACCESSIBLE = 1;
  static const uint8_t SIGNALING_POINT_CONGESTED = 2;
  static const uint8_t SIGNALING_POINT_ACCESSIBLE = 3;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint16_t _affectedSignalingPoint;
  uint8_t _signalingPointStatus;
  uint8_t _remoteSCCPStatus;

  static const uint32_t _MSG_CODE=0x06;
  enum { SET_REMOTE_SCCP_STATUS = 0x01 };
};

}}}

#endif
