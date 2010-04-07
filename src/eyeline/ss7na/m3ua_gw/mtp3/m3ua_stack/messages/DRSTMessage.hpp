#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_DRSTMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_DRSTMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/M3uaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

class DRSTMessage : public common::AdaptationLayer_Message {
public:
  DRSTMessage ()
  : common::AdaptationLayer_Message(_MSG_CODE) {}

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "DRST"; }

  void setNetworkAppearance(const TLV_NetworkAppearance& network_appearance) { _networkAppearance = network_appearance; }
  const TLV_NetworkAppearance& getNetworkAppearance() const;

  void setAffectedPointCode(const common::TLV_AffectedPointCode& affected_point_code) { _affectedPointCode = affected_point_code; }
  common::TLV_AffectedPointCode getAffectedPointCode() const;

  void setInfoString(const common::TLV_InfoString& info_string) { _infoString = info_string; }
  const common::TLV_InfoString& getInfoString() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0206;

  TLV_NetworkAppearance _networkAppearance;
  common::TLV_AffectedPointCode _affectedPointCode;
  common::TLV_InfoString _infoString;
};

}}}}}}

#endif
