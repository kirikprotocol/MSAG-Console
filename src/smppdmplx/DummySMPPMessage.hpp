#ifndef __SMPPDMPLX_DUMMYSMPPMESSAGE_HPP__
# define __SMPPDMPLX_DUMMYSMPPMESSAGE_HPP__ 

# include "SMPP_message.hpp"
# include <util/Exception.hpp>

namespace smpp_dmplx {

class DummySMPPMessage : public  SMPP_message {
public:
  DummySMPPMessage() : SMPP_message(0) {}
  //virtual bool checkMessageCodeEquality(uint32_t msgCode) const { return false; }
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const { return std::auto_ptr<SMPP_message>(new DummySMPPMessage()); }

  virtual uint32_t getCommandLength() const { throw smsc::util::Exception("DummySMPPMessage::getCommandLength::: message is invalid"); }
  virtual void setCommandLength(uint32_t arg) { throw smsc::util::Exception("DummySMPPMessage::setCommandLength::: message is invalid"); }

  virtual uint32_t getCommandId() const { throw smsc::util::Exception("DummySMPPMessage::getCommandId::: message is invalid"); }
  virtual void setCommandId(uint32_t arg) { throw smsc::util::Exception("DummySMPPMessage::setCommandId::: message is invalid"); }

  virtual uint32_t getCommandStatus() const { throw smsc::util::Exception("DummySMPPMessage::getCommandStatus::: message is invalid"); }
  virtual void setCommandStatus(uint32_t arg) { throw smsc::util::Exception("DummySMPPMessage::setCommandStatus::: message is invalid"); }

  virtual uint32_t getSequenceNumber() const { throw smsc::util::Exception("DummySMPPMessage::getSequenceNumber::: message is invalid"); }
  virtual void setSequenceNumber(uint32_t) { throw smsc::util::Exception("DummySMPPMessage::setSequenceNumber::: message is invalid"); }

  virtual std::string getSystemId() const { throw smsc::util::Exception("DummySMPPMessage::getSystemId::: message is invalid"); };
  virtual void setSystemId(const std::string& arg) { throw smsc::util::Exception("DummySMPPMessage::setSystemId::: message is invalid"); };

  virtual std::string getPassword() const { throw smsc::util::Exception("DummySMPPMessage::getPassword::: message is invalid"); };
  virtual void setPassword(const std::string& arg) { throw smsc::util::Exception("DummySMPPMessage::setPassword::: message is invalid"); };
};

}

#endif
