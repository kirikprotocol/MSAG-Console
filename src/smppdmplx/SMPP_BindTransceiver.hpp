#ifndef __SMPPDMPLX_SMPP_BINDTRANSCEIVER_HPP__
# define __SMPPDMPLX_SMPP_BINDTRANSCEIVER_HPP__ 1

#include "SMPP_message.hpp"
#include "SMPP_BindRequest.hpp"
#include "SMPP_BindResponse.hpp"
#include <string>
#include <vector>
#include <memory>

namespace smpp_dmplx {

class SMPP_BindTransceiver : public SMPP_BindRequest {
public:
  SMPP_BindTransceiver();
  virtual ~SMPP_BindTransceiver();
  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  std::auto_ptr<SMPP_BindResponse> prepareResponse(uint32_t status) const;
};

}

#endif
