#ifndef __SMPPDMPLX_SMPP_UNBIND_RESP_HPP__
# define __SMPPDMPLX_SMPP_UNBIND_RESP_HPP__ 1

#include "SMPP_message.hpp"
#include <memory>

namespace smpp_dmplx {

class SMPP_Unbind_Resp : public SMPP_message {
public:
  SMPP_Unbind_Resp();
  virtual ~SMPP_Unbind_Resp();

  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);
};

}

#endif
