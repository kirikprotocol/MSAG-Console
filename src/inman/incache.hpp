#ident "$Id$"
#ifndef SMSC_INMAN_CACHE_HPP
#define SMSC_INMAN_CACHE_HPP

#include <list>
#include <time.h>

#include "logger/Logger.h"
#include "inman/common/adrutil.hpp"
#include "core/buffers/Hash.hpp"
#define NOLOGGERPLEASE
#include "core/buffers/DiskHash.hpp"
#undef NOLOGGERPLEASE

using smsc::logger::Logger;
using smsc::cvtutil::TonNpiAddress;
using smsc::core::buffers::Hash;
using smsc::core::buffers::DiskHash;

namespace smsc {
namespace inman {
namespace cache {

typedef TonNpiAddress AbonentId; //isdn international number assumed

enum { MAX_ABONENT_ID_LEN =  11 }; //only isdn international numbers supported

typedef enum { btUnknown = 0, btPostpaid, btPrepaid 
} AbonentBillType;

extern const char * _sabBillType[];

class AbonentCacheITF {
public:
    virtual AbonentBillType getAbonentInfo(AbonentId & ab_number) = 0;
    virtual void setAbonentInfo(const AbonentId & ab_number, AbonentBillType ab_type,
                                                            time_t queried = 0) = 0;
};


class InAbonentQueryListenerITF {
public:
    virtual void onAbonentQueried(const AbonentId & ab_number, AbonentBillType ab_type) = 0;
};

class InAbonentProviderITF {
public:
    //binds AbonentCache to Provider
    virtual void bindCache(AbonentCacheITF * cache) = 0;
    //Starts query and binds listener to it. If AbonentCache is bound, the abonent info
    //will be stored in it on query completion. 
    //Returns true if query succesfully started, false otherwise
    virtual bool startQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    //Unbinds query listener, cancels query if no listeners remain.
    virtual void cancelQuery(const AbonentId & ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    virtual void cancelAllQueries(void) = 0;
};

struct AbonentRecord {
    AbonentBillType ab_type;
    time_t          tm_queried;

    AbonentRecord()
        : ab_type(smsc::inman::cache::btUnknown), tm_queried(0) {}
    AbonentRecord(AbonentBillType abType, time_t qryTm)
        : ab_type(abType), tm_queried(qryTm) {}
};

struct AbonentCacheCFG {
    long    interval;   //abonent info refreshing interval, units: seconds
    long    RAM;        //abonents cache RAM buffer size, units: Mb
    int     fileRcrd ;  //initial number of cache file records
    const char *nmDir;  //

    AbonentCacheCFG() {
        interval = RAM = 0; /*nmFile = */nmDir = NULL;
    }
};

class AbonentHashKey : public AbonentId {
public:
    AbonentHashKey() { }
    AbonentHashKey(const AbonentId & ab_num) : AbonentId(ab_num) { }

    //DiskHash interface methods
    void Read(File& fh) /* throw (FileException) */
    {
        fh.Read(value, MAX_ABONENT_ID_LEN);
        value[length = MAX_ABONENT_ID_LEN] = 0;
        //isdn international address only
        numPlanInd = typeOfNumber = 1;
    }
    void Write(File& fh) /* throw (FileException) */ const
    { fh.Write(value, MAX_ABONENT_ID_LEN); }

    static uint32_t Size(void)
    { return MAX_ABONENT_ID_LEN; }

    //based on Donald E. Knuth hash
    uint32_t HashCode(uint32_t attempt = 0) const
    { 
        uint32_t hcode = attempt + length;
        for (uint32_t i = 0; i < (uint32_t)length; i++)
           hcode = ((hcode << 5) ^ (hcode >> 27)) ^ value[i];
        return hcode;
    }
    int operator== (const AbonentHashKey & key2)
    {
        return !strcmp((const char*)value, key2.getSignals());
    }
};

class AbonentHashData : public AbonentRecord {
public:
    AbonentHashData() : AbonentRecord()
    { }
    AbonentHashData(AbonentBillType abType, time_t qryTm)
        : AbonentRecord(abType, qryTm)
    { }
    
    static void WriteTimeT(File& fh, time_t val) /* throw (FileException) */
    {
        if (sizeof(time_t) > sizeof(uint32_t))
            fh.WriteNetInt64((uint64_t)val);
        else
            fh.WriteNetInt32((uint32_t)val);
    }
    static time_t ReadTimeT(File& fh) /* throw (FileException) */
    {
        time_t val;
        if (sizeof(time_t) > sizeof(uint32_t))
            val = (time_t)fh.ReadNetInt64();
        else
            val = (time_t)fh.ReadNetInt32();
        return val;
    }

    //DiskHash interface methods
    static uint32_t Size(void)
    { return sizeof(time_t) + 1; }

    void Read(File& fh) /* throw (FileException) */
    {
        ab_type = (AbonentBillType)fh.ReadByte();
        tm_queried = ReadTimeT(fh);
    }
    void Write(File& fh) /* throw (FileException) */ const
    { 
        fh.WriteByte((unsigned char)ab_type);
        WriteTimeT(fh, tm_queried);
    }
};


class AbonentCache: public AbonentCacheITF {
public:
    typedef std::list<AbonentId>    AccessList;
    struct AbonentRecordRAM : public AbonentRecord {
        AccessList::iterator accIt;

        AbonentRecordRAM() : AbonentRecord()
        { }
        AbonentRecordRAM(AbonentBillType abType, time_t qryTm)
            : AbonentRecord(abType, qryTm)
        { }
    };

private:
    typedef DiskHash<AbonentHashKey, AbonentHashData> AbonentHash;

    Logger *            logger;
    AbonentCacheCFG     _cfg;
    Mutex                   cacheGuard;
    Hash<AbonentRecordRAM>  cache;
    AccessList              accList;
    unsigned int            maxRamIt;
    AbonentHash             flCache;
    bool                    useFile;

public:
    AbonentCache(AbonentCacheCFG * cfg, Logger * uselog = NULL);
    ~AbonentCache();

    //AbonentCacheITF interface methods
    AbonentBillType getAbonentInfo(AbonentId & ab_number);
    void setAbonentInfo(const AbonentId & ab_number, AbonentBillType ab_type,
                        time_t queried = 0);
protected:
    bool load(const char * file_nm);
    bool close(void);
    int  ramInsert(const AbonentId & ab_number, AbonentBillType ab_type, time_t & queried);
    AbonentBillType ramLookUp(AbonentId & ab_number);
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_HPP */

