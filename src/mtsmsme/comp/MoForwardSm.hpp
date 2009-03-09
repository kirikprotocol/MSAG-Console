#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_MOFORWARDSM_HPP__
#define __SMSC_MTSMSME_COMP_MOFORWARDSM_HPP__

#include "mtsmsme/comp/Component.hpp"
#include "MO-ForwardSM-Arg.h"
#include "SM-RP-OA.h"
#include "SM-RP-DA.h"
#include "logger/Logger.h"
#include "mtsmsme/processor/util.hpp"
#include <string>
#include <vector>

namespace smsc{ namespace mtsmsme{ namespace comp{

using smsc::mtsmsme::comp::CompIF;
using smsc::logger::Logger;
using std::string;
using std::vector;

class MoForwardSmReq: public  CompIF {
  public:
    MoForwardSmReq(const string& _smsca, const string& _msisdn, const vector<unsigned char>& _ui );
    ~MoForwardSmReq();
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
private:
    MO_ForwardSM_Arg_t arg;
    OCTET_STRING_DECL(da,20);
    OCTET_STRING_DECL(oa, 20);
    OCTET_STRING_DECL(ui,200);
};
class MoForwardSmInd: public CompIF {
  public:
    MoForwardSmInd(Logger* logger);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    Logger* logger;
};
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }

#endif /* __SMSC_MTSMSME_COMP_MOFORWARDSM_HPP__ */
