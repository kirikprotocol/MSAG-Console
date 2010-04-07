#ifndef __EYELINE_SS7NA_COMMON_ADAPTATIONLAYERMESSAGES_HPP__
# define __EYELINE_SS7NA_COMMON_ADAPTATIONLAYERMESSAGES_HPP__

# include "eyeline/ss7na/common/Message.hpp"
# include "eyeline/ss7na/common/AdaptationLayer_TLV.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class AdaptationLayer_Message : public Message {
public:
  typedef uint32_t msg_code_t;

  typedef enum { SUA_MANAGEMENT_MESSAGES=0, SUA_SIGNALING_NETWORK_MANAGEMENT_MESSAGES=2,
                 ASPSM_MESSAGES=3, ASPTM_MESSAGES=4, CONNECTIONLESS_MESSAGES=7,
                 CONNECTION_ORIENTED_MESSAGES=8, RKM_MESSAGES=9 } msg_class_t;

  AdaptationLayer_Message(msg_code_t msg_code)
  : _msgCode(msg_code), _streamNo(0) {}

  // after call to this method will be completed
  // the resultBuf->packetBuf will contain all message data,
  // i.e. message header(4 bytes) + message length(4 bytes) + message body
  virtual size_t serialize(TP* result_buf) const;

  // packetBuf.packetBody contains all message data,
  // i.e. message header(4 bytes) + message length(4 bytes) + message body
  virtual size_t deserialize(const TP& packet_buf);

  virtual std::string toString() const;

  virtual uint16_t getStreamNo() const { return _streamNo; }
  void setStreamNo(uint16_t stream_no) { _streamNo = stream_no; }

  virtual bool getOrderDelivering() const { return false; }

  virtual msg_code_t getMsgCode() const { return _msgCode; }

  enum { HEADER_SIZE = 8, MESSAGE_LENGTH_SIZE = 4, MESSAGE_PREAMBLE_SIZE = 4 };
  typedef uint8_t message_header_t[HEADER_SIZE];

  size_t makeHeader(message_header_t message_header, msg_code_t sua_msg_code,
                    uint32_t sua_message_body_size) const;

  unsigned int getMessageClass() const {
    return (getMsgCode() & 0x0000FF00) >> 8;
  }

  bool isSetRoutingContext() const { return _routingContext.isSetValue(); }

  void setRoutingContext(const TLV_RoutingContext& routing_context) { _routingContext = routing_context; }

  void updateRoutingContext(const TLV_RoutingContext& routing_context) const { _routingContext = routing_context; }

  const TLV_RoutingContext& getRoutingContext() const;

protected:
  mutable TLV_RoutingContext _routingContext;

private:
  enum magic_values_e { RESERVED_FIELD = 0x00, PROTOCOL_VERSION = 0x01 };
  msg_code_t _msgCode;
  uint16_t _streamNo;
};

}}}

#endif
