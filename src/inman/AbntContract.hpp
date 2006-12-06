#ident "$Id$"
#ifndef SMSC_INMAN_SUBSCR_CONTRACT_HPP
#define SMSC_INMAN_SUBSCR_CONTRACT_HPP

#include <inttypes.h>

#include "util/TonNpiAddress.hpp"
using smsc::util::TonNpiAddress;

namespace smsc {
namespace inman {

struct GsmSCFinfo { //gsmSCF paramaters
    uint32_t      serviceKey;   //4 bytes long
    TonNpiAddress scfAddress;

    GsmSCFinfo() : serviceKey(0) { }
};

struct AbonentContractInfo {
    typedef enum { abtUnknown = 0, abtPostpaid = 1, abtPrepaid = 2 } ContractType;

    ContractType    ab_type;
    GsmSCFinfo      gsmSCF;

    AbonentContractInfo() : ab_type(abtUnknown)
    { }
    AbonentContractInfo(ContractType cntr_type, const GsmSCFinfo * p_scf = NULL)
        : ab_type(cntr_type)
    { if (p_scf) gsmSCF = *p_scf; }

    inline bool isUnknown(void) const { return (bool)(ab_type == abtUnknown); }
    inline bool isPostpaid(void) const { return (bool)(ab_type == abtPostpaid); }
    inline bool isPrepaid(void) const { return (bool)(ab_type == abtPrepaid); }

    static const char * type2Str(ContractType cntr_type)
    {
        if (cntr_type == abtPostpaid)
            return "Postpaid";
        if (cntr_type == abtPrepaid)
            return "Prepaid";
        return "Unknown";
    }

    const char * type2Str(void) const { return type2Str(ab_type); }
    const GsmSCFinfo * getSCFinfo(void) const
    { return gsmSCF.scfAddress.length ? &gsmSCF : NULL; }
};

} //inman
} //smsc
#endif /* SMSC_INMAN_SUBSCR_CONTRACT_HPP */

