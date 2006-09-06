#ifndef __SMPPDMPLX_SMPP_GENERICREQUEST_HPP__
# define __SMPPDMPLX_SMPP_GENERICREQUEST_HPP__ 1

# include "SMPP_message.hpp"
# include <string>
# include <vector>

namespace smpp_dmplx {

class SMPP_GenericRequest : public SMPP_message {
public:
  SMPP_GenericRequest(uint32_t msgCode);

  virtual ~SMPP_GenericRequest();

  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual uint32_t getCommandId() const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);

  virtual std::string toString() const;

  static const uint32_t GENERIC_REQUEST;
private:
  std::vector<uint8_t> unimportantDataBuf;
};

}

#endif
