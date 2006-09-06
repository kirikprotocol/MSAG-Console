#ifndef __SMPPDMPLX_SMPP_MESSAGE_HPP__
# define __SMPPDMPLX_SMPP_MESSAGE_HPP__ 1

# include <sys/types.h>
# include <memory>

# include "BufferedInputStream.hpp"
# include "BufferedOutputStream.hpp"

namespace smpp_dmplx {

class SMPP_message {
public:
  SMPP_message(uint32_t msgCode) : _commandLength(SMPP_HEADER_SZ), _commandId(msgCode), _commandStatus(0), _sequenceNumber(0) {}
  virtual ~SMPP_message();

  virtual bool checkMessageCodeEquality(uint32_t msgCode) const = 0;
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

  static const uint32_t BIND_RECEIVER;
  static const uint32_t BIND_RECEIVER_RESP;
  static const uint32_t BIND_TRANSMITTER;
  static const uint32_t BIND_TRANSMITTER_RESP;
  static const uint32_t BIND_TRANSCEIVER;
  static const uint32_t BIND_TRANSCEIVER_RESP;
  static const uint32_t UNBIND;
  static const uint32_t UNBIND_RESP;
  static const uint32_t ENQUIRE_LINK;
  static const uint32_t ENQUIRE_LINK_RESP;
  static const uint32_t SMPP_HEADER_SZ;
  static const uint32_t MAX_SMPP_MESSAGE_SIZE;
private:
  uint32_t _commandLength, _commandId, _commandStatus, _sequenceNumber;
};

}
#endif
