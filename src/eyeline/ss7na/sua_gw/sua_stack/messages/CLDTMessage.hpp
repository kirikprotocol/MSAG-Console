#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_CLDTMESSAGE_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_CLDTMESSAGE_HPP__

# include "eyeline/ss7na/common/AdaptationLayer_Message.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class CLDTMessage : public common::AdaptationLayer_Message {
public:
  CLDTMessage()
  : common::AdaptationLayer_Message(_MSG_CODE)
  {
    setStreamNo(1);
  }

  virtual size_t serialize(common::TP* result_buf) const;

  virtual size_t deserialize(const common::TP& packet_buf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "CLDT"; }

  void setProtocolClass(const TLV_ProtocolClass& protocol_class) { _protocolClass = protocol_class; }
  const TLV_ProtocolClass& getProtocolClass() const;

  void setSourceAddress(const TLV_SourceAddress& source_address) { _sourceAddress = source_address; }
  const TLV_SourceAddress& getSourceAddress() const;

  void setDestinationAddress(const TLV_DestinationAddress& destination_address) { _destinationAddress = destination_address; }
  const TLV_DestinationAddress& getDestinationAddress() const;

  void setSequenceControl(const TLV_SequenceControl& sequence_control) { _sequenceControl = sequence_control; }
  const TLV_SequenceControl& getSequenceControl() const;

  void setSS7HopCount(const TLV_SS7HopCount& ss7_hop_count) { _hopCount = ss7_hop_count; }
  const TLV_SS7HopCount& getSS7HopCount() const;

  void setImportance(const TLV_Importance& importance) { _importance = importance; }
  const TLV_Importance& getImportance() const;

  void setMessagePriority(const TLV_MessagePriority& message_priority) { _messagePriority = message_priority; }
  const TLV_MessagePriority& getMessagePriority() const;

  void setCorrelationId(const common::TLV_CorrelationId& correlation_id) { _correlationId = correlation_id; }
  const common::TLV_CorrelationId& getCorrelationId() const;

  void setSegmentation(const TLV_Segmentation& segmentation) { _segmentation = segmentation; }
  const TLV_Segmentation& getSegmentation() const;
  bool isSetSegmentation() const { return _segmentation.isSetValue(); }

  void setData(const TLV_Data& data) { _data = data; }
  const TLV_Data& getData() const;

protected:
  virtual uint32_t getLength() const;

private:
  TLV_ProtocolClass _protocolClass;
  TLV_SourceAddress _sourceAddress;
  TLV_DestinationAddress _destinationAddress;
  TLV_SequenceControl _sequenceControl;
  TLV_SS7HopCount _hopCount;
  TLV_Importance _importance;
  TLV_MessagePriority _messagePriority;
  common::TLV_CorrelationId _correlationId;
  TLV_Segmentation _segmentation;
  TLV_Data _data;

  static const uint32_t _MSG_CODE = 0x0701;
};

}}}}}

#endif
