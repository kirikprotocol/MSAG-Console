#ifndef __SUA_LIBSUA_MESSAGES_LIBSUAMESSAGE_HPP__
# define __SUA_LIBSUA_MESSAGES_LIBSUAMESSAGE_HPP__

# include <sys/types.h>
# include <string>

namespace libsua {

struct variable_data_t {
  variable_data_t(const uint8_t* aData, uint8_t aDataLen)
    : data(aData), dataLen(aDataLen) {}
  const uint8_t* data;
  uint8_t dataLen;
};

class LibsuaMessage {
public:
  LibsuaMessage(uint32_t msgCode);
  virtual ~LibsuaMessage() {}

  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual uint32_t getMsgCode() const;

  virtual const char* getMsgCodeTextDescription() const = 0;
protected:
  virtual uint32_t getLength() const;

  void setLength(uint32_t msgLen);

  static const size_t _MSGCODE_SZ = sizeof(uint32_t), _MSGLEN_SZ = sizeof(uint32_t);
private:
  uint32_t _msgLen;
  uint32_t _msgCode;
};

}

#endif
