#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_SENDROUTINGINFOFORSM_HPP__
#define __SMSC_MTSMSME_COMP_SENDROUTINGINFOFORSM_HPP__

//#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/comp/Component.hpp"
#include "mtsmsme/processor/util.hpp"
#include "RoutingInfoForSM-Arg.h"
#include <string>

namespace smsc{ namespace mtsmsme{ namespace comp{

using smsc::mtsmsme::comp::CompIF;
using std::string;

class SendRoutingInfoForSMReq: public  CompIF {
  public:
    SendRoutingInfoForSMReq(const string& msisdn, bool force, const string& sca );
    ~SendRoutingInfoForSMReq();
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
private:
    RoutingInfoForSM_Arg_t arg;
    OCTET_STRING_DECL(_msisdn,20);
    OCTET_STRING_DECL(_sca, 20);
};

/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }

#endif /* __SMSC_MTSMSME_COMP_SENDROUTINGINFOFORSM_HPP__ */
