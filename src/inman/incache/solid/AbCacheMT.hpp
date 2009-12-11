/* ************************************************************************* *
 * Abonents contract and gsmSCFs data cache. Multithreaded implementation with
 * real-time response. Two-layers schema is used: preemptive RAM cache over
 * MT direct addressing file cache(index and data are stored in same file).
 * Up to three threads are in use:
 * 1) RAMCache, 2) FileCacheUpdater, 3) FileCacheRehasher (if needed)
 * ************************************************************************* */
#ifndef __INMAN_ABNT_MTCACHE_HPP
#ident "@(#)$Id$"
#define __INMAN_ABNT_MTCACHE_HPP

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "core/buffers/XHash.hpp"
using smsc::core::buffers::XHash;

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

#include "inman/incache/AbCacheDefs.hpp"
#define HFREHASH_LOG_ON
#include "inman/incache/solid/MTHashFile.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::util::MAPConst;

using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::packMAPAddress2OCTS;
using smsc::cvtutil::unpackOCTS2MAPAddress;
using smsc::cvtutil::packNumString2BCD;
using smsc::cvtutil::unpackBCD2NumString;

class AbonentCacheMTR: public AbonentCacheITF {
public:
    AbonentCacheMTR(const AbonentCacheCFG & use_cfg, Logger * uselog = NULL);
    ~AbonentCacheMTR();

    // -- AbonentCacheITF interface methods
    AbonentContractInfo::ContractType
        getAbonentInfo(const AbonentId & ab_number,
                       AbonentRecord * ab_rec = NULL, uint32_t exp_timeout = 0);
    void
        setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec);

    //Starts file cache activity
    bool Start(void);
    //Stops file cache activity
    void Stop(bool do_wait = false);

protected:
    // -----------------------------------------------
    // -- MTHashFileT<> template arguments implementation
    // -----------------------------------------------
    //MTHashFileT<> HFKeyTA argument implementation:
    //maintain signals of ISDN International address as its BCD form
    class AbonentHashKey : public AbonentId, public HashFileKeyITF {
    public:
        static const uint32_t _maxSize = MAPConst::MAX_ISDN_AddressLength;

    private:
        using AbonentId::operator==; //hide it to avoid annoying CC warnings

    protected:
        unsigned char bcd[_maxSize + 2];
        uint32_t      bcdSz;

    public:
        AbonentHashKey() : bcdSz(0)
        { }
        AbonentHashKey(const AbonentId & ab_num) _THROWS_HFE
            : AbonentId(ab_num)
        {
            bcdSz = packNumString2BCD(bcd, signals, length);
            if (bcdSz > _maxSize)
                throw Exception("AbonentHashKey(): invalid length %u", length);
        }

        inline bool operator== (const AbonentHashKey & key2) const
        {
            if ((typeOfNumber == key2.typeOfNumber)
                && (numPlanInd == key2.numPlanInd)
                && (length == key2.length)) {
                return !strcmp(signals, key2.signals) ? true : false;
            }
            return false;
        }

        // -- HashFileKeyITF interface methods
        uint32_t Read(File& fh, uint32_t max_octs = 0) _THROWS_HFE
        {
            if (!max_octs || (max_octs > _maxSize))
                max_octs = _maxSize;
            //File::Read() throws if reads less bytes than requested
            bcdSz = (uint32_t)fh.Read(bcd, max_octs);
            length = unpackBCD2NumString(bcd, signals, bcdSz);
            signals[length] = 0;
            if (!length)
                throw Exception("AbonentHashKey::Read(): zero length");

            //isdn international address only
            numPlanInd = typeOfNumber = 1;
            return bcdSz;
        }
        uint32_t Write(File& fh) _THROWS_HFE const
        {
            fh.Write(bcd, bcdSz); //throws
            return bcdSz;
        }

        inline uint32_t Size(void) const { return bcdSz; }

        uint32_t HashCode(uint32_t attempt = 0) const
        {
            uint32_t hcode = 0;
            for (uint32_t i = 0; i<= attempt; ++i)
                hcode = crc32(hcode, bcd, bcdSz);
            return hcode;
        }

        bool isEqual(const void * p_obj2) const
        {
            return operator==(*(const AbonentHashKey*)p_obj2);
        }

        //-- XHash<> argument HF implemenatation
        static inline unsigned int CalcHash(const AbonentHashKey & key)
        {
            return key.HashCode(0);
        }
    };

    //MTHashFileT<> HFValueTA argument implementation
    class AbonentHashData : public HashFileEntityITF, public AbonentRecord {
    public:
        static const uint32_t _maxSize = sizeof(time_t) + 1 + 1 
            + (TDPCategory::dpRESERVED_MAX - 1)
                * (1 + sizeof(uint32_t) + 1 + MAPConst::MAX_ISDN_AddressLength);

        AbonentHashData() : AbonentRecord()
        { }
        ~AbonentHashData()
        { }

        inline AbonentRecord & abRecord(void) { return *(AbonentRecord*)this; }

        static inline uint32_t WriteTimeT(File& fh, time_t val) _THROWS_HFE
        {
            if (sizeof(time_t) > sizeof(uint32_t)) {
                fh.WriteNetInt64((uint64_t)val);
                return 8;
            }
            fh.WriteNetInt32((uint32_t)val);
            return 4;
        }
        static inline time_t ReadTimeT(File& fh) _THROWS_HFE
        {
            time_t val;
            if (sizeof(time_t) > sizeof(uint32_t))
                val = (time_t)fh.ReadNetInt64();
            else
                val = (time_t)fh.ReadNetInt32();
            return val;
        }

        // -- HashFileEntityITF methods implementation
        uint32_t Read(File& fh, uint32_t max_octs = 0) _THROWS_HFE
        {
            uint32_t rv = sizeof(time_t) + 1;
            uint8_t fb = fh.ReadByte();
            ab_type = (AbonentContractInfo::ContractType)(fb & 0x7F);
            tm_queried = ReadTimeT(fh);

            tdpSCF.clear();
            if (fb & 0x80) { //SCF parms present
                uint8_t cnt = fh.ReadByte(); ++rv;   //number of SCFs
                for (; cnt; --cnt) {
                    fb = fh.ReadByte(); ++rv;
                    TDPCategory::Id  tdpType = static_cast<TDPCategory::Id>(fb);
                    if (tdpType != TDPCategory::dpUnknown) {   //read SCF parms
                        GsmSCFinfo      gsmSCF;
                        gsmSCF.serviceKey = (uint32_t)fh.ReadNetInt32();
                        uint8_t len = fh.ReadByte();
                        rv += 5;
                        if (len && (len <= MAPConst::MAX_ISDN_AddressLength)) {
                            uint8_t oct_buf[sizeof(TONNPI_ADDRESS_OCTS)];
                            fh.Read(oct_buf, len);
                            rv += len;
                            unpackOCTS2MAPAddress(gsmSCF.scfAddress, oct_buf, len);
                        }
                        tdpSCF[tdpType] = gsmSCF;
                    }
                }
            }
            return rv;
        }

        uint32_t Write(File& fh) _THROWS_HFE const
        {
            uint32_t sz = 1;
            uint8_t fb = (uint8_t)ab_type;
            if (!tdpSCF.empty())
                fb |= 0x80;
            fh.WriteByte(fb);
            sz += WriteTimeT(fh, tm_queried);

            if ((fb = (uint8_t)tdpSCF.size()) != 0) {
                fh.WriteByte(fb); ++sz;     //number of SCFs
                for (TDPScfMap::const_iterator it = tdpSCF.begin();
                                            it != tdpSCF.end(); ++it) {
                    if (it->second.scfAddress.empty()) {
                        fh.WriteByte(TDPCategory::dpUnknown); ++sz;
                    } else {
                        fh.WriteByte((uint8_t)it->first); ++sz;
                        //write SCF parms
                        fh.WriteNetInt32(it->second.serviceKey);
                        sz += 4 + 1;    //serviceKey + address length

                        uint8_t oct_buf[sizeof(TONNPI_ADDRESS_OCTS)];
                        unsigned len = packMAPAddress2OCTS(it->second.scfAddress, oct_buf);
                        if ((len > 1) && (len <= MAPConst::MAX_ISDN_AddressLength)) {
                            fh.WriteByte((uint8_t)len);
                            fh.Write(oct_buf, len);
                            sz += len;
                        } else
                            fh.WriteByte(0);
                    }
                }
            }
            return sz;
        }

        //NOTE: for simplicity just return max record size
        uint32_t Size(void) const { return _maxSize; }
    };

    // -- RAM Cache definition
    class AbonentsList {    //LILO list
    protected:
        typedef std::list<AbonentId> AbntIdList;

        AbntIdList  idList;
        AbntIdList::size_type   size;

    public:
        typedef std::list<AbonentId>::iterator iterator;
        typedef std::list<AbonentId>::size_type size_type;

        AbonentsList() : size(0)
        { }
        ~AbonentsList()
        { }

        inline size_type Size(void) const { return size; }
        inline bool empty(void) const    { return idList.empty(); }
        inline void clear(void)          { idList.clear(); size = 0; }

        inline AbonentId front(void)     { return idList.front(); }
        inline iterator  begin(void)     { return idList.begin(); }
        inline iterator  end(void)       { return idList.end(); }
        inline void erase(iterator & it)
        {
            idList.erase(it); 
            if (size)
                --size;
        }
        inline void push_back(AbonentId new_id)
        {
            idList.push_back(new_id);
            ++size;
            return;
        }
        inline void pop_front(void)
        {
            idList.pop_front();
            if (size)
                --size;
            return;
        }
        inline void move_back(iterator & it_from)
        {
            idList.splice(idList.end(), idList, it_from);
        }
    };


    struct AbonentRecordRAM : public AbonentRecord {
    public:
        bool marked;    //record marked for transfer to external
                        //storage, so cann't be evicted
        AbonentsList::iterator accIt;

        AbonentRecordRAM()
            : AbonentRecord(), marked(false)
        { }
        AbonentRecordRAM(const AbonentRecord & ab_rec, bool do_mark = false)
            : AbonentRecord(ab_rec), marked(do_mark)
        { if (!tm_queried) tm_queried = time(NULL); }

        inline const AbonentRecord & abRecord(void)
        {
            return *(const AbonentRecord *)this;
        }
    };

    class RAMCache : XHash<AbonentHashKey, AbonentRecordRAM, AbonentHashKey> {
    protected:
        mutable Mutex   rcSync;
         //abonents sorted by access time (first is the last accessed one)
        AbonentsList    accList;
        //abonents marked for transfer to external storage
        AbonentsList    updList;
        uint32_t        maxRamIt;   //maximum number of elements in hash
        uint32_t        maxSpaceAtt;//maximum number of 'victim search'
                                    //attempts for makeSpace()

        void makeSpace(void);

    public:
        RAMCache() : maxRamIt(/* must be != 0*/ 1)
        { }
        ~RAMCache()
        { }

        inline void Init(uint32_t max_size, uint32_t max_space_att = 250)
        {
            maxRamIt = max_size ? max_size : 10000;
            maxSpaceAtt = max_space_att;
        }
        inline Mutex & Sync(void) { return rcSync; }

        bool LookUp(const AbonentId & ab_number,
                            AbonentRecord & ab_rec, uint32_t exp_timeout = 0);
        //Marks the record, preventing it from eviction,
        //returns false if no such abonent exists
        bool Mark(const AbonentId & ab_number);
        //Updates abonent record. NOTE: also marks the record
        int  Update(const AbonentId & ab_number, const AbonentRecord & ab_rec);
        //Extracts marked abonent record, unmarking the latter
        bool NextMarked(AbonentId & ab_num, AbonentRecord & ab_rec);
    };

    typedef MTHashFileT<AbonentHashKey, AbonentHashData,
                    AbonentHashKey::_maxSize, AbonentHashData::_maxSize> FileCache;
    //FileSystem Cache updater/rehasher
    class FSCacheMonitor : Thread, public FileCache::HFRehashAcquirerITF {
    private:
        using Thread::Start; //hide it to avoid annoying CC warnings

    protected:
        EventMonitor            _sync;
        volatile bool           _rehashOn;
        volatile bool           _running;
        volatile bool           _rehashMode; //
        RAMCache *              ramCache;
        std::auto_ptr<FileCache> fsCache;
        std::auto_ptr<FileCache::HFRehasher> rehasher;
        Logger *                logger;

        int Execute(void);

    public:
        static const int FSC_TIMEOUT_STEP = 100; //millisecs

        //Takes ownership of fs_cache!!!
        FSCacheMonitor(FileCache * fs_cache, RAMCache *ram_cache, Logger * use_log = NULL)
            : _rehashOn(false), _running(false), _rehashMode(true), ramCache(ram_cache)
        { 
            logger = use_log ? use_log : Logger::getInstance("smsc.inman.InCache");
            fsCache.reset(fs_cache);
            _rehashMode = fsCache->rehashAllowed();
            if (!_rehashMode) {
                smsc_log_info(logger, "FSCache: rehash mode is OFF for %s",
                              fsCache->Details().c_str());
            }
        }
        ~FSCacheMonitor()
        {
            Stop(true);
            Thread::WaitFor();
            MutexGuard  grd(_sync);
            try {
                fsCache->Close();
                smsc_log_info(logger, "FSCache: closed %s", fsCache->Details().c_str());
            } catch (std::exception & exc) {
                smsc_log_error(logger, "FSCache: %s", exc.what());
            }
        }

        inline void Awake(void) { _sync.notify(); }

        uint32_t LookUp(const AbonentHashKey & key, AbonentHashData & val) _THROWS_HFE
        {
            MutexGuard  grd(_sync);
            return fsCache->LookUp(key, &val);
        }

        bool isRunning(void)
        {
            MutexGuard grd(_sync);
            return _running;
        }

        bool Start(void)
        {
            if (isRunning())
                return true;
            smsc_log_debug(logger, "FSCache: starting ..");
            Thread::Start();
            {
                MutexGuard grd(_sync);
                if (_running)
                    return true;
                _sync.wait(FSC_TIMEOUT_STEP);
                if (!_running) {
                    smsc_log_fatal(logger, "FSCache: unable to start");
                    return false;
                }
            }
            return true;
        }

        void Stop(bool do_wait = false) //waits for thread being finished
        {
            {
                MutexGuard  grd(_sync);
                if (_running) {
                    _rehashMode = _running = false;
                    smsc_log_debug(logger, "FSCache: stopping ..");
                }
                if (_rehashOn) {
                    smsc_log_debug(logger, "FSCache: stopping rehasher ..");
                    rehasher->Stop();
                    //wait for rehasher report
                    while (_rehashOn)
                        _sync.wait(FSC_TIMEOUT_STEP);
                }
                _sync.notify();
            }
            if (do_wait)
                Thread::WaitFor();
        }

        // -- HFRehashAcquirerITF methods
        void onRehashDone(FileCache * new_hf, const char * error = NULL) _THROWS_NONE;
    };

//private:
    Logger *                logger;
    AbonentCacheCFG         _cfg;
    EventMonitor            _sync;
    RAMCache                ramCache;
    std::auto_ptr<FSCacheMonitor> fscMgr;
};

} //cache
} //inman
} //smsc

#endif /* __INMAN_ABNT_MTCACHE_HPP */

