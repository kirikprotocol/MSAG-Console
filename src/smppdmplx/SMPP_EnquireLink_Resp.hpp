#ifndef __SMPPDMPLX_SMPP_ENQUIRELINK_RESP_HPP__
# define __SMPPDMPLX_SMPP_ENQUIRELINK_RESP_HPP__

# include <memory>
# include <smppdmplx/SMPP_message.hpp>

namespace smpp_dmplx {

class SMPP_EnquireLink_Resp : public SMPP_message {
public:
  SMPP_EnquireLink_Resp();
  virtual ~SMPP_EnquireLink_Resp();

  virtual std::auto_ptr<SMPP_message> clone(uint32_t msgCode, BufferedInputStream& buf) const;

  virtual std::auto_ptr<BufferedOutputStream> marshal() const;

  virtual void unmarshal(BufferedInputStream& buf);
};

}

#endif
