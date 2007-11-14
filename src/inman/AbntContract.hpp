#pragma ident "$Id$"
#ifndef SMSC_INMAN_SUBSCR_CONTRACT_HPP
#define SMSC_INMAN_SUBSCR_CONTRACT_HPP

#include "inman/InScf.hpp"  //using TDPScfMap

namespace smsc {
namespace inman {

typedef char AbonentImsi[MAP_MAX_IMSI_AddressValueLength + 1];

struct AbonentContractInfo {
    enum ContractType { abtUnknown = 0, abtPostpaid = 1, abtPrepaid = 2 };

    ContractType    ab_type;
    AbonentImsi     abImsi;
    TDPScfMap       tdpSCF;

    AbonentContractInfo(ContractType cntr_type = abtUnknown, const char * p_imsi = NULL)
        : ab_type(cntr_type)
    {
        setImsi(p_imsi);
    }

    void setImsi(const char * p_imsi)
    { 
        if (p_imsi && p_imsi[0])
            strlcpy(abImsi, p_imsi, sizeof(abImsi));
        else
            abImsi[0] = 0;
    }

    void setSCF(TDPCategory::Id tdp_type, const GsmSCFinfo * p_scf)
    {
        if (p_scf)
            tdpSCF[tdp_type] = *p_scf;
        else
            tdpSCF[tdp_type].Reset();
    }

    void Reset(void) { ab_type = abtUnknown; abImsi[0] = 0; tdpSCF.clear();  }

    //Returns true if at least one parameter was updated
    bool Merge(const AbonentContractInfo & use_info)
    {
        bool rval = false;

        if (use_info.ab_type != AbonentContractInfo::abtUnknown) {
            ab_type = use_info.ab_type;
            rval = true;
        }
        if (use_info.getImsi()) {
            strlcpy(abImsi, use_info.abImsi, sizeof(abImsi));
            rval = true;
        }
        if (!use_info.tdpSCF.empty()) {
            tdpSCF.Merge(use_info.tdpSCF);
            rval = true;
        }
        return rval;
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
    inline const char * getImsi(void) const { return abImsi[0] ? (const char*)abImsi : NULL; }
    inline const char * imsiCStr(void) const { return abImsi[0] ? (const char*)abImsi : "<none>"; }

    inline const GsmSCFinfo * getSCFinfo(TDPCategory::Id tdp_type) const
    {
        TDPScfMap::const_iterator it = tdpSCF.find(tdp_type);
        return ((it == tdpSCF.end()) || it->second.scfAddress.empty()) ? NULL : &(it->second);
    }
};

} //inman
} //smsc
#endif /* SMSC_INMAN_SUBSCR_CONTRACT_HPP */

