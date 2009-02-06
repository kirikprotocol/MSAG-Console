#ifndef __SMPPDMPLX_SMPP_MESSAGE_HPP__
# define __SMPPDMPLX_SMPP_MESSAGE_HPP__

# include <sys/types.h>
# include <memory>

# include <smppdmplx/BufferedInputStream.hpp>
# include <smppdmplx/BufferedOutputStream.hpp>

namespace smpp_dmplx {

class SMPP_message {
public:
  SMPP_message(uint32_t msgCode) : _commandLength(SMPP_HEADER_SZ), _commandId(msgCode), _commandStatus(0), _sequenceNumber(0) {}
  virtual ~SMPP_message() {}

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const = 0;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;
  virtual void unmarshal(BufferedInputStream& buf);

  virtual uint32_t getCommandLength() const;
  virtual void setCommandLength(uint32_t arg);

  virtual uint32_t getCommandId() const;
  virtual void setCommandId(uint32_t arg);

  virtual uint32_t getCommandStatus() const;
  virtual void setCommandStatus(uint32_t arg);

  virtual uint32_t getSequenceNumber() const;
  virtual void setSequenceNumber(uint32_t);

  virtual std::string toString() const;

  static const uint32_t BIND_RECEIVER = 0x00000001;
  static const uint32_t BIND_RECEIVER_RESP = 0x80000001;
  static const uint32_t BIND_TRANSMITTER = 0x00000002;
  static const uint32_t BIND_TRANSMITTER_RESP = 0x80000002;
  static const uint32_t BIND_TRANSCEIVER = 0x00000009;
  static const uint32_t BIND_TRANSCEIVER_RESP = 0x80000009;
  static const uint32_t UNBIND = 0x00000006;
  static const uint32_t UNBIND_RESP = 0x80000006;
  static const uint32_t ENQUIRE_LINK = 0x00000015;
  static const uint32_t ENQUIRE_LINK_RESP = 0x80000015;
  static const uint32_t SMPP_HEADER_SZ = 16;
  static const uint32_t MAX_SMPP_MESSAGE_SIZE = 70*1024;
private:
  uint32_t _commandLength, _commandId, _commandStatus, _sequenceNumber;
};

}
#endif
