#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_UNBINDMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_UNBINDMESSAGE_HPP__

# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class UnbindMessage : public LibsccpMessage {
public:
  UnbindMessage();

  virtual size_t serialize(common::TP* result_buf) const;
  virtual size_t serialize(uint8_t* result_buf, size_t resul_buf_max_sz) const;

  virtual size_t deserialize(const common::TP& packet_buf);
  virtual size_t deserialize(const uint8_t* packet_buf, size_t packet_buf_sz);

  virtual std::string toString() const;

  const char* getMsgCodeTextDescription() const;
private:
  static const uint32_t _MSG_CODE=0x02;
};

}}}

#endif
