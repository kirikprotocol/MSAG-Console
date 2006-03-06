#ident "$Id$"
#ifndef SMSC_INMAN_CACHE_HPP
#define SMSC_INMAN_CACHE_HPP

#include <time.h>
#include "logger/Logger.h"

using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace cache {

typedef const char * AbonentId;

typedef enum { btUnknown = 0, btPostpaid, btPrepaid 
} AbonentBillType;

class AbonentCacheITF {
public:
    virtual AbonentBillType getAbonentInfo(AbonentId ab_number) = 0;
    virtual void setAbonentInfo(AbonentId ab_number, AbonentBillType ab_type,
                                                            time_t expired = 0) = 0;
};


class InAbonentQueryListenerITF {
public:
    virtual void onAbonentQueried(AbonentId ab_number, AbonentBillType ab_type) = 0;
};

class InAbonentProviderITF {
public:
    //binds AbonentCache to Provider
    virtual void bindCache(AbonentCacheITF * cache) = 0;
    //Starts query and binds listener to it. If AbonentCache is bound, the abonent info
    //will be stored in it on query completion. 
    //Returns true if query succesfully started, false otherwise
    virtual bool startQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    //Unbinds query listener, cancels query if no listeners remain.
    virtual void cancelQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    virtual void cancelAllQueries(void) = 0;
};

struct AbonentRecord {
    AbonentBillType ab_type;
    time_t          tm_expired;

    AbonentRecord()
        : ab_type(smsc::inman::cache::btUnknown), tm_expired(0) {}
    AbonentRecord(AbonentBillType type, time_t tmex)
        : ab_type(type), tm_expired(tmex) {}
};

struct AbonentCacheCFG {
    long    interval;   //abonent info refreshing interval, units: seconds
    long    RAM;        //abonents cache RAM buffer size, units: Mb
    const char *nmFile;      //

    AbonentCacheCFG() {
        interval = RAM = 0; nmFile = NULL;
    }
};

class AbonentCache: public AbonentCacheITF {
private:
    Logger *            logger;
    AbonentCacheCFG     _cfg;
    Mutex               cacheGuard;
    Hash<AbonentRecord> cache;

public:
    AbonentCache(AbonentCacheCFG * cfg, Logger * uselog = NULL);
    ~AbonentCache();

    //AbonentCacheITF interface methods
    AbonentBillType getAbonentInfo(AbonentId ab_number);
    void setAbonentInfo(AbonentId ab_number, AbonentBillType ab_type,
                        time_t expired = 0);

    bool load(const char * file_nm) { /* todo: */ return true; }
    bool save(const char * file_nm) { /* todo: */ return true; }
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_HPP */

