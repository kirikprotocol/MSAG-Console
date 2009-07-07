#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_QUERYSMRESP_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_QUERYSMRESP_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/load_balancer/protocols/smpp/QuerySm.hpp"
# include "eyeline/load_balancer/protocols/smpp/FastParsableSmppMessage.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class QuerySmResp : public FastParsableSmppMessage {
public:
  QuerySmResp();
  QuerySmResp(const QuerySm& querySm, uint32_t status);

  virtual size_t serialize(io_subsystem::Packet* packet) const;

  virtual std::string toString() const;

  uint64_t getMessageId() const;

  static const uint32_t _MSGCODE = 0x80000003;

protected:
  virtual uint32_t calculateCommandBodyLength() const;

  virtual void generateSerializationException() const {
    throw utilx::SerializationException("QuerySmResp::serialize::: message body wasn't set");
  }

  void setMessageId(uint64_t messageId);
private:
  uint64_t _messageId;
  char _messageIdCStr[24];
  bool _isSetMessageId, _isItTransitMessage;
  enum { LENGTH_OF_NULL_VALUE_FINAL_DATE_FIELD = 1, MESSAGE_STATE_FIELD_LENGTH =1, ERROR_CODE_FIELD_LENGTH = 1 };
};

}}}}

#endif
