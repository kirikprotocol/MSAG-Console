#pragma ident "$Id$"
/* ************************************************************************* *
 * INMan abonent contract data cache interface and helper classes definitions
 * ************************************************************************* */
#ifndef SMSC_INMAN_CACHE_DEFS_HPP
#define SMSC_INMAN_CACHE_DEFS_HPP

#include "inman/AbntContract.hpp"
using smsc::inman::AbonentContractInfo;
using smsc::inman::AbonentRecord;
using smsc::inman::AbonentId;

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

