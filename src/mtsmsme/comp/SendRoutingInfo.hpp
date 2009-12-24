#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_SENDROUTINGINFO_HPP_
#define __SMSC_MTSMSME_COMP_SENDROUTINGINFO_HPP_

#include "mtsmsme/comp/Component.hpp"
#include "mtsmsme/processor/util.hpp"
#include "logger/Logger.h"
#include "SendRoutingInfoArg.h"
#include "SendRoutingInfoRes.h"
#include "SendRoutingInfoArg-v2.h"
#include "SendRoutingInfoRes-v2.h"
#include <string>

namespace smsc{ namespace mtsmsme{ namespace comp{
using std::string;
using smsc::logger::Logger;

class SendRoutingInfoReq: public  CompIF {
  public:
    SendRoutingInfoReq(const string& msisdn, const string& gmsc);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    SendRoutingInfoArg_t arg;
    OCTET_STRING_DECL(_msisdn,20);
    OCTET_STRING_DECL(_gmsc,20);
};
class SendRoutingInfoReqV2: public  CompIF {
  public:
    SendRoutingInfoReqV2(const string& msisdn, const string& gmsc);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    SendRoutingInfoArg_v2 arg;
    OCTET_STRING_DECL(_msisdn,20);
};
class SendRoutingInfoConf: public  CompIF {
  public:
    SendRoutingInfoConf(Logger* logger);
    char* getMSRN();
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    Logger* logger;
    char _msrn[20];
};
class SendRoutingInfoConfV2: public  CompIF {
  public:
    SendRoutingInfoConfV2(Logger* logger);
    char* getMSRN();
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    Logger* logger;
    char _msrn[20];
};
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
#endif /* __SMSC_MTSMSME_COMP_SENDROUTINGINFO_HPP_ */
