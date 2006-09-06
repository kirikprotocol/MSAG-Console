#ifndef __SMPPDMPLX_SMPP_UNBIND_HPP__
# define __SMPPDMPLX_SMPP_UNBIND_HPP__ 1

#include "SMPP_message.hpp"
#include <memory>

#include "SMPP_Unbind_Resp.hpp"

namespace smpp_dmplx {

class SMPP_Unbind : public SMPP_message {
public:
  SMPP_Unbind();
  virtual ~SMPP_Unbind();

  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);

  std::auto_ptr<SMPP_Unbind_Resp> prepareResponse(uint32_t status) const;
};

}

#endif
