#ifndef __SMPPDMPLX_SMPP_BINDTRANSCEIVER_HPP__
# define __SMPPDMPLX_SMPP_BINDTRANSCEIVER_HPP__

#include <string>
#include <vector>
#include <memory>

#include <smppdmplx/SMPP_message.hpp>
#include <smppdmplx/SMPP_BindRequest.hpp>
#include <smppdmplx/SMPP_BindResponse.hpp>

namespace smpp_dmplx {

class SMPP_BindTransceiver : public SMPP_BindRequest {
public:
  SMPP_BindTransceiver();
  virtual ~SMPP_BindTransceiver();
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  std::auto_ptr<SMPP_BindResponse> prepareResponse(uint32_t status) const;
};

}

#endif
