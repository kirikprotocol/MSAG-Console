#ifndef __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_UNBINDMESSAGE_HPP__
# define __EYELINE_SUA_COMMUNICATION_LIBSUAMESSAGES_UNBINDMESSAGE_HPP__

# include <string>
# include <eyeline/sua/communication/libsua_messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace communication {
namespace libsua_messages {

class UnbindMessage : public LibsuaMessage {
public:
  UnbindMessage();

  virtual size_t serialize(communication::TP* resultBuf) const;

  virtual size_t deserialize(const communication::TP& packetBuf);

  virtual std::string toString() const;

  const char* getMsgCodeTextDescription() const;
private:
  static const uint32_t _MSG_CODE=0x02;
};

}}}}

#endif
