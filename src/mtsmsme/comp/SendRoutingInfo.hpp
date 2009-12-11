#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_SENDROUTINGINFO_HPP_
#define __SMSC_MTSMSME_COMP_SENDROUTINGINFO_HPP_

#include "mtsmsme/comp/Component.hpp"
#include "SendRoutingInfoArg.h"

namespace smsc{ namespace mtsmsme{ namespace comp{
class SendRoutingInfoReq: public  CompIF {
  public:
    SendRoutingInfoReq();
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    SendRoutingInfoArg_t arg;
};
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
#endif /* __SMSC_MTSMSME_COMP_SENDROUTINGINFO_HPP_ */
