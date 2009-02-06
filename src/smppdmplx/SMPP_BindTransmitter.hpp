#ifndef __SMPPDMPLX_SMPP_BINDTRANSMITTER_HPP__
# define __SMPPDMPLX_SMPP_BINDTRANSMITTER_HPP__

#include <string>
#include <vector>
#include <memory>

#include <smppdmplx/SMPP_message.hpp>
#include <smppdmplx/SMPP_BindRequest.hpp>

namespace smpp_dmplx {

class SMPP_BindTransmitter : public SMPP_BindRequest {
public:
  SMPP_BindTransmitter();
  virtual ~SMPP_BindTransmitter();
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  std::auto_ptr<SMPP_BindResponse> prepareResponse(uint32_t status) const;
};

}

#endif
