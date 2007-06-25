#ident "$Id$"
/* ************************************************************************* *
 * INMan abonent contract data cache interface and helper classes definitions
 * ************************************************************************* */
#ifndef SMSC_INMAN_CACHE_DEFS_HPP
#define SMSC_INMAN_CACHE_DEFS_HPP

#include <list>
#include <time.h>

#include "inman/AbntContract.hpp"
using smsc::inman::AbonentContractInfo;

namespace smsc {
namespace inman {
namespace cache {

typedef TonNpiAddress AbonentId; //isdn international number assumed

enum { MAX_ABONENT_ID_LEN =  11 }; //only isdn international numbers supported

struct AbonentRecord : public AbonentContractInfo {
    time_t  tm_queried;

    AbonentRecord(ContractType abType = abtUnknown, time_t qryTm = 0,
                  const GsmSCFinfo * p_scf = NULL, const char * p_imsi = NULL)
        : AbonentContractInfo(abType, p_scf, p_imsi), tm_queried(qryTm)
    { }
    AbonentRecord(const AbonentRecord & ab_rec)
        : AbonentContractInfo(ab_rec.ab_type, ab_rec.getSCFinfo(), ab_rec.getImsi())
        , tm_queried(ab_rec.tm_queried)
    { }

    void Merge(const AbonentRecord & use_rcd)
    {
        AbonentContractInfo::Merge(use_rcd);
        if (use_rcd.tm_queried)
            tm_queried = use_rcd.tm_queried;
    }

    void reset(void)    { tm_queried = 0; AbonentContractInfo::Reset(); }

    //NOTE: tm_queried = zero, means record ALWAYS expired!
    inline bool isExpired(long interval) const
    { return (bool)(time(NULL) >= (tm_queried + interval)); }
};

//typedef AbonentContractInfo::ContractType AbonentContractType;


class AbonentCacheITF {
public:
    virtual AbonentContractInfo::ContractType
            getAbonentInfo(const AbonentId & ab_number, AbonentRecord * ab_rec = NULL) = 0;
    virtual void
            setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec) = 0;
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_DEFS_HPP */

