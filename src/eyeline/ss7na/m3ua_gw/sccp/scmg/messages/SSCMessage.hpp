#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SSCMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGES_SSCMESSAGE_HPP__

# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SCMGMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {
namespace messages {

enum sccp_congestion_level_e { NoCogestion = 0, CongestionLevel_1, CongestionLevel_2,
                               CongestionLevel_3, CongestionLevel_4, CongestionLevel_5,
                               CongestionLevel_6, CongestionLevel_7, CongestionLevel_8 };

class SSCMessage : public SCMGMessage {
public:
  SSCMessage()
  : SCMGMessage(_MSG_CODE), _isSetCongestionLevel(false)
  {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "SSC"; }

  void setCongestionLevel(sccp_congestion_level_e cong_level) {
    _congestionLevel = cong_level; _isSetCongestionLevel = true;
  }
  sccp_congestion_level_e getCongestionLevel() const {
    if ( _isSetCongestionLevel )
      return _congestionLevel;
    else
      throw utilx::FieldNotSetException("SSCMessage::getCongestionLevel::: mandatory field isn't set");
  }

private:
  static const msg_code_t _MSG_CODE = 0x06;
  sccp_congestion_level_e _congestionLevel;
  bool _isSetCongestionLevel;
};

}}}}}}

#endif
