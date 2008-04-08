#ifndef __SUA_COMMUNICATION_SUAMESSAGES_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_HPP__

# include <sua/communication/Message.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace sua_messages {

class SUAMessage : public communication::Message {
public:
  typedef uint32_t msg_code_t;

  SUAMessage(msg_code_t msgCode);
  // after call to this method will be completed
  // the resultBuf->packetBuf will contain all message data,
  // i.e. message header(4 bytes) + message length(4 bytes) + message body
  virtual size_t serialize(communication::TP* resultBuf) const;

  // packetBuf.packetBody contains all message data,
  // i.e. message header(4 bytes) + message length(4 bytes) + message body
  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  virtual uint16_t getStreamNo() const;

  virtual bool getOrderDelivering() const;

  virtual msg_code_t getMsgCode() const;

  static void registerMessageCode(msg_code_t msgCode);
  static unsigned int getMessageIndex(msg_code_t msgCode);

  void setStreamNo(uint16_t streamNo);

  enum { HEADER_SIZE = 8, MESSAGE_LENGTH_SIZE = 4, MESSAGE_PREAMBLE_SIZE = 4 };
  typedef uint8_t message_header_t[HEADER_SIZE];

  static size_t makeHeader(message_header_t messageHeader, msg_code_t suaMsgCode, uint32_t suaMessageBodySize);

  virtual const TLV_Address& getDestinationAddress() const;

  virtual const TLV_Address& getSourceAddress() const;

protected:
  TLV_SourceAddress _sourceAddress;
  TLV_DestinationAddress _destinationAddress;

private:
  enum { MAX_MSG_CLASS_VALUE = 9, MAX_MSG_TYPE_VALUE = 11 };
  static unsigned int _msgClassTypeToMessageIdx[MAX_MSG_CLASS_VALUE+1][MAX_MSG_TYPE_VALUE+1];

  static const uint8_t RESERVED_FIELD = 0x00, PROTOCOL_VERSION = 0x01;

  msg_code_t _msgCode;
  uint16_t _streamNo;
};

} // namespace sua_stack



#endif /* SUAMESSAGE_HPP_HEADER_INCLUDED_B87B46E4 */
