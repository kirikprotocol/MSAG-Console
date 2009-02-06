#ifndef __SMPPDMPLX_SMPP_BINDRECEIVER_RESP_HPP__
# define __SMPPDMPLX_SMPP_BINDRECEIVER_RESP_HPP__

#include <string>
#include <vector>
#include <memory>

#include <smppdmplx/SMPP_message.hpp>
#include <smppdmplx/SMPP_BindResponse.hpp>

namespace smpp_dmplx {

class SMPP_BindReceiver_Resp : public SMPP_BindResponse {
public:
  SMPP_BindReceiver_Resp();
  virtual ~SMPP_BindReceiver_Resp();
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;
  virtual std::auto_ptr<SMPP_BindResponse> makeCopy() const;
};

}

#endif
