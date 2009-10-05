#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_REPORTSMDELIVERYSTATUS_HPP__
#define __SMSC_MTSMSME_COMP_REPORTSMDELIVERYSTATUS_HPP__

//#include "mtsmsme/processor/Message.hpp"
#include "Component.hpp"
#include "ReportSM-DeliveryStatusArg.h"
#include <string>

#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name)	{ memset(&name, 0, sizeof(name)); name.buf = name##_buf; }
#define Address2OCTET_STRING(octs, addr)	{ ZERO_OCTET_STRING(octs); \
    octs.size = packMAPAddress2OCTS(addr, (TONNPI_ADDRESS_OCTS *)(octs.buf)); }

namespace smsc{ namespace mtsmsme{ namespace comp{

using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::AC;
using std::string;

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
