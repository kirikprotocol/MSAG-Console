/* ************************************************************************* *
 * Abonents Cache service types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_ABONENTS_CACHE_DEFS_HPP
#ident "@(#)$Id$"
#define __INMAN_ABONENTS_CACHE_DEFS_HPP

#include "inman/AbntContract.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::inman::AbonentContractInfo;
using smsc::inman::AbonentRecord;
using smsc::inman::AbonentId;

struct AbonentCacheCFG {
    static const uint32_t   _MAX_CACHE_INTERVAL = 65535; //minutes >= 45 days
    static const uint32_t   _DFLT_CACHE_INTERVAL = 60;   //minutes
    static const int        _DFLT_CACHE_RECORDS = 10000;
    static const uint32_t   _DFLT_RAMCACHE_SZ = 5;       // Mb

    uint32_t    interval;   //default abonent info expiration interval, units: seconds
    uint32_t    RAM;        //abonents cache RAM buffer size, units: Mb
    int         fileRcrd;   //initial number of cache file records
    std::string nmDir;      //directory storing cache files

    AbonentCacheCFG()
    {
        interval = RAM = fileRcrd = 0;
    }
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

#endif /* __INMAN_ABONENTS_CACHE_DEFS_HPP */

