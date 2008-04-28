#ifndef __SUA_COMMUNICATION_SUAMESSAGES_CLDTMESSAGE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_CLDTMESSAGE_HPP__ 1

# include <sua/communication/sua_messages/SUAMessage.hpp>

namespace sua_messages {

class CLDTMessage : public SUAMessage {
public:
  CLDTMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  void setProtocolClass(const TLV_ProtocolClass& protocolClass);
  const TLV_ProtocolClass& getProtocolClass() const;

  void setSourceAddress(const TLV_SourceAddress& sourceAddress);
  const TLV_SourceAddress& getSourceAddress() const;

  void setDestinationAddress(const TLV_DestinationAddress& destinationAddress);
  const TLV_DestinationAddress& getDestinationAddress() const;

  void setSequenceControl(const TLV_SequenceControl& sequenceControl);
  const TLV_SequenceControl& getSequenceControl() const;

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
  TLV_ProtocolClass _protocolClass;
  TLV_SequenceControl _sequenceControl;
  TLV_SS7HopCount _hopCount;
  TLV_Importance _importance;
  TLV_MessagePriority _messagePriority;
  TLV_CorrelationId _correlationId;
  TLV_Segmentation _segmentation;
  TLV_Data _data;

  static const uint32_t _MSG_CODE = 0x0701;
};

}

#endif
