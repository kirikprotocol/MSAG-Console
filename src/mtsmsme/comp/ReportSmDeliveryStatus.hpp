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

class ReportSmDeliveryStatus: public  CompIF {
  public:
    ReportSmDeliveryStatus();
    ~ReportSmDeliveryStatus();
    void setOTID(TrId _otid);
    void setComponent(const string& imsi, const string& msc,const string& vlr);

    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
private:
    TCMessage_t begin;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    UpdateLocationArg_t arg;
    VLR_Capability_t vlrcap;
    uint8_t otid[4];
    AC ac;
    OCTET_STRING_DECL(_imsi,20);
    OCTET_STRING_DECL(_vlr, 20);
    OCTET_STRING_DECL(_msc, 20);
};

/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }
#endif /* __SMSC_MTSMSME_COMP_REPORTSMDELIVERYSTATUS_HPP__ */
