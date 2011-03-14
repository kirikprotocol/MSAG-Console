#ifndef _SCAG_UTIL_STORAGE_CACHEDDELAYEDTHREADEDDISKSTORAGE_H
#define _SCAG_UTIL_STORAGE_CACHEDDELAYEDTHREADEDDISKSTORAGE_H

#include "logger/Logger.h"
#include "core/buffers/XHash.hpp"
#include "scag/util/Time.h"
#include "HeapAllocator.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "FlushConfig.h"

namespace scag2 {
namespace util {
namespace storage {

template <
    class MemStorage,
    class DiskStorage,
    class Serializer,
    class Allocator = HeapAllocator< typename MemStorage::key_type, typename MemStorage::value_type > >
    class CachedDelayedThreadedDiskStorage : public Allocator
{
public:
    typedef typename MemStorage::key_type       key_type;
    typedef typename MemStorage::value_type     value_type;
    typedef typename MemStorage::stored_type    stored_type;
    typedef typename MemStorage::hash_function  hash_function;

private:
    // dirty things
    struct Dirty {
        key_type            key;
        stored_type         stored;
        unsigned            changes;
        util::msectime_type addTime;
    };
    typedef std::list< Dirty >  DirtyList;
    typedef smsc::core::buffers::XHash< key_type, typename DirtyList::iterator, hash_function > DirtyHash;

public:

    CachedDelayedThreadedDiskStorage( MemStorage* ms,
                                      DiskStorage* ds,
                                      Serializer* srin,
                                      Serializer* srout,
                                      const FlushConfig& fc,
                                      smsc::logger::Logger* thelog = 0 ) :
    cache_(ms), disk_( ds ), serin_(srin), serout_(srout), hitcount_(0),
    pfget_(0), bget_(0), pfset_(0), bset_(0), fc_(fc),
    log_(thelog), moreToFlush_(0)
    {
        if ( !ms || !srin || !srout || !ds ) {
            delete ms;
            delete srin;
            delete srout;
            delete ds;
            throw smsc::util::Exception("CachedDiskStorage2: all storages and serializers should be provided!");
        }
        spare_ = cache_->val2store(0);
    }

    /// underlying structure access
    MemStorage& memoryCache() { return *cache_; }
    DiskStorage& diskStorage() { return *disk_; }
    Serializer& serializer() { return *serin_; }

    ~CachedDelayedThreadedDiskStorage() {
        // first of all, flush everything not flushed yet
        util::msectime_type now = util::currentTimeMillis() + fc_.flushInterval*2;
        while ( flushDirty(now) ) {}
        // then deallocate elements remained in the clean queue
        Dirty d;
        while ( cleanQueue_.Pop(d) ) {
            cache_->dealloc(d.stored);
        }
        cache_->dealloc(spare_);
        cache_->clean();
        delete cache_;
        delete serin_;
        delete serout_;
        delete disk_;
    }

    /// @return true if old value was replaced
    bool set( const key_type& k, value_type* v ) {
        // first check the cache
        stored_type* vv = cache_->get(k);
        if (vv) {
            value_type* ov = cache_->setval(*vv,v);
            delete ov;
            return ov;
        }
        
        {
            MutexGuard mg(dirtyMon_);
            stored_type sv = popCleanQ(k);
            if (cache_->store2val(sv)) {
                value_type* ov = cache_->setval(sv,v);
                delete ov;
                return cache_->set(k,sv);
            }

            typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
            if (di) {
                if (log_) {
                    smsc_log_error(log_,"strange: profile k=%s in dirtyhash on set -- is get() before set() forgotten?",k.toString().c_str());
                }
            }
        }
        return cache_->set(k,cache_->val2store(v));
    }


    value_type* get( const key_type& k, bool create = false ) {
        // first check the cache
        stored_type* const vv = cache_->get(k);
        if ( vv ) {
            if ( cache_->store2val(*vv) ) {
                ++this->hitcount_;
            } else if ( create ) {
                // NOTE: we won't invoke faultHandler if we already
                // have something (even NULL) with this key in cache.
                // This is to remember the negative hit.
                // However, if we want to create something it should be done here.
                stored_type v = cache_->val2store(alloc(k));
                cache_->set(k,v);
                return cache_->store2val(v);
            }
            return cache_->store2val(*vv);
        }

        {
            MutexGuard mg(dirtyMon_);
            // then we want to check clean queue
            stored_type sv = popCleanQ(k);
            if ( cache_->store2val(sv) ) {
                cache_->set(k,sv);
                return cache_->store2val(sv);
            }

            // we did not find the element in clean queue
            // trying the dirty hash
            typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
            if (di) {
                if (log_) { smsc_log_debug(log_,"dirty key=%s found",k.toString().c_str()); }
                return cache_->store2val((*di)->stored);
            }
        }

        // we did not find the element in dirty hash, going to fetch it from the disk
        stored_type v = faultHandler(k, create);
        // NOTE: we may set NULL in cache to remember negative hit.
        // if ( cache_->store2val(v) )
        cache_->set(k,v);
        return cache_->store2val(v);
    }


    /// flush dirty item to disk
    bool flush( const key_type& k ) {
        throw smsc::util::Exception("flush is not allowed");
        /*
        typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
        bool res = false;
        if (di) {
            typename DirtyList::iterator i = *di;
            if ( cache_->store2val(i->stored) ) {
                res = doFlush(k,i->stored);
                cache_->set(k,i->stored);
            } else {
                cache_->dealloc(i->stored);
            }
            dirtyList_.erase(i);
            dirtyHash_.Delete(k);
        }
        return res;
         */
    }

    //TODO: flush without cache_->get()

    /// Only necessary for get + modifications
    bool markDirty( const key_type& k )
    {
        stored_type sv = cache_->release(k);
        if ( cache_->store2val(sv) ) {
            MutexGuard mg(dirtyMon_);
            if (addDirty(k,sv)) return true;
        } else {
            cache_->dealloc(sv);
            MutexGuard mg(dirtyMon_);
            // look into clean queue
            sv = popCleanQ(k);
            if ( ! cache_->store2val(sv) ) {
                typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
                if (di) {
                    ++((*di)->changes);
                }
                return true;
            }
            // NOTE: we have to increment the number of changes for item in dirtyHash
            // as it may be going to become clean by being flushed right now.
            if (addDirty(k,sv)) return true;
        }
        // else, we cannot add a new dirty element to the dirty hash
        // as it is overwhelmed.  Simply put it to cache.
        cache_->set(k,sv);
        return false;
    }


    /// flush dirty item to disk
    /// @return number of bytes written
    unsigned flushDirty( util::msectime_type now )
    {
        // fast check w/o locking
        if ( dirtyHash_.Count() == 0 ) return 0;
        typename DirtyList::iterator di;
        unsigned dhc, cqc, oldChanges;
        const char* reason = "?";
        {
            MutexGuard mg(dirtyMon_);
            dhc = unsigned(dirtyHash_.Count());
            if ( !dhc ) return 0;

            cqc = unsigned(cleanQueue_.Count()); // for logging

            // accessing dirtyList item via iterator is safe
            di = dirtyList_.begin();
            oldChanges = di->changes;

            // low mark check
            if (moreToFlush_) {
                // we have more items to flush
                reason = (moreToFlush_ == 1) ? "last" : "more";
            } else if ( dhc > fc_.flushLowMark ) {
                // we have reached low mark
                moreToFlush_ = fc_.flushCount;
                reason = "lowmark";
            } else if ( now - di->addTime > fc_.flushInterval ) {
                // too old item
                reason = "tooold";
            } else {
                // nothing to flush
                return 0;
            }

        }

        value_type* dv = cache_->store2val(di->stored);
        assert(dv);
        {
            // NOTE: we have to serialize under the lock
            smsc::core::synchronization::MutexGuardTmpl<value_type> mg(*dv);
            serout_->serialize(di->key,cache_->store2ref(di->stored));
        }

        const unsigned written = unsigned(serout_->getOwnedBuffer()->size());

        if (log_) {
            smsc_log_debug(log_,"flushing dirty (%s) [dsz=%u,qsz=%u] key=%s ptr=%p bsz=%u",
                           reason, dhc, cqc, di->key.toString().c_str(), dv, written);
        }

        try {
            MutexGuard mg(diskLock_);
            disk_->set(di->key,*serout_->getOwnedBuffer(),serout_->getFreeBuffer());
        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_error(log_,"cannot save key=%s: %s",di->key.toString().c_str(),e.what());
            }
        }

        {
            MutexGuard mg(statLock_);
            ++pfset_;
            bset_ += written;
        }

        // save done, cleanup
        {
            MutexGuard mg(dirtyMon_);
            if ( di->changes != oldChanges ) {
                if (log_) {
                    smsc_log_debug(log_,"key=%s is still dirty after flush",
                                   di->key.toString().c_str());
                }
                di->addTime = now;
                *dirtyHash_.GetPtr(di->key) = 
                    dirtyList_.insert(dirtyList_.end(),*di);
            } else {
                cleanQueue_.Push(*di);
                if (log_) {
                    smsc_log_debug(log_,"key=%s is clean now, qsz=%u",
                                   di->key.toString().c_str(),unsigned(cleanQueue_.Count()));
                }
                dirtyHash_.Delete(di->key);
            }
            dirtyList_.erase(di);
            if (moreToFlush_>0) --moreToFlush_;
        }
        return written;
    }


    void flushIOStatistics( unsigned& pfget,
                            unsigned& kbget,
                            unsigned& pfset,
                            unsigned& kbset )
    {
        MutexGuard mg(statLock_);
        pfget += pfget_;
        kbget += bget_ / 1024;
        pfset += pfset_;
        kbset += bset_ / 1024;
        pfget_ = bget_ = pfset_ = bset_ = 0;
    }


    // NOTE: it is your responsibility to delete the return value.
    value_type* release( const key_type& k, bool fromdiskalso = false ) {
        throw smsc::util::Exception("release is not allowed, key=%s",k.toString().c_str());
    }
    /*
        typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
        if (di) {
            typename DirtyList::iterator i = *di;
            value_type* res = 0;
            if (cache_->store2val(i->stored)) {
                if (fromdiskalso) {
                    disk_->remove(k);
                } else {
                    doFlush(k,i->stored);
                }
                res = cache_->setval(i->stored,0);
            }
            cache_->dealloc(i->stored);
            dirtyList_.erase(i);
            dirtyHash_.Delete(k);
            return res;
        }
        stored_type sv = cache_->release(k);
        if ( cache_->store2val(sv) && fromdiskalso ) disk_->remove( k );
        value_type* res = cache_->setval(sv,0);
        cache_->dealloc(sv);
        return res;
    }
     */


    /// clean cache only, it will destroy all items
    void clean() {
        cache_->clean();
    }


    class Iterator {
    protected:
        Iterator( const MemStorage& cache ) : cache_(&cache), iter_(cache.begin()) {}
        friend class CachedDelayedThreadedDiskStorage< MemStorage, DiskStorage, Allocator >;
    public:
        ~Iterator() {}
        // void reset() { iter_ = cache_->begin(); }
        bool next( key_type& k, value_type*& v ) {
            // stored_type x;
            bool res = iter_.next();
            if ( res ) {
                k = iter_.key();
                v = cache_->store2val(iter_.value());
            }
            return res;
        }
    private:
        const MemStorage*                  cache_;
        typename MemStorage::iterator_type iter_;
    };
    typedef Iterator iterator_type;

    /// NOTE: iterator is only for cached data
    iterator_type begin() const {
        return iterator_type(*cache_);
    }


    /// iterator on the disk data.
    typename DiskStorage::iterator_type dataBegin() const {
        return disk_->begin();
    }


    /// return the number of items on disk
    unsigned long dataSize() const {
        return disk_->size();
    }


    /// return the number of filled items on disk
    unsigned long filledDataSize() const {
        return disk_->filledSize();
    }


    /// for statistics
    unsigned int hitcount() const {
        return hitcount_;
    }


private:

    inline bool addDirty( const key_type& k, stored_type sv )
    {
        // util::msectime_type now = util::currentTimeMillis();
        if ( unsigned(dirtyHash_.Count()) > fc_.flushHighMark ) {
            if (log_) {
                smsc_log_debug(log_,"dirty queue is filled, sz=%u", unsigned(dirtyHash_.Count()));
            }
            return false;
        }
        if (log_) {
            smsc_log_debug(log_,"adding dirty key=%s",k.toString().c_str());
        }
        typename DirtyList::iterator di = dirtyList_.insert(dirtyList_.end(),Dirty());
        di->key = k;
        di->stored = sv;
        di->changes = 0;
        di->addTime = util::currentTimeMillis();
        dirtyHash_.Insert(k,di);
        dirtyMon_.notify();
        return true;
    }


    stored_type faultHandler( const key_type& k, bool create ) const
    {
        if (!cache_->store2val(spare_)) spare_ = cache_->val2store( alloc(k) );
        bool itemLoaded;
        {
            MutexGuard mg(diskLock_);
            itemLoaded = disk_->get(k,*serin_->getFreeBuffer(true));
        }
        {
            MutexGuard mg(statLock_);
            ++pfget_;
            bget_ += serin_->getFreeBuffer()->size();
        }
        if ( itemLoaded ) {
            stored_type& ref = cache_->store2ref(spare_);
            if (!serin_->deserialize(k,ref) && !create ) {
                return cache_->val2store(0);
            }
        } else if (!create) {
            return cache_->val2store(0);
        }
        stored_type v = spare_;
        spare_ = cache_->val2store(0);
        return v;
    }


    stored_type popCleanQ( const key_type& k ) {
        Dirty d;
        while ( cleanQueue_.Pop(d) ) {
            if ( d.key == k ) {
                if (log_) {
                    smsc_log_debug(log_,"cleanq pop key=%s val=%p",k.toString().c_str(),cache_->store2val(d.stored));
                }
                return d.stored;
            }
            cache_->set(d.key,d.stored);
        }
        return cache_->store2val(0);
    }

    /*
    void startCleaner() {
        Start();
    }

    void stopCleaner() {
        if (inited_) {
            {
                MutexGuard mg(dirtyMon_);
                inited_ = false;
                dirtyMon_.notifyAll();
            }
            WaitFor();
        }
    }
     */

    CachedDelayedThreadedDiskStorage( const CachedDelayedThreadedDiskStorage& );

private:
    smsc::core::synchronization::EventMonitor  dirtyMon_;
    mutable smsc::core::synchronization::Mutex diskLock_;
    mutable smsc::core::synchronization::Mutex statLock_;
    mutable MemStorage*  cache_;
    DiskStorage*         disk_;
    Serializer*          serin_;
    Serializer*          serout_;

    /// a queue to return entries from dirtyHash_ to cache_.
    smsc::core::buffers::CyclicQueue< Dirty > cleanQueue_;

    // NOTE: for optimization purposes we store a pointer to a temporary instance.
    //  it is used only in faultHandler to avoid unnecessary new/delete
    //  when item is not found.
    mutable stored_type spare_;
    mutable unsigned int hitcount_;

    mutable unsigned pfget_;
    mutable unsigned bget_;
    unsigned         pfset_;
    unsigned         bset_;

    FlushConfig           fc_;
    smsc::logger::Logger* log_;

    DirtyList           dirtyList_;
    DirtyHash           dirtyHash_;
    unsigned            moreToFlush_;
};

}
}
}

#endif
