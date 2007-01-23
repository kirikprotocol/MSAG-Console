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

    AbonentRecord(ContractType abType = abtUnknown, time_t qryTm = 0, const GsmSCFinfo * p_scf = NULL)
        : AbonentContractInfo(abType, p_scf), tm_queried(qryTm)
    { }
    AbonentRecord(const AbonentRecord & ab_rec)
        : AbonentContractInfo(ab_rec.ab_type, ab_rec.getSCFinfo())
        , tm_queried(ab_rec.tm_queried)
    { }

    void reset(void)    { tm_queried = 0; AbonentContractInfo::Reset(); }

    inline bool isExpired(long interval) const
    { return tm_queried ? (bool)(time(NULL) >= (tm_queried + interval)) : false; }

    AbonentRecord& operator= (const AbonentContractInfo & ab_info)
    {
        *(AbonentContractInfo*)this = ab_info;
        tm_queried = 0;
        return *this;
    }
};

typedef AbonentRecord::ContractType AbonentBillType;

class AbonentCacheITF {
public:
    virtual AbonentBillType getAbonentInfo(AbonentId & ab_number,
                                           AbonentRecord * ab_rec = NULL) = 0;
    virtual void setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec) = 0;
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_DEFS_HPP */

