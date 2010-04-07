#ifndef __EYELINE_SS7NA_LIBSCCP_MESSAGES_BINDMESSAGE_HPP__
# define __EYELINE_SS7NA_LIBSCCP_MESSAGES_BINDMESSAGE_HPP__

# include "eyeline/ss7na/libsccp/messages/LibsccpMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class BindMessage : public LibsccpMessage {
public:
  BindMessage();

  virtual size_t serialize(common::TP* resultBuf) const;
  virtual size_t serialize(uint8_t* resultBuf, size_t resultBufMaxSz) const;

  virtual size_t deserialize(const common::TP& packetBuf);
  virtual size_t deserialize(const uint8_t* packetBuf, size_t packetBufSz);

  virtual std::string toString() const;

  virtual const char* getMsgCodeTextDescription() const { return "BIND"; }

  std::string getAppId() const { return _appId; }
  void setAppId(const std::string& appid);

  uint8_t getProtocolVersion() const { return PROTOCOL_VERSION; }

  uint8_t getNumberOfSSN() const { return _numberOfSSN; }
  // return pointer to array of ssn.
  const uint8_t* getSSNList() const {
    if ( _numberOfSSN) return _ssn;
    else return NULL;
  }
  void setSSN(const uint8_t* ssn_list, uint8_t num_of_ssn) {
    memcpy(_ssn, ssn_list, num_of_ssn);
    _numberOfSSN = num_of_ssn;
  }

protected:
  uint32_t getLength() const;

private:
  uint8_t _numberOfSSN;
  char _appId[32+1];
  uint8_t _ssn[256];

  static const uint32_t _MSG_CODE=0x01;
  enum { PROTOCOL_VERSION = 0x02, VERSION_FIELD_SZ = 0x01 };
};

}}}

#endif
