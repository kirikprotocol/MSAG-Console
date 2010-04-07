#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_DATAMESSAGE_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_DATAMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/M3uaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

class DATAMessage : public common::AdaptationLayer_Message {
public:
  DATAMessage()
  : common::AdaptationLayer_Message(_MSG_CODE) {
    setStreamNo(1);
  }

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "DATA"; }

  void setNetworkAppearance(const TLV_NetworkAppearance& network_appearance) { _networkAppearance = network_appearance; }
  const TLV_NetworkAppearance& getNetworkAppearance() const;

  void setRoutingConext(const common::TLV_RoutingContext& routing_ctx) { _routingContext = routing_ctx; }
  const common::TLV_RoutingContext& getRoutingContext() const;

  void setProtocolData(TLV_ProtocolData& protocol_data) { _protocolData = protocol_data; }
  const TLV_ProtocolData& getProtocolData() const;

  void setCorrelationId(const common::TLV_CorrelationId& correlation_id) { _correlationId = correlation_id; }
  const common::TLV_CorrelationId& getCorrelationId() const;

protected:
  virtual uint32_t getLength() const;

private:
  static const uint32_t _MSG_CODE = 0x0101;

  TLV_NetworkAppearance _networkAppearance;
  common::TLV_RoutingContext _routingContext;
  TLV_ProtocolData _protocolData;
  common::TLV_CorrelationId _correlationId;
};

}}}}}}

#endif
