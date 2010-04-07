#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_ACTIVEMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_ACTIVEMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class ActiveMessage : public common::AdaptationLayer_Message {
public:
  ActiveMessage()
  : common::AdaptationLayer_Message(_MSG_CODE) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "ASP ACTIVE"; }

  void setTrafficModyType(const common::TLV_TrafficModeType& traffic_mode_type) { _trafficModeType = traffic_mode_type; }
  const common::TLV_TrafficModeType& getTrafficModeType() const;

  void setTIDLabel(const TLV_TIDLabel& tid_label) { _tidLabel = tid_label; }
  const TLV_TIDLabel& getTIDLabel() const;

  void setDRNLabel(const TLV_DRNLabel& drn_label) { _drnLabel = drn_label; }
  const TLV_DRNLabel& getDRNLabel() const;

  void setInfoString(const common::TLV_InfoString& info_string) { _infoString = info_string; }
  const common::TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  common::TLV_TrafficModeType _trafficModeType;
  TLV_TIDLabel _tidLabel;
  TLV_DRNLabel _drnLabel;
  common::TLV_InfoString _infoString;

  static const uint32_t _MSG_CODE = 0x0401;
};

}}}}}

#endif
