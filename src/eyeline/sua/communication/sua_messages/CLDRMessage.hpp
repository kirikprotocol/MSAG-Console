#ifndef __EYELINE_SUA_COMMUNICATION_SUAMESSAGES_CLDRMESSAGE_HPP__
# define __EYELINE_SUA_COMMUNICATION_SUAMESSAGES_CLDRMESSAGE_HPP__

# include <eyeline/sua/communication/sua_messages/SUAMessage.hpp>
# include <eyeline/sua/communication/sua_messages/CLDTMessage.hpp>
# include <eyeline/sua/communication/types.hpp>

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

class CLDRMessage : public SUAMessage {
public:
  CLDRMessage();
  CLDRMessage(const CLDTMessage& originalMessage, communication::return_cause_value_t returnCause);

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setSCCPCause(const TLV_SCCP_Cause& sccpCause);
  const TLV_SCCP_Cause& getSCCPCause() const;

  void setSourceAddress(const TLV_SourceAddress& sourceAddress);
  const TLV_SourceAddress& getSourceAddress() const;

  void setDestinationAddress(const TLV_DestinationAddress& destinationAddress);
  const TLV_DestinationAddress& getDestinationAddress() const;

  void setSS7HopCount(const TLV_SS7HopCount& ss7HopCount);
  const TLV_SS7HopCount& getSS7HopCount() const;

  void setImportance(const TLV_Importance& importance);
  const TLV_Importance& getImportance() const;

  void setMessagePriority(const TLV_MessagePriority& messagePriority);
  const TLV_MessagePriority& getMessagePriority() const;

  void setCorrelationId(const TLV_CorrelationId& correlationId);
  const TLV_CorrelationId& getCorrelationId() const;

  void setSegmentation(const TLV_Segmentation& segmentation);
  const TLV_Segmentation& getSegmentation() const;

  void setData(const TLV_Data& data);
  const TLV_Data& getData() const;

protected:
  virtual uint32_t getLength() const;

private:
  TLV_SCCP_Cause _sccpCause;
  TLV_SS7HopCount _hopCount;
  TLV_Importance _importance;
  TLV_MessagePriority _messagePriority;
  TLV_CorrelationId _correlationId;
  TLV_Segmentation _segmentation;
  TLV_Data _data;

  static const uint32_t _MSG_CODE = 0x0702;
};

}}}}

#endif
