#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_REPORTSMDELIVERYSTATUS_HPP__
#define __SMSC_MTSMSME_COMP_REPORTSMDELIVERYSTATUS_HPP__

#include "mtsmsme/comp/Component.hpp"
#include "logger/Logger.h"
#include "ReportSM-DeliveryStatusArg.h"
#include <string>

namespace smsc{ namespace mtsmsme{ namespace comp{

using std::string;
using smsc::logger::Logger;
using smsc::mtsmsme::comp::CompIF;

class ReportSmDeliveryStatusReq: public CompIF {
  public:
    ReportSmDeliveryStatusReq(const string& msisdn, const string& scaddress);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    ReportSM_DeliveryStatusArg_t arg;
};
class ReportSmDeliveryStatusInd: public CompIF {
  public:
    ReportSmDeliveryStatusInd(Logger* logger);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
  private:
    Logger* logger;
};
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
#endif /* __SMSC_MTSMSME_COMP_REPORTSMDELIVERYSTATUS_HPP__ */
