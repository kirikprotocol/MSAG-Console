#ifndef __EYELINE_SUA_LIBSUA_MESSAGES_BINDMESSAGE_HPP__
# define __EYELINE_SUA_LIBSUA_MESSAGES_BINDMESSAGE_HPP__

# include <eyeline/sua/libsua/messages/LibsuaMessage.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class BindMessage : public LibsuaMessage {
public:
  BindMessage();

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const;

  std::string getAppId() const;
  void setAppId(const std::string& appid);

  uint8_t getProtocolVersion() const;

protected:
  uint32_t getLength() const;

private:
  char _appId[32+1];

  static const uint32_t _MSG_CODE=0x01;
  enum { PROTOCOL_VERSION = 0x01, VERSION_FIELD_SZ = 0x01 };
};

}}}

#endif
