#ifndef __SMPPDMPLX_SMPP_BINDREQUEST_HPP__
# define __SMPPDMPLX_SMPP_BINDREQUEST_HPP__ 1

# include "SMPP_message.hpp"
# include "SMPP_BindResponse.hpp"
# include <string>
# include <vector>

namespace smpp_dmplx {

class SMPP_BindRequest : public SMPP_message {
public:
  SMPP_BindRequest(uint32_t msgCode);
  virtual ~SMPP_BindRequest();

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;
  virtual void unmarshal(BufferedInputStream& buf);

  virtual std::string getSystemId() const;
  virtual void setSystemId(const std::string& arg);

  virtual std::string getPassword() const;
  virtual void setPassword(const std::string& arg);

  virtual std::auto_ptr<SMPP_BindResponse> prepareResponse(uint32_t status) const = 0;

  virtual std::string toString() const;
private:
  std::string _systemId, _password;
  std::vector<uint8_t> unimportantDataBuf;
};

}

#endif
