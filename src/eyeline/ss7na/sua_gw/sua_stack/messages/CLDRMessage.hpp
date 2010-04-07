#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_CLDRMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_CLDRMESSAGE_HPP__

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDTMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class CLDRMessage : public common::AdaptationLayer_Message {
public:
  CLDRMessage()
  : common::AdaptationLayer_Message(_MSG_CODE)
  {
    setStreamNo(1);
  }

  CLDRMessage(const CLDTMessage& original_message, common::return_cause_value_t cause_value);

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "CLDR"; }

  void setSCCPCause(const TLV_SCCP_Cause& sccp_cause) { _sccpCause = sccp_cause; }
  const TLV_SCCP_Cause& getSCCPCause() const;

  void setSourceAddress(const TLV_SourceAddress& source_address) { _sourceAddress = source_address; }
  const TLV_SourceAddress& getSourceAddress() const;

  void setDestinationAddress(const TLV_DestinationAddress& destination_address) { _destinationAddress = destination_address; }
  const TLV_DestinationAddress& getDestinationAddress() const;

  void setSS7HopCount(const TLV_SS7HopCount& ss7_hop_count) { _hopCount = ss7_hop_count; }
  const TLV_SS7HopCount& getSS7HopCount() const;

  void setImportance(const TLV_Importance& importance) { _importance = importance; }
  const TLV_Importance& getImportance() const;

  void setMessagePriority(const TLV_MessagePriority& message_priority) {   _messagePriority = message_priority; }
  const TLV_MessagePriority& getMessagePriority() const;

  void setCorrelationId(const common::TLV_CorrelationId& correlation_id) { _correlationId = correlation_id; }
  const common::TLV_CorrelationId& getCorrelationId() const;

  void setSegmentation(const TLV_Segmentation& segmentation) { _segmentation = segmentation; }
  const TLV_Segmentation& getSegmentation() const;

  void setData(const TLV_Data& data) { _data = data; }
  const TLV_Data& getData() const;

protected:
  virtual uint32_t getLength() const;

private:
  TLV_SCCP_Cause _sccpCause;
  TLV_SourceAddress _sourceAddress;
  TLV_DestinationAddress _destinationAddress;
  TLV_SS7HopCount _hopCount;
  TLV_Importance _importance;
  TLV_MessagePriority _messagePriority;
  common::TLV_CorrelationId _correlationId;
  TLV_Segmentation _segmentation;
  TLV_Data _data;

  static const uint32_t _MSG_CODE = 0x0702;
};

}}}}}

#endif
