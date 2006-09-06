#ifndef __SMPPDMPLX_SMPP_BINDRECEIVER_RESP_HPP__
# define __SMPPDMPLX_SMPP_BINDRECEIVER_RESP_HPP__ 1

#include "SMPP_message.hpp"
#include "SMPP_BindResponse.hpp"
#include <string>
#include <vector>
#include <memory>

namespace smpp_dmplx {

class SMPP_BindReceiver_Resp : public SMPP_BindResponse {
public:
  SMPP_BindReceiver_Resp();
  virtual ~SMPP_BindReceiver_Resp();
  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;
  virtual std::auto_ptr<SMPP_BindResponse> makeCopy() const;
};

}

#endif
