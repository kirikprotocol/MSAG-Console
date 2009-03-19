#ifndef __EYELINE_SUA_LIBSUA_MESSAGES_UNBINDMESSAGE_HPP__
# define __EYELINE_SUA_LIBSUA_MESSAGES_UNBINDMESSAGE_HPP__

# include <eyeline/sua/libsua/messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class UnbindMessage : public LibsuaMessage {
public:
  UnbindMessage();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  const char* getMsgCodeTextDescription() const;
private:
  static const uint32_t _MSG_CODE=0x02;
};

}}}

#endif
