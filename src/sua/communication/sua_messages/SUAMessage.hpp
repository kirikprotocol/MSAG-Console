#ifndef __EYELINE_SUA_COMMUNICATION_SUAMESSAGES_HPP__
# define __EYELINE_SUA_COMMUNICATION_SUAMESSAGES_HPP__

# include <eyeline/sua/communication/Message.hpp>
# include <eyeline/sua/communication/sua_messages/SuaTLV.hpp>

namespace eyeline {
namespace sua {
namespace communication {
namespace sua_messages {

class SUAMessage : public communication::Message {
public:
  typedef uint32_t msg_code_t;

  typedef enum { SUA_MANAGEMENT_MESSAGES=0, SUA_SIGNALING_NETWORK_MANAGEMENT_MESSAGES=2,
                 ASPSM_MESSAGES=3, ASPTM_MESSAGES=4, CONNECTIONLESS_MESSAGES=7,
                 CONNECTION_ORIENTED_MESSAGES=8, RKM_MESSAGES=9 } msg_class_t;

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

  unsigned int getMessageClass() const {
    return (getMsgCode() & 0x0000FF00) >> 8;
  }

  bool isSetRoutingContext() const;

  void setRoutingContext(const TLV_RoutingContext& routingContext);

  void updateRoutingContext(const TLV_RoutingContext& routingContext) const;

  const TLV_RoutingContext& getRoutingContext() const;

protected:
  mutable TLV_RoutingContext _routingContext;
  TLV_SourceAddress _sourceAddress;
  TLV_DestinationAddress _destinationAddress;

private:
  enum { MAX_MSG_CLASS_VALUE = 9, MAX_MSG_TYPE_VALUE = 11 };
  static unsigned int _msgClassTypeToMessageIdx[MAX_MSG_CLASS_VALUE+1][MAX_MSG_TYPE_VALUE+1];

  static const uint8_t RESERVED_FIELD = 0x00, PROTOCOL_VERSION = 0x01;

  msg_code_t _msgCode;
  uint16_t _streamNo;
};

}}}}

#endif
