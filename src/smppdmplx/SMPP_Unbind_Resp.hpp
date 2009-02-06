#ifndef __SMPPDMPLX_SMPP_UNBIND_RESP_HPP__
# define __SMPPDMPLX_SMPP_UNBIND_RESP_HPP__

#include <memory>
#include <smppdmplx/SMPP_message.hpp>

namespace smpp_dmplx {

class SMPP_Unbind_Resp : public SMPP_message {
public:
  SMPP_Unbind_Resp();
  virtual ~SMPP_Unbind_Resp();

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);
};

}

#endif
