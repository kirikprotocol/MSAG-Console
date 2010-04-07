#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_ACTIVEACKMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_ACTIVEACKMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/M3uaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

class ActiveAckMessage : public common::AdaptationLayer_Message {
public:
  ActiveAckMessage()
  : common::AdaptationLayer_Message(_MSG_CODE) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "ASPAC ACK"; }

  void setTrafficModyType(const common::TLV_TrafficModeType& traffic_mode_type) { _trafficModeType = traffic_mode_type; }
  const common::TLV_TrafficModeType& getTrafficModeType() const;

  void setInfoString(const common::TLV_InfoString& info_string) { _infoString = info_string; }
  const common::TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  common::TLV_TrafficModeType _trafficModeType;
  common::TLV_InfoString _infoString;

  static const uint32_t _MSG_CODE = 0x0403;
};

}}}}}}

#endif
