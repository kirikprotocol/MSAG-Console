/* ************************************************************************* *
 * Abonents contract and gsmSCFs data cache. Multithreaded implementation with
 * real-time response. Two-layers schema is used: preemptive RAM cache over
 * MT direct addressing file cache(index and data are stored in same file).
 * Up to three threads are in use:
 * 1) RAMCache, 2) FileCacheUpdater, 3) FileCacheRehasher (if needed)
 * ************************************************************************* */
#ifndef __INMAN_ABNT_MTCACHE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABNT_MTCACHE_HPP

#include <list>

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "core/buffers/XHash.hpp"
using smsc::core::buffers::XHash;

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/incache/AbCacheDefs.hpp"
#define HFREHASH_LOG_ON
#include "inman/incache/solid/MTHashFile.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::util::MAPConst;
using smsc::inman::comp::CSIUid_e;
using smsc::inman::comp::UnifiedCSI;


class AbonentCacheMTR: public AbonentCacheITF {
public:
  AbonentCacheMTR(const AbonentCacheCFG & use_cfg, Logger * uselog = NULL);
  ~AbonentCacheMTR();

  // -- AbonentCacheITF interface methods
  virtual AbonentContract_e
      getAbonentInfo(const AbonentId & ab_number,
                     AbonentSubscription * ab_rec = NULL, uint32_t exp_timeout = 0);
  virtual void
      setAbonentInfo(const AbonentId & ab_number, const AbonentSubscription & ab_rec);

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
    uint32_t      bcdSz;
    unsigned char bcd[_maxSize + 2];

  public:
    AbonentHashKey() : AbonentId(), bcdSz(0)
    { }
    AbonentHashKey(const AbonentId & ab_num) _THROWS_HFE;
    //
    virtual ~AbonentHashKey()
    { }

    bool operator== (const AbonentHashKey & key2) const
    {
      return AbonentId::operator ==(key2);
    }

    // -- HashFileKeyITF interface methods
    uint32_t Read(File & fh, uint32_t max_octs = 0) _THROWS_HFE;
    uint32_t Write(File& fh) _THROWS_HFE const
    {
      fh.Write(bcd, bcdSz); //throws
      return bcdSz;
    }

    uint32_t Size(void) const { return bcdSz; }

    uint32_t HashCode(uint32_t attempt = 0) const;

    bool isEqual(const void * p_obj2) const
    {
      return operator==(*(const AbonentHashKey*)p_obj2);
    }

    //-- XHash<> argument HF implemenatation
    static unsigned int CalcHash(const AbonentHashKey & key)
    {
      return key.HashCode(0);
    }
  };

  //MTHashFileT<> HFValueTA argument implementation
  class AbonentHashData : public HashFileEntityITF, public AbonentSubscription {
  public:
    static const CSIUid_e _knownCSI[2]; //MO-BC & MO-SM
    static const uint8_t  _maxCSIsNum = (uint8_t)(sizeof(_knownCSI)/sizeof(CSIUid_e));

    static const uint32_t _maxSize = sizeof(time_t) + 1 + 1 
        + _maxCSIsNum * (1 + sizeof(uint32_t) + 1 + MAPConst::MAX_ISDN_AddressLength);

    AbonentHashData() : AbonentSubscription()
    { }
    ~AbonentHashData()
    { }

    AbonentSubscription & abRecord(void) { return *(AbonentSubscription*)this; }

    static uint32_t WriteTimeT(File& fh, time_t val) _THROWS_HFE;
    static time_t ReadTimeT(File& fh) _THROWS_HFE;

    // -- HashFileEntityITF methods implementation
    uint32_t Read(File& fh, uint32_t max_octs = 0) _THROWS_HFE;
    //
    uint32_t Write(File& fh) const _THROWS_HFE;

    //NOTE: for simplicity just return max record size
    uint32_t Size(void) const { return _maxSize; }

  protected:
    uint8_t getKnownCSIsNum(void) const;
  };

  // -- RAM Cache definition
  class AbonentsList {    //LILO list
  protected:
    typedef std::list<AbonentId> AbntIdList;

    AbntIdList            idList;
    AbntIdList::size_type size;

  public:
    typedef std::list<AbonentId>::iterator iterator;
    typedef std::list<AbonentId>::size_type size_type;

    AbonentsList() : size(0)
    { }
    ~AbonentsList()
    { }

    size_type Size(void) const { return size; }
    bool empty(void) const    { return idList.empty(); }
    void clear(void)          { idList.clear(); size = 0; }

    AbonentId front(void)     { return idList.front(); }
    iterator  begin(void)     { return idList.begin(); }
    iterator  end(void)       { return idList.end(); }

    void erase(iterator & it)
    {
      idList.erase(it);
    }
    void push_back(AbonentId new_id)
    {
      idList.push_back(new_id); ++size;
    }
    void pop_front(void)
    {
      idList.pop_front();
      if (size) --size;
    }
    void move_back(iterator & it_from)
    {
      idList.splice(idList.end(), idList, it_from);
    }
  };

  class AbonentRecordRAM : public AbonentSubscription {
  public:
    bool marked;    //record marked for transfer to external
                    //storage, so cann't be evicted
    AbonentsList::iterator accIt;

    AbonentRecordRAM() : AbonentSubscription(), marked(false)
    { }
    AbonentRecordRAM(const AbonentSubscription & ab_rec, bool do_mark = false)
      : AbonentSubscription(ab_rec), marked(do_mark)
    {
      if (!tmQueried) tmQueried = time(NULL); 
    }

    const AbonentSubscription & abRecord(void) const
    {
      return *(const AbonentSubscription *)this;
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

    void Init(uint32_t max_size, uint32_t max_space_att = 250)
    {
      maxRamIt = max_size ? max_size : 10000;
      maxSpaceAtt = max_space_att;
    }
    Mutex & Sync(void) { return rcSync; }

    bool LookUp(const AbonentId & ab_number,
                        AbonentSubscription & ab_rec, uint32_t exp_timeout = 0);
    //Marks the record, preventing it from eviction,
    //returns false if no such abonent exists
    bool Mark(const AbonentId & ab_number);
    //Updates abonent record. NOTE: also marks the record
    int  Update(const AbonentId & ab_number, const AbonentSubscription & ab_rec);
    //Extracts marked abonent record, unmarking the latter
    bool NextMarked(AbonentId & ab_num, AbonentSubscription & ab_rec);
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
      FSCacheMonitor(FileCache * fs_cache, RAMCache *ram_cache, Logger * use_log = NULL);
      //
      ~FSCacheMonitor();

      void Awake(void) { _sync.notify(); }

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
      //
      bool Start(void);
      //
      void Stop(bool do_wait = false);

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

