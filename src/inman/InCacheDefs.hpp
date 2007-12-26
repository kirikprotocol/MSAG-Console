#pragma ident "$Id$"
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

struct AbonentCacheCFG {
    uint32_t    interval;   //default abonent info expiration interval, units: seconds
    uint32_t    RAM;        //abonents cache RAM buffer size, units: Mb
    int         fileRcrd;   //initial number of cache file records
    std::string nmDir;      //directory storing cache files

    AbonentCacheCFG() {
        interval = RAM = fileRcrd = 0;
    }
};

typedef TonNpiAddress AbonentId; //isdn international number assumed

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

    //NOTE: tm_queried = zero, means record ALWAYS expired!
    inline bool isExpired(uint32_t interval) const
    { return (bool)(time(NULL) >= (tm_queried + interval)); }
};


class AbonentCacheITF {
public:
    virtual AbonentContractInfo::ContractType
            getAbonentInfo(const AbonentId & ab_number,
                           AbonentRecord * ab_rec = NULL, uint32_t exp_timeout = 0) = 0;
    virtual void
            setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec) = 0;
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_DEFS_HPP */

