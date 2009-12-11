/* ************************************************************************* *
 * Abonent CSI (CAMEL Subscription Information) helper classes definitions
 * ************************************************************************* */
#ifndef SMSC_INMAN_SUBSCR_CONTRACT_HPP
#ident "@(#)$Id$"
#define SMSC_INMAN_SUBSCR_CONTRACT_HPP

#include "inman/InScf.hpp"
#include "inman/CDRTypes.hpp"

namespace smsc {
namespace inman {

using smsc::util::IMSIString;

typedef TonNpiAddress AbonentId; //isdn international number assumed

struct AbonentContractInfo {
    enum ContractType { abtUnknown = 0, abtPostpaid = 1, abtPrepaid = 2 };

    ContractType    ab_type;
    IMSIString      abImsi;
    TDPScfMap       tdpSCF;

    AbonentContractInfo(ContractType cntr_type = abtUnknown, const char * p_imsi = NULL)
        : ab_type(cntr_type), abImsi(p_imsi)
    { }

    void setImsi(const char * p_imsi)
    {
        if (p_imsi && p_imsi[0])
            abImsi = p_imsi;
        else
            abImsi.clear();
    }

    void setSCF(TDPCategory::Id tdp_type, const GsmSCFinfo * p_scf)
    {
        if (p_scf)
            tdpSCF[tdp_type] = *p_scf;
        else
            tdpSCF[tdp_type].Reset();
    }

    void Reset(void) { ab_type = abtUnknown; abImsi.clear(); tdpSCF.clear();  }

    //Returns true if at least one parameter was updated
    bool Merge(const AbonentContractInfo & use_info)
    {
        bool rval = false;

        if (use_info.ab_type != AbonentContractInfo::abtUnknown) {
            ab_type = use_info.ab_type;
            rval = true;
        }
        if (use_info.getImsi()) {
            abImsi = use_info.abImsi;
            rval = true;
        }
        if (!use_info.tdpSCF.empty()) {
            tdpSCF.Merge(use_info.tdpSCF);
            rval = true;
        }
        return rval;
    }

    bool isUnknown(void) const { return (bool)(ab_type == abtUnknown); }
    bool isPostpaid(void) const { return (bool)(ab_type == abtPostpaid); }
    bool isPrepaid(void) const { return (bool)(ab_type == abtPrepaid); }

    static const char * type2Str(ContractType cntr_type)
    {
        if (cntr_type == abtPostpaid)
            return "Postpaid";
        if (cntr_type == abtPrepaid)
            return "Prepaid";
        return "Unknown";
    }

    const char * type2Str(void) const { return type2Str(ab_type); }
    //Returns cstr containing IMSI signals if latter is defined
    const char * getImsi(void) const { return !abImsi.empty() ? abImsi.c_str() : NULL; }
    //Returns cstr containing either IMSI signals or string "none"
    const char * imsiCStr(void) const { return !abImsi.empty() ? abImsi.c_str() : "<none>"; }

    const GsmSCFinfo * getSCFinfo(TDPCategory::Id tdp_type) const
    {
        TDPScfMap::const_iterator it = tdpSCF.find(tdp_type);
        return ((it == tdpSCF.end()) || it->second.scfAddress.empty()) ? NULL : &(it->second);
    }
};

struct AbonentRecord : public AbonentContractInfo {
    time_t  tm_queried;

    AbonentRecord(ContractType abType = abtUnknown,
                                const char * p_imsi = NULL, time_t qryTm = 0)
        : AbonentContractInfo(abType, p_imsi), tm_queried(qryTm)
    { }

    //Returns true if at least one parameter was updated
    bool Merge(const AbonentRecord & use_rcd)
    {
        bool rval = AbonentContractInfo::Merge(use_rcd);
        if (rval && use_rcd.tm_queried)
            tm_queried = use_rcd.tm_queried;
        return rval;
    }

    void reset(void)    { tm_queried = 0; AbonentContractInfo::Reset(); }

    //NOTE: tm_queried = zero, means record is ALWAYS expired!
    bool isExpired(uint32_t interval) const
    { return (bool)(time(NULL) >= (tm_queried + (time_t)interval)); }
};

} //inman
} //smsc
#endif /* SMSC_INMAN_SUBSCR_CONTRACT_HPP */

