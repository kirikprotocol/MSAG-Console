#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_NPCSTATEMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_NPCSTATEMESSAGE_HPP__

# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class N_PCSTATE_IND_Message : public LibsccpMessage {
public:
  N_PCSTATE_IND_Message();

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* result_buf, size_t result_buf_max_sz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packet_buf, size_t packet_buf_sz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "N_PCSTATE_IND_Message"; }

  void setAffectedSignalingPoint(uint32_t point_code) { _affectedSignalingPoint = point_code; }
  uint32_t getAffectedSignalingPoint() const { return _affectedSignalingPoint; }

  enum signalling_point_status_e {
    SIGNALING_POINT_INACCESSIBLE = 1,
    SIGNALING_POINT_CONGESTED = 2,
    SIGNALING_POINT_ACCESSIBLE = 3 };

  void setSignalingPointStatus(signalling_point_status_e signaling_point_status) { _signalingPointStatus = uint8_t(signaling_point_status); }
  signalling_point_status_e getSignalingPointStatus() const { return signalling_point_status_e(_signalingPointStatus); }

  enum remote_sccp_status_e {
    REMOTE_SCCP_UNAVAILABLE = 0,
    REMOTE_SCCP_UNEQUIPPED = 1,
    REMOTE_SCCP_INACCESSIBLE = 2,
    REMOTE_SCCP_AVAILABLE = 3,
    REMOTE_SCCP_CONGESTED = 4
  };
  void setRemoteSCCPStatus(remote_sccp_status_e remote_SCCP_status);
  remote_sccp_status_e getRemoteSCCPStatus() const;

protected:
  virtual uint32_t getLength() const;

private:
  uint8_t _fieldsMask;
  uint32_t _affectedSignalingPoint;
  uint8_t _signalingPointStatus;
  uint8_t _remoteSCCPStatus;

  static const uint32_t _MSG_CODE=0x06;
  enum { SET_REMOTE_SCCP_STATUS = 0x01 };
};

}}}

#endif
