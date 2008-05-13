#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_UPDATELOCATION_HPP_
#define __SMSC_MTSMSME_COMP_UPDATELOCATION_HPP_

#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/util.hpp"
#include "Component.hpp"
#include "UpdateLocationArg.h"
#include <string>

namespace smsc{ namespace mtsmsme{ namespace comp{

using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::AC;
using std::string;

class UpdateLocationMessage: public  CompIF {
  public:
    UpdateLocationMessage();
    ~UpdateLocationMessage();
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
#endif /* __SMSC_MTSMSME_COMP_UPDATELOCATION_HPP_ */
