#ifndef __SMPPDMPLX_SMPP_BINDRESPONSE_HPP__
# define __SMPPDMPLX_SMPP_BINDRESPONSE_HPP__

# include <string>
# include <vector>
# include <smppdmplx/SMPP_message.hpp>

namespace smpp_dmplx {

class SMPP_BindResponse : public SMPP_message {
public:
  SMPP_BindResponse(uint32_t msgCode);
  virtual ~SMPP_BindResponse();

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;
  virtual void unmarshal(BufferedInputStream& buf);

  virtual std::string getSystemId() const;
  virtual void setSystemId(const std::string& arg);

  virtual std::string toString() const;

  virtual std::auto_ptr<SMPP_BindResponse> makeCopy() const = 0;
private:
  std::string _systemId;
  std::vector<uint8_t> _unimportantDataBuf;
};

}

#endif
