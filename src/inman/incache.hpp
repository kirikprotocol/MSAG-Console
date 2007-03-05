#ident "$Id$"
/* ************************************************************************* *
 * INMan abonent contract data cache implementation.
 * Two-layers schema is used: preemptive RAM cache over direct addressing
 * file cache (smsc::core::buffers::DiskHash).
 * ************************************************************************* */
#ifndef SMSC_INMAN_CACHE_HPP
#define SMSC_INMAN_CACHE_HPP

#include "inman/InCacheDefs.hpp"

#include "inman/common/adrutil.hpp"
using smsc::cvtutil::packMAPAddress2OCTS;
using smsc::cvtutil::unpackOCTS2MAPAddress;

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "core/buffers/Hash.hpp"
#define NOLOGGERPLEASE
#include "core/buffers/DiskHash.hpp"
#undef NOLOGGERPLEASE
using smsc::core::buffers::Hash;
using smsc::core::buffers::DiskHash;


namespace smsc {
namespace inman {
namespace cache {

struct AbonentCacheCFG {
    long    interval;   //abonent info refreshing interval, units: seconds
    long    RAM;        //abonents cache RAM buffer size, units: Mb
    int     fileRcrd;   //initial number of cache file records
    std::string nmDir;  //directory storing cache files

    AbonentCacheCFG() {
        interval = RAM = fileRcrd = 0;
    }
};

class AbonentCache: public AbonentCacheITF {
public:
    AbonentCache(AbonentCacheCFG * cfg, Logger * uselog = NULL);
    ~AbonentCache();

    // -- AbonentCacheITF interface methods
    AbonentContractInfo::ContractType
        getAbonentInfo(const AbonentId & ab_number, AbonentRecord * ab_rec = NULL);
    void
        setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec);

protected:

    // -- RAM Cache definition
    typedef std::list<AbonentId>    AccessList;
    struct AbonentRecordRAM : public AbonentRecord {
        AccessList::iterator accIt;

        AbonentRecordRAM() : AbonentRecord()
        { }
        AbonentRecordRAM(const AbonentRecord & ab_rec) : AbonentRecord(ab_rec)
        { 
            if (!tm_queried)
                tm_queried = time(NULL);
        }
    };

    int  ramInsert(const AbonentId & ab_number, const AbonentRecord & ab_rec);
    AbonentRecord * ramLookUp(const AbonentId & ab_number);

    // -- DiskHash interfaces implementation

    //DiskHash Key interface methods implementation
    class AbonentHashKey : public AbonentId {
    public:
        AbonentHashKey() { }
        AbonentHashKey(const AbonentId & ab_num) : AbonentId(ab_num) { }

        //DiskHash interface methods
        void Read(File& fh) /* throw (FileException) */
        {
            fh.Read(signals, MAX_ABONENT_ID_LEN);
            signals[length = MAX_ABONENT_ID_LEN] = 0;
            //isdn international address only
            numPlanInd = typeOfNumber = 1;
        }
        void Write(File& fh) /* throw (FileException) */ const
        { fh.Write(signals, MAX_ABONENT_ID_LEN); }

        static uint32_t Size(void) { return MAX_ABONENT_ID_LEN; }

        //based on Donald E. Knuth hash
        uint32_t HashCode(uint32_t attempt = 0) const
        { 
            uint32_t hcode = attempt + length;
            for (uint32_t i = 0; i < (uint32_t)length; i++)
                hcode = ((hcode << 5) ^ (hcode >> 27)) ^ signals[i];
            return hcode;
        }
        int operator== (const AbonentHashKey & key2)
        {
            return !strcmp((const char*)signals, key2.getSignals());
        }
    };

    //DiskHash Value interface methods implementation
    class AbonentHashData : public AbonentRecord {
    public:
        AbonentHashData() : AbonentRecord()
        { }
        AbonentHashData(const AbonentRecord & ab_rec) : AbonentRecord(ab_rec)
        { 
            if (!tm_queried)
                tm_queried = time(NULL);
        }

        AbonentRecord * getAbonentRecord(void) { return (AbonentRecord *)this; }

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

        static uint32_t Size(void)
        { return sizeof(time_t) + 1 + sizeof(uint32_t) + 1 + MAP_MAX_ISDN_AddressLength; }

        void Read(File& fh) /* throw (FileException) */
        {
            uint8_t fb = fh.ReadByte();
            ab_type = (AbonentContractInfo::ContractType)(fb & 0x7F);
            tm_queried = ReadTimeT(fh);
            gsmSCF.scfAddress.clear();
            if (fb & 0x80) {
                gsmSCF.serviceKey = (uint32_t)fh.ReadNetInt32();
                uint8_t len = fh.ReadByte();
                if (len && (len <= MAP_MAX_ISDN_AddressLength)) {
                    TONNPI_ADDRESS_OCTS oct;
                    oct.b0.tonpi = fh.ReadByte();
                    fh.Read(oct.val, len - 1);
                    unpackOCTS2MAPAddress(gsmSCF.scfAddress, &oct, len - 1);
                }
            } else
                gsmSCF.serviceKey = 0;
        }

        void Write(File& fh) /* throw (FileException) */ const
        { 
            fh.WriteByte(gsmSCF.serviceKey ? ((unsigned char)ab_type | 0x80) : 
                         (unsigned char)ab_type);
            WriteTimeT(fh, tm_queried);
            if (gsmSCF.serviceKey) {
                fh.WriteNetInt32(gsmSCF.serviceKey);
                TONNPI_ADDRESS_OCTS oct;
                unsigned len = packMAPAddress2OCTS(gsmSCF.scfAddress, &oct);
                if ((len > 1) && (len <= MAP_MAX_ISDN_AddressLength)) {
                    fh.WriteByte((uint8_t)len);
                    fh.WriteByte(oct.b0.tonpi);
                    fh.Write(oct.val, len - 1);
                } else
                    fh.WriteByte(0);
            }
        }
    };

    //Loads file cache
    bool load(const char * file_nm);
    //Closes and saves file cache
    bool close(void);

private:
    typedef DiskHash<AbonentHashKey, AbonentHashData> AbonentHash;

    Logger *                logger;
    AbonentCacheCFG         _cfg;
    Mutex                   cacheGuard;
    Hash<AbonentRecordRAM>  cache;
    AccessList              accList;
    unsigned int            maxRamIt;
    AbonentHash             flCache;
    bool                    useFile;
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_HPP */

