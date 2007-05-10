#ident "$Id$"
/* ************************************************************************** *
 * MTHashFileT<>: Direct addressing disk hash for use in MT environment.
 * NOTE: Rehashing doesn't perfomed automatically but just reported as condition
 * occurs. Special class is provided to perform rehashing in other thread.
 * NOTE: The following C preprocessor definitions customize rehasher functionality:
 *      HFREHASH_LOG_ON  - turns on general rehashing logging (errors & info),
 *      HFREHASH_LOG_DBG - turns on full rehashing logging
 * ************************************************************************** */
#ifndef _MULTITHREADED_HASH_FILE_HPP
#define _MULTITHREADED_HASH_FILE_HPP

#ifdef  HFREHASH_LOG_DBG
#  ifndef HFREHASH_LOG_ON
#    define HFREHASH_LOG_ON
#  endif /* HFREHASH_LOG_ON */
#endif /* HFREHASH_LOG_DBG */

#include "util/crc32.h"
#include "core/buffers/File.hpp"
using smsc::core::buffers::File;

#include "core/buffers/ExtendingBuf.hpp"
using smsc::core::buffers::ExtendingBuffer;

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "core/threads/Thread.hpp"
using smsc::core::threads::Thread;

#ifdef HFREHASH_LOG_ON
#include "logger/Logger.h"
using smsc::logger::Logger;
#endif /* HFREHASH_LOG_ON */

#define _THROWS_HFE  /* throw(FileException,Exception,std::runtime_error) */
#define _THROWS_NONE /* throw() */

namespace smsc {
namespace inman {
namespace cache {

class HashFileEntityITF {
public:
    //Reads serialized entity content from File,
    //Returns bumber of bytes red
    virtual uint32_t Read(File & fh, uint32_t max_octs = 0) _THROWS_HFE = 0; 
    //Returns bumber of bytes written
    virtual uint32_t Write(File & fh) const _THROWS_HFE = 0;
    //returns size of serialized entity content is to write
    virtual uint32_t Size(void) const = 0;
};

typedef HashFileEntityITF HashFileValueITF;

class HashFileKeyITF : public HashFileEntityITF {
public:
    virtual uint32_t HashCode(uint32_t attempt = 0) const = 0;
    virtual bool isEqual(const void * p_obj2) const = 0;
    //NOTE: operator== should be defined for interface implementation !!!
    //vitual int operator== (const HashFileKey & key2) const = 0;
};

struct HashFileCFG {
    static const uint32_t   _HF_Magic = 0x48664772;  //"HfGr"
    static const uint8_t    _HF_Version = 0x01;

    uint32_t    magic;
    uint8_t     ver;
    uint8_t     maxColl;
    uint16_t    keySz;
    uint16_t    valSz;

    HashFileCFG(uint16_t key_sz = 0, uint16_t val_sz = 0, uint8_t max_coll = 0)
        : magic(_HF_Magic), ver(_HF_Version)
        , keySz(key_sz), valSz(val_sz), maxColl(max_coll)
    { }

    int operator== (const HashFileCFG & dst) const
    {
        return (magic == dst.magic) && (ver == dst.ver) && (maxColl == dst.maxColl)
                && (keySz == dst.keySz) && (valSz == dst.valSz);
    }
    std::string PrintCfg(void) const
    {
        char buf[sizeof("{0x%X,0x%x,%u,%u,%u}") + sizeof(uint32_t)*3*5];
        int n = snprintf(buf, sizeof(buf)-1, "{0x%X,0x%x,%u,%u,%u}",
                         magic, (unsigned)ver, (unsigned)maxColl,
                         (unsigned)keySz, (unsigned)valSz);
        return buf;
    }
};

struct HashFileHeader : public HashFileCFG {
    uint32_t    size;
    uint32_t    used;
    uint32_t    crcSumm;

    HashFileHeader(uint32_t init_size = 0, uint16_t key_sz = 0,
                        uint16_t val_sz = 0, uint8_t max_coll = 0)
       : HashFileCFG(key_sz, val_sz, max_coll)
       , size(init_size), used(0), crcSumm(0)
    { }

    static inline uint32_t Size(void) { return 4*4 + 2*3; }
    inline uint32_t RcdSize(void) const { return Size() + keySz + valSz; }

    int equalCFG(const HashFileHeader & dst) const
    {
        return (*(HashFileCFG*)this) == (*(HashFileCFG*)&dst);
    }

    void Read(File& f) //throw (FileException)
    {
        magic = f.ReadNetInt32();
        ver   = f.ReadByte();
        maxColl = f.ReadByte();
        keySz = f.ReadNetInt16();
        valSz = f.ReadNetInt16();
        size  = f.ReadNetInt32();
        used  = f.ReadNetInt32();
        crcSumm = f.ReadNetInt32();
    }

    void Write(File& f)
    {
        f.WriteNetInt32(magic);
        f.WriteByte(ver);
        f.WriteByte(maxColl);
        f.WriteNetInt16(keySz);
        f.WriteNetInt16(valSz);
        f.WriteNetInt32(size);
        f.WriteNetInt32(used);
        crcSumm = HdrCrc();
        f.WriteNetInt32(crcSumm);
    }

    inline uint32_t add2crc(uint32_t crc, uint32_t fld)
    {
        uint32_t no = htonl(fld);
        return crc32(crc, &no, 4);
    }

    uint32_t HdrCrc(void)
    {
        uint32_t rv = add2crc(0, magic);
        rv = add2crc(rv, (uint32_t)ver);
        rv = add2crc(rv, (uint32_t)maxColl);
        rv = add2crc(rv, (uint32_t)keySz);
        rv = add2crc(rv, (uint32_t)valSz);
        rv = add2crc(rv, used);
        rv = add2crc(rv, size);
        return rv;
    }
};

class HFRecordHdr : public HashFileEntityITF {
public:
    typedef enum { rcdEmpty = 0, rcdUsed = 0x40, rcdErased = 0xC0 } RecordType;

    static const uint16_t _maxKeySzTA = 317; //0x13D
    static const uint16_t _maxValSzTA = 0x7FFF; //32767
    static const uint32_t _maxHdrSize = 6;

    RecordType  rType;
    uint32_t    hashCode;
    uint16_t    keySz;

    HFRecordHdr(RecordType r_type = rcdEmpty)
        : rType(r_type), hashCode(0), keySz(0)
    { }
    virtual ~HFRecordHdr() { }

    // -- HashFileEntityITF methods
    uint32_t Read(File & fh, uint32_t max_octs = 0) _THROWS_HFE
    {
        uint32_t    rv = 1;
        uint8_t fb = fh.ReadByte();
        rType = static_cast<RecordType>(fb & 0xC0);
        if (rType == rcdUsed) {
            keySz = fb & 0x3F;
            hashCode = fh.ReadNetInt32(); rv += 4;
            if (keySz == 0x3F) {
                keySz = (uint16_t)fh.ReadByte() + 0x3E; rv++;
            }
        } else
            hashCode = keySz = 0;
        return rv;
    }

    uint32_t Write(File & fh) const _THROWS_HFE
    {
        if (keySz > _maxKeySzTA)
            throw Exception("Illegal key size: %u", keySz);

        uint32_t    rv = 5; // 1+4 [+1]
        uint8_t     fb = (uint8_t)rType + ((keySz < 0x3F) ? (uint8_t)keySz : 0x3F);
        fh.WriteByte(fb);
        fh.WriteNetInt32(hashCode);
        if (keySz >= 0x3F) {
            fh.WriteByte(keySz - 0x3E); rv++;
        }
        return rv;
    }

    uint32_t Size(void) const
    {
        return (keySz >= 0x3F) ? 6 : 5; // 1+4 [+1]
    }
};

//maxKeySz <= 317(0x13D), maxValSz <= 32767(0x7FFF)
template < class HFKeyTA, /* : public HashFileKeyITF */
    uint16_t maxKeySzTA = HFRecordHdr::_maxKeySzTA,
    uint16_t maxValSzTA = HFRecordHdr::_maxValSzTA
> class HFRecordT { //HFRecord parameters (header + key)
protected:
    inline uint16_t readValSz(File& fh, uint16_t & valSz) _THROWS_HFE
    {
        uint16_t rv = 1;
        valSz = fh.ReadByte();
        if (valSz == 0xFF) {
            rv++; valSz = (uint16_t)fh.ReadByte() + 0xFE;
        }
        return rv;
    }

    inline uint16_t writeValSz(File& fh, uint16_t valSz) const _THROWS_HFE
    {
        if (valSz >= 0xFF) {
            fh.WriteByte(0xFF);
            fh.WriteByte((uint8_t)(valSz - 0xFE));
            return 2;
        }
        fh.WriteByte((uint8_t)valSz);
        return 1;
    }

public:
    HFKeyTA     key;
    HFRecordHdr hdr;
    unsigned    r_num;
    uint32_t    hcAttempt;
    
    HFRecordT(const HFKeyTA * use_key = NULL)
        : r_num(0), hcAttempt(0)
    { 
        if (use_key) setHashKey(*use_key);
    }

    inline uint32_t setHashCode(uint32_t attempt_num = 0)
    { return hdr.hashCode = key.HashCode(hcAttempt = attempt_num); }

    inline void setHashKey(const HFKeyTA & use_key)
    { key = use_key; hdr.keySz = use_key.Size(); }

    //reads record header at current file position,
    //{Key} is red only for non-empty record
    uint32_t    ReadHdrKey(File& fh) _THROWS_HFE
    {
        uint32_t rv = hdr.Read(fh);
        if (hdr.rType != HFRecordHdr::rcdEmpty) {
            if (!hdr.keySz || (hdr.keySz > maxKeySzTA))
                throw Exception("Illegal key size: %u", hdr.keySz);
            key.Read(fh, hdr.keySz);
            rv += hdr.keySz;
        }
        return rv;
    }

    //Reads value part of record
    uint32_t    ReadVal(File& fh, HashFileEntityITF *p_val) _THROWS_HFE
    {
        uint16_t valSz;
        uint32_t rv = readValSz(fh, valSz);
        if (valSz > maxValSzTA)
            throw Exception("Illegal value size: %u", valSz);

        if (valSz && p_val)
            rv += p_val->Read(fh, valSz);
        return rv;
    }

    //Writes value part of record
    uint32_t    WriteVal(File& fh, const HashFileEntityITF *p_val) const _THROWS_HFE
    {
        uint32_t valSz = p_val ? p_val->Size() : 0;
        if (valSz > maxValSzTA)
            throw Exception("Illegal value size: %u", valSz);

        uint32_t rv = writeValSz(fh, (uint16_t)valSz);
        if (valSz)
            rv += p_val->Write(fh);
        return rv;
    }

    //Reads whole record at current file position,
    //{Key,Value} is red only for non-empty record
    uint32_t    Read(File& fh, HashFileValueITF *p_val) _THROWS_HFE
    {
        uint32_t rv = ReadHdrKey(fh);
        if ((hdr.rType == HFRecordHdr::rcdUsed) && p_val)
            rv += ReadVal(fh, p_val);
        return rv;
    }

    //Writes whole record at current file position
    //NOTE: Do not forget to set hashCode!
    uint32_t    Write(File& fh, const HashFileValueITF *p_val) const _THROWS_HFE
    {
        if (!hdr.keySz || (hdr.keySz > HFRecordHdr::_maxKeySzTA))
            throw Exception("Illegal key size: %u", hdr.keySz);

        uint32_t rv = hdr.Write(fh);
        rv += key.Write(fh);
        if (p_val)
            rv += WriteVal(fh, p_val);
        return rv;
    }
};


//maxKeySz <= 317(0x13D), maxValSz <= 32767(0x7FFF)
template <
    class HFKeyTA /* : public HashFileKeyITF */,
    class HFValueTA /* : public HashFileValueITF */,
    uint16_t maxKeySzTA = 317, uint16_t maxValSzTA = 32767, uint8_t maxCollisionTA = 30>
class MTHashFileT {
public:
    typedef HFRecordT<HFKeyTA, maxKeySzTA, maxValSzTA> HFRecord;
    typedef typename HFValueTA  HFValue;

    class HFRehashAcquirerITF {
    public:
        virtual void onRehashDone(MTHashFileT * new_hf, const char * error = NULL) _THROWS_NONE  = 0;
    };

protected:
    Mutex           _fSync;
    bool            readOnly;
    File            fHdl;
    HashFileHeader  hfHdr;

    typedef enum { SEARCH_EMPTY = 0x01, SEARCH_USED  = 0x02, SEARCH_BOTH  = 0x03
    } SearchMode;

    inline void    SeekRcd(const uint32_t & r_num) _THROWS_HFE
    {
        static uint32_t rcdSz = HFRecordHdr::_maxHdrSize + maxKeySzTA + maxValSzTA;

        if (!r_num || (r_num > hfHdr.size))
            throw Exception("Invalid record number: %u", r_num);
        fHdl.Seek(HashFileHeader::Size() + (r_num-1)*rcdSz);
    }

    //Returns record number or zero if latter was not found.
    HFRecord searchRcd(const HFKeyTA & s_key, const SearchMode mode) _THROWS_HFE
    {
        HFRecord    rcd;
        for (uint8_t attempt = 0; attempt < hfHdr.maxColl; attempt++) {
            uint32_t hc = s_key.HashCode(rcd.hcAttempt = attempt);
            rcd.r_num = (hc % hfHdr.size) + 1;

            SeekRcd(rcd.r_num); //checks file status
            rcd.ReadHdrKey(fHdl);
            //in case of SEARCH_BOTH mode check first the SEARCH_EMPTY condition !!!
            if ((mode & SEARCH_EMPTY) && (rcd.hdr.rType != HFRecordHdr::rcdUsed)) {
                rcd.setHashKey(s_key); rcd.hdr.hashCode = hc;
                return rcd;
            }
            if (mode & SEARCH_USED) {
                if (rcd.hdr.rType == HFRecordHdr::rcdEmpty) {
                    rcd.r_num = 0; rcd.setHashKey(s_key); rcd.hdr.hashCode = hc;
                    return rcd;
                }
                if ((rcd.hdr.rType == HFRecordHdr::rcdErased) || (rcd.hdr.hashCode != hc))
                    continue;
                if (rcd.key.isEqual(&s_key))
                    return rcd;
            }
        }
        rcd.r_num = 0;
        return rcd;
    }

    inline HFRecordHdr::RecordType  ReadRecord(uint32_t r_num, HFRecord & rcd, HashFileValueITF *p_val) _THROWS_HFE
    {
        SeekRcd(r_num);
        rcd.Read(fHdl, p_val);
        rcd.r_num = r_num;
        return rcd.hdr.rType;
    }
    inline void WriteRecord(const HFRecord & rcd, const HFValueTA *p_val) _THROWS_HFE
    {
        SeekRcd(rcd.r_num);
        rcd.Write(fHdl, p_val);
    }

    void    Reset(void) _THROWS_HFE
    {
        hfHdr.used = 0;
        hfHdr.crcSumm = hfHdr.HdrCrc();
        fHdl.Seek(0);
        hfHdr.Write(fHdl);
        fHdl.ZeroFill(hfHdr.size*hfHdr.RcdSize());
        fHdl.Flush();
    }

    //Returns number of records used
    uint32_t   do_close(bool flushHdr = true) _THROWS_HFE
    {
        if (fHdl.isOpened()) {
            if (!readOnly && flushHdr) {
                hfHdr.crcSumm = hfHdr.HdrCrc();
                fHdl.Seek(0);
                hfHdr.Write(fHdl);
            }
            fHdl.Close();
        }
        return hfHdr.used;
    }

public:
    MTHashFileT(uint32_t init_size = maxCollisionTA) : readOnly(true)
    { 
        hfHdr.maxColl = maxCollisionTA;
        hfHdr.keySz = maxKeySzTA;
        hfHdr.valSz = maxValSzTA;
        hfHdr.size  = init_size > maxCollisionTA ? init_size : maxCollisionTA;
        hfHdr.used  = hfHdr.crcSumm = 0;
    }

    MTHashFileT(const HashFileHeader & use_hdr) : readOnly(true), hfHdr(use_hdr)
    { }

    ~MTHashFileT()
    { 
        MutexGuard  grd(_fSync);
        try { do_close(); } catch (...) { }
    }

    inline uint32_t Size(void) const { return hfHdr.size; }
    inline uint32_t Used(void) const { return hfHdr.used; }
    inline uint8_t  Usage(void) const { return (uint8_t)(((1000L*hfHdr.used)/hfHdr.size) + 5)/10; }
    inline void     getCFG(HashFileHeader & hf_hdr) const { hf_hdr = hfHdr; }
    inline bool     isOpened(void) const { return fHdl.isOpened(); }

    inline const std::string & FileName(void) const { return fHdl.getFileName(); }
    inline void Rename(const char* newname)   { fHdl.Rename(newname); }

    //updates file header and flushes hash file
    void Flush(void)  _THROWS_HFE
    {
        hfHdr.crcSumm = hfHdr.HdrCrc();
        fHdl.Seek(0);
        hfHdr.Write(fHdl);
        fHdl.Flush();
    }

    void    Clear(void) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        Reset();
    }

    //Returns true if file exists, false if it was just created
    bool    Open(const char* nm_file, uint32_t init_sz = 0, bool read_only = false) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        if (!nm_file || !nm_file[0])
            throw Exception("Hash file name is not specified");

        bool alive = File::Exists(nm_file);
        if (read_only && !alive)
            throw Exception("Hash file not found: '%s'", nm_file);

        if (read_only)
            fHdl.ROpen(nm_file);
        else if (alive)
            fHdl.RWOpen(nm_file);
        else {
            if (hfHdr.size < init_sz)
                hfHdr.size = init_sz;
            fHdl.RWCreate(nm_file);
            Reset();
        }

        if (alive) { //check header
            HashFileHeader hdr;
            //check file size first
            File::offset_type fSz = fHdl.Size();
            if (!fSz) {
                Reset();
            } else if (fSz >= hdr.Size()) {
                hdr.Read(fHdl);
                if (hdr.crcSumm != hdr.HdrCrc()) {
                    fHdl.Close();
                    throw Exception("Hash file header CRC error: '%s'", nm_file);
                }
                if (!hfHdr.equalCFG(hdr)) {
                    fHdl.Close();
                    throw Exception("Hash file incompatible, header '%s' vs '%s'",
                                    hdr.PrintCfg().c_str(), hfHdr.PrintCfg().c_str());
                }
                hfHdr = hdr;
            } else {
                fHdl.Close();
                throw Exception("Hash file incompatible, header size: '%u'", (unsigned)fSz);
            }
        }
        readOnly = read_only;
        return alive;
    }

    //Returns number of records used
    uint32_t   Close(void) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        return do_close();
    }

    //Returns number of records used
    uint32_t   Destroy(void) _THROWS_NONE
    {
        MutexGuard  grd(_fSync);
        try { do_close(false); unlink(fHdl.getFileName().c_str()); 
        } catch (...) { }
        return hfHdr.used;
    }

    //returns number of record, zero - no record with given key exists
    uint32_t    LookUp(const HFKeyTA & key, HFValueTA * p_val = NULL) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        HFRecord srcd = searchRcd(key, SEARCH_USED);
        if (srcd.r_num && p_val) //record header is already red
            p_val->Read(fHdl);
        return srcd.r_num;
    }

    //Returns number of record, zero - failure, hashFile should be rehashed
    inline uint32_t Insert(const HFKeyTA & key, const HFValueTA * value, bool update = false) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        HFRecord srcd = searchRcd(key, update ? SEARCH_BOTH : SEARCH_EMPTY);
        if (srcd.r_num) {
            uint16_t    newRcd = (srcd.hdr.rType != HFRecordHdr::rcdUsed) ? 1 : 0;
            srcd.hdr.rType = HFRecordHdr::rcdUsed;
            WriteRecord(srcd, value); //throws
            hfHdr.used += newRcd;
        } //else needs rehash
        return srcd.r_num;
    }

    //Returns number of erased record, zero - no record with given key exists
    //NOTE: it only marks record as erased, preserving content
    uint32_t    Erase(const HFKeyTA & key) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        HFRecord srcd = searchRcd(&key, SEARCH_USED);
        if ((srcd.r_num && (srcd.hdr.rType == HFRecordHdr::rcdUsed)) {
            SeekRcd(srcd.r_num);
            fHdl.WriteByte(HFRecordHdr::rcdErased);
            hfHdr.used--;
        }
        return srcd.r_num;
    }
    //returns number of next non-empty record, zero - no more records.
    uint32_t    NextRecord(uint32_t r_num, HFRecord & rcd, HashFileValueITF *p_val) _THROWS_HFE
    {
        MutexGuard  grd(_fSync);
        rcd.hdr.rType = HFRecordHdr::rcdEmpty;
        while ((r_num <= hfHdr.size) && (rcd.hdr.rType != HFRecordHdr::rcdUsed)) {
            ReadRecord(r_num++, rcd, p_val); //throws
        }
        return (rcd.hdr.rType != HFRecordHdr::rcdUsed) ? 0 : --r_num;
    }

    //NOTE: HFRehasher doesn't close either original HashFile or produced one
    class HFRehasher : Thread {
    protected:
        typedef ExtendingBuffer<char, 256> FNameBuffer;

        volatile bool   _running;
        Mutex           _sync;
        MTHashFileT *   srcHF;
        HashFileHeader  hfHdr;
        HFRehashAcquirerITF * client;
        std::auto_ptr<MTHashFileT> dstHF;
        std::string     _exc;
        FNameBuffer     new_fname;
        uint8_t         _rate;
        uint32_t        orgSz;
#ifdef HFREHASH_LOG_ON
        Logger *        logger;
#endif /* HFREHASH_LOG_ON */

    public:
#ifndef HFREHASH_LOG_ON
        HFRehasher(HFRehashAcquirerITF * p_client, MTHashFileT * use_src, uint8_t r_rate = 2)
            : _running(false), client(p_client), srcHF(use_src), _rate(r_rate)
        { }
#else  /* HFREHASH_LOG_ON */
        HFRehasher(HFRehashAcquirerITF * p_client, MTHashFileT * use_src, Logger * use_log = NULL, uint8_t r_rate = 2)
            : _running(false), client(p_client), srcHF(use_src), _rate(r_rate)
        { 
            logger = use_log ? use_log : Logger::getInstance("smsc.inman.cache");
        }
#endif /* HFREHASH_LOG_ON */
        ~HFRehasher()
        {
            Stop();
            Thread::WaitFor();
        }

        void Start(void) _THROWS_NONE
        {
            MutexGuard tmp(_sync);
            _exc.clear();
            Thread::Start();
        }
        void Stop(void)
        {
            _sync.Lock();
            if (_running == true) {
                _running = false;
                _exc = "aborted";
            }
            _sync.Unlock();
        }
//        inline void WaitFor(void) { Thread::WaitFor(); }

    protected:
        void dupHashFile(void) _THROWS_HFE
        {
            srcHF->getCFG(hfHdr);
            orgSz = hfHdr.size;

            hfHdr.used = 0;
            hfHdr.size *= _rate;
            if (orgSz > hfHdr.size)
                throw Exception("rehashing rate limit reached: %u", _rate);
            
            new_fname.Write(0, srcHF->FileName().c_str(), srcHF->FileName().length());
            new_fname.setPos(new_fname.getDataSize());
            new_fname.Append(".XXXXXX\0", 8);
            int fd = mkstemp(new_fname.get());
            if (fd < 0)
                throw FileException(FileException::errOpenFailed, new_fname.get());

            close(fd);
            if (dstHF.get())
                dstHF->Destroy();
            dstHF.reset(new MTHashFileT(hfHdr));
            dstHF->Open(new_fname.get()); //throws
#ifdef HFREHASH_LOG_ON
            smsc_log_info(logger, "HFRH: %s -> %s, rate %u", srcHF->FileName().c_str(),
                           new_fname.get(), (unsigned)_rate);
#endif /* HFREHASH_LOG_ON */
        }

        int Execute(void) _THROWS_NONE
        {
            _sync.Lock();
            _running = true;

            try { dupHashFile(); 
            } catch (const std::exception & exc) {
#ifdef HFREHASH_LOG_ON
                smsc_log_error(logger, "HFRH: %s", exc.what());
#endif /* HFREHASH_LOG_ON */
                _exc = exc.what();
                _running = false;
            }
            uint32_t    r_num = 1;
            while (_running && r_num) {
                _sync.Unlock();
                try {
                    uint32_t    n_num;
                    HFRecord    rcd;
                    HFValueTA   val;
                    if ((r_num = srcHF->NextRecord(r_num, rcd, &val)) != 0) {
                        if (!(n_num = dstHF->Insert(rcd.key, &val, true))) {
                            _rate++;
                            dupHashFile();  //enlarge new HF
                            r_num = 1;      //repeat copying to a new file
                        } else {
#ifdef HFREHASH_LOG_DBG
                            smsc_log_debug(logger, "HFRH: rcd[%u.%u] -> rcd[%u.%u]",
                                           orgSz, r_num, hfHdr.size, n_num);
#endif /* HFREHASH_LOG_DBG */
                            r_num++;
                        }
                    }
                } catch (const std::exception & exc) {
                    MutexGuard tmp(_sync);
#ifdef HFREHASH_LOG_ON
                    smsc_log_error(logger, "HFRH: %s", exc.what());
#endif /* HFREHASH_LOG_ON */

                    _exc = exc.what();
                    r_num = 0;
                }
                _sync.Lock();
            }
            _running = false;
            _sync.Unlock();

            if (!_exc.empty()) {
                if (dstHF.get()) {
                    dstHF->Destroy();
                    dstHF.reset();
                }
                client->onRehashDone(NULL, _exc.c_str());
            } else {
                dstHF->Flush();
                client->onRehashDone(dstHF.release());
            }
            return 0;
        }
    };
};

}// namespace cache
}// namespace inman
}// namespace smsc
#endif /* _MULTITHREADED_HASH_FILE_HPP */

