#ifndef __SMPPDMPLX_SMPP_BINDRECEIVER_HPP__
# define __SMPPDMPLX_SMPP_BINDRECEIVER_HPP__

#include <string>
#include <vector>
#include <memory>

#include <smppdmplx/SMPP_message.hpp>
#include <smppdmplx/SMPP_BindRequest.hpp>
#include <smppdmplx/SMPP_BindResponse.hpp>

namespace smpp_dmplx {

class SMPP_BindReceiver : public SMPP_BindRequest {
public:
  SMPP_BindReceiver();
  virtual ~SMPP_BindReceiver();
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  std::auto_ptr<SMPP_BindResponse> prepareResponse(uint32_t status) const;
};

}

#endif
