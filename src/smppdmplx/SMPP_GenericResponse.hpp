#ifndef __SMPPDMPLX_SMPP_GENERICRESPONSE_HPP__
# define __SMPPDMPLX_SMPP_GENERICRESPONSE_HPP__

# include <string>
# include <vector>
# include <smppdmplx/SMPP_message.hpp>

namespace smpp_dmplx {

class SMPP_GenericResponse : public SMPP_message {
public:
  explicit SMPP_GenericResponse(uint32_t msgCode=GENERIC_RESPONSE);
  virtual ~SMPP_GenericResponse();

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual uint32_t getCommandId() const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);

  virtual std::string toString() const;

  static const uint32_t GENERIC_RESPONSE = 0x8FFFFFFF;
private:
  std::vector<uint8_t> _unimportantDataBuf;
};

}

#endif
