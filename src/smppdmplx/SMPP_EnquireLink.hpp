#ifndef __SMPPDMPLX_SMPP_ENQUIRELINK_HPP__
# define __SMPPDMPLX_SMPP_ENQUIRELINK_HPP__ 1

#include "SMPP_message.hpp"
#include <memory>

#include "SMPP_EnquireLink_Resp.hpp"

namespace smpp_dmplx {

class SMPP_EnquireLink : public SMPP_message {
public:
  SMPP_EnquireLink();
  virtual ~SMPP_EnquireLink();

  virtual bool checkMessageCodeEquality(uint32_t msgCode) const;

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);

  std::auto_ptr<SMPP_EnquireLink_Resp> prepareResponse(uint32_t status) const;
};

}

#endif
