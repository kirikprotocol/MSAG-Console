#ifndef __SMPPDMPLX_SMPP_BINDTRANSMITTER_HPP__
# define __SMPPDMPLX_SMPP_BINDTRANSMITTER_HPP__ 1

#include "SMPP_message.hpp"
#include "SMPP_BindRequest.hpp"
#include <string>
#include <vector>
#include <memory>

namespace smpp_dmplx {

class SMPP_BindTransmitter : public SMPP_BindRequest {
public:
  SMPP_BindTransmitter();
  virtual ~SMPP_BindTransmitter();
  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;
  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  std::auto_ptr<SMPP_BindResponse> prepareResponse(uint32_t status) const;
};

}

#endif
