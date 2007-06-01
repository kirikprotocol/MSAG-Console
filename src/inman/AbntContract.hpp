#ident "$Id$"
#ifndef SMSC_INMAN_SUBSCR_CONTRACT_HPP
#define SMSC_INMAN_SUBSCR_CONTRACT_HPP

#include <inttypes.h>

#include "util/TonNpiAddress.hpp"
using smsc::util::TonNpiAddress;
using smsc::util::GsmSCFinfo;

namespace smsc {
namespace inman {

typedef char AbonentImsi[MAP_MAX_IMSI_AddressValueLength + 1];

struct AbonentContractInfo {
    enum ContractType { abtUnknown = 0, abtPostpaid = 1, abtPrepaid = 2 };

    ContractType    ab_type;
    GsmSCFinfo      gsmSCF;
    AbonentImsi     abImsi;

    AbonentContractInfo(ContractType cntr_type = abtUnknown, const GsmSCFinfo * p_scf = NULL,
                        const char * p_imsi = NULL)
        : ab_type(cntr_type)
    {
        setSCF(p_scf);
        setImsi(p_imsi);
    }

    void setSCF(const GsmSCFinfo * p_scf)
    {
        if (p_scf)
            gsmSCF = *p_scf;
        else
            gsmSCF.Reset();
    }
    void setImsi(const char * p_imsi)
    { 
        if (p_imsi && p_imsi[0])
            strlcpy(abImsi, p_imsi, sizeof(abImsi));
        else
            abImsi[0] = 0;
    }
    void Reset(void) { ab_type = abtUnknown; gsmSCF.Reset(); abImsi[0] = 0; }
    void Merge(const AbonentContractInfo & use_info)
    {
        ab_type = use_info.ab_type;
        if (use_info.getSCFinfo())
            gsmSCF = use_info.gsmSCF;
        if (use_info.getImsi())
            strlcpy(abImsi, use_info.abImsi, sizeof(abImsi));
    }

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

    inline const char * type2Str(void) const { return type2Str(ab_type); }
    inline const GsmSCFinfo * getSCFinfo(void) const
    { return gsmSCF.scfAddress.length ? &gsmSCF : NULL; }
    inline const char * getImsi(void) const { return abImsi[0] ? (const char*)abImsi : NULL; }
    inline const char * imsiCStr(void) const { return abImsi[0] ? (const char*)abImsi : "<none>"; }
};

} //inman
} //smsc
#endif /* SMSC_INMAN_SUBSCR_CONTRACT_HPP */

