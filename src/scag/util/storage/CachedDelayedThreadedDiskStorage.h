#ifndef _SCAG_UTIL_STORAGE_CACHEDDELAYEDTHREADEDDISKSTORAGE_H
#define _SCAG_UTIL_STORAGE_CACHEDDELAYEDTHREADEDDISKSTORAGE_H

#include "logger/Logger.h"
#include "core/buffers/XHash.hpp"
#include "scag/util/Time.h"
#include "HeapAllocator.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/threads/Thread.hpp"

namespace scag2 {
namespace util {
namespace storage {

template <
    class MemStorage,
    class DiskStorage,
    class Serializer,
    class Allocator = HeapAllocator< typename MemStorage::key_type, typename MemStorage::value_type > >
    class CachedDelayedThreadedDiskStorage : public Allocator, protected smsc::core::threads::Thread
{
public:
    typedef typename MemStorage::key_type       key_type;
    typedef typename MemStorage::value_type     value_type;
    typedef typename MemStorage::stored_type    stored_type;
    typedef typename MemStorage::hash_function  hash_function;

private:
    // dirty things
    struct Dirty {
        Dirty() {}
        Dirty(util::msectime_type t, key_type k, const stored_type& s) :
        dirtyTime(t), key(k), stored(s) {}
        util::msectime_type dirtyTime;
        key_type            key;
        stored_type         stored;
    };
    typedef std::list< Dirty >  DirtyList;
    typedef smsc::core::buffers::XHash< key_type, typename DirtyList::iterator, hash_function > DirtyHash;

public:

    CachedDelayedThreadedDiskStorage( MemStorage* ms,
                                      DiskStorage* ds,
                                      Serializer* srin,
                                      Serializer* srout,
                                      smsc::logger::Logger* thelog = 0 ) :
    cache_(ms), disk_( ds ), serin_(srin), serout_(srout), hitcount_(0), log_(thelog), inited_(false),
    maxSpeed_(10000)
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

    void init( unsigned maxDirtySpeed )
    {
        if (inited_) return;
        inited_ = true;
        // lastDirtyFlush_ = util::currentTimeMillis();
        maxSpeed_ = std::max(maxDirtySpeed,10U);
        if (log_) {
            smsc_log_debug(log_,"inited maxSpeed: %u", unsigned(maxSpeed_));
        }
        startCleaner();
    }


    ~CachedDelayedThreadedDiskStorage() {
        stopCleaner();
        // flushDirty( util::currentTimeMillis() + 2*maxDirtyTime_ );
        cache_->dealloc(spare_);
        cache_->clean();
        delete cache_;
        delete serin_;
        delete serout_;
        delete disk_;
    }

    /// @return true if old value was replaced
    bool set( const key_type& k, value_type* v ) {
        if (!inited_) {
            if (log_) { smsc_log_warn(log_,"DELAYED STORAGE IS NOT INITED"); }
            return false;
        }
        // first check the cache
        stored_type sv = cache_->release(k);
        if ( cache_->store2val(sv) ) {
            // we have old value
            value_type* ov = cache_->setval(sv,v);
            delete ov;
            MutexGuard mg(dirtyMon_);
            // FIXME: adding dirty to dirtyHash_
            addDirty(k,sv);
            return ov;
        }

        MutexGuard mg(dirtyMon_);

        // otherwise we have to check the clean queue
        stored_type qv = popCleanQ(k);
        if ( cache_->store2val(qv) ) {
            value_type* ov = cache_->setval(qv,v);
            delete ov;
            addDirty(k,qv);
            return ov;
        }

        /*
        Dirty d;
        while ( cleanQueue_.Pop(d) ) {
            if ( d.key == k ) {
                // we have found the key
                value_type* ov = cache_->setval(d.stored,v);
                delete ov;
                addDirty(k,d.stored);
                return ov;
            }
            cache_->set(d.key,d.stored);
        }
         */

        // finally, check the dirty hash
        typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
        if (di) {
            if (log_) { smsc_log_debug(log_,"dirty key=%s updated",k.toString().c_str()); }
            value_type* ov = cache_->setval((*di)->stored,v);
            delete ov;
            return ov;
        }
        // nothing found, create new
        addDirty(k,cache_->val2store(v));
        return false;
    }


    value_type* get( const key_type& k, bool create = false ) {
        if (!inited_) {
            if (log_) { smsc_log_warn(log_,"DELAYED STORAGE IS NOT INITED"); }
            return 0;
        }
        /*
        // first check in dirty
        typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
        if (di) {
            if (log_) { smsc_log_debug(log_,"dirty key=%s found",k.toString().c_str()); }
            return cache_->store2val((*di)->stored);
        }
         */
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

    /*
    // NOTE: use only if type of stored_type is DataBlockBackup
    void backup2Profile ( const key_type& k ) {
        stored_type* const vv = cache_->get( k );
        if ( !vv ) {return;}
        if ( !vv->value || !vv->backup ||
             !disk_->recoverFromBackup(cache_->store2ref(*vv)) ) {
            // we have to delete such an entry from cache
            delete cache_->release(k);
        }
    }
     */

    /// flush dirty item to disk
    bool flush( const key_type& k ) {
        if (!inited_) {
            if (log_) { smsc_log_warn(log_,"DELAYED STORAGE IS NOT INITED"); }
            return false;
        }
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
    void markDirty( const key_type& k )
    {
        stored_type sv = cache_->release(k);
        if ( cache_->store2val(sv) ) {
            MutexGuard mg(dirtyMon_);
            addDirty(k,sv);
        } else {
            MutexGuard mg(dirtyMon_);
            // look into clean queue
            stored_type qv = popCleanQ(k);
            if ( cache_->store2val(qv) ) {
                addDirty(k,qv);
            }
        }
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


    /// flush all cached data to disk
    /// @return number of flushed items
    /*
    unsigned int flushAll() {
        // key_type k;
        // stored_type v;
        unsigned int count = 0;
        if (log_) {
            smsc_log_debug( log_, "FLUSH STARTED" );
        }
        // cache_->preflush();
        for ( typename MemStorage::iterator_type i = cache_->begin();
              i.next(); ) {
            if ( cache_->store2val(i.value()) ) {
                ++count;
                ser_->serialize(cache_->store2ref(i.value()));
                disk_->set(i.key(),ser_->getOwnedBuffer(),ser_->getFreeBuffer());
            }
        }
        if (log_) {
            smsc_log_debug( log_, "FLUSH FINISHED, count=%d", count );
        }
        return count;
    }
     */


    /// clean cache only, it will destroy all items
    void clean() {
        cache_->clean();
    }


    class Iterator {
    protected:
        Iterator( const MemStorage& cache ) : cache_(&cache), iter_(cache.begin()) {}
        friend class CachedDelayedDiskStorage< MemStorage, DiskStorage, Allocator >;
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

    /*
    inline bool doFlush( const key_type& k, stored_type& sv )
    {
        if (log_) {
            smsc_log_debug(log_,"flushing key=%s ptr=%p",
                           k.toString().c_str(),cache_->store2val(sv));
        }
        serout_->serialize(k,cache_->store2ref(sv));
        MutexGuard mg(diskLock_);
        return disk_->set(k,*serout_->getOwnedBuffer(),serout_->getFreeBuffer());
    }
     */


    inline void addDirty( const key_type& k, stored_type sv )
    {
        util::msectime_type now = util::currentTimeMillis();
        if (log_) {
            smsc_log_debug(log_,"adding dirty key=%s now=%llu",k.toString().c_str(),now);
        }
        dirtyHash_.Insert(k,dirtyList_.insert(dirtyList_.end(), Dirty(now,k,sv)));
        // flushDirty(now);
        // FIXME: wake up if needed
        dirtyMon_.notify();
    }


    stored_type faultHandler( const key_type& k, bool create ) const
    {
        if (!cache_->store2val(spare_)) spare_ = cache_->val2store( alloc(k) );
        bool itemLoaded;
        {
            MutexGuard mg(diskLock_);
            itemLoaded = disk_->get(k,*serin_->getFreeBuffer(true));
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


    /// NOTE: This method is run by inner thread
    virtual int Execute()
    {
        util::msectime_type now = util::currentTimeMillis();
        util::msectime_type w0 = now-1000;
        const int minWaitTime = 20;
        const int idleWaitTime = 100;
        const unsigned maxDeltaT = 10000;
        unsigned written = 0;
        while ( true ) {
            now = util::currentTimeMillis();
            const unsigned deltat = now - w0;
            const unsigned shouldBeWrt = deltat * maxSpeed_;
            // const unsigned speedkbs = written / deltat;
            typename DirtyList::iterator di;
            unsigned dhc = 0;
            {
                MutexGuard mg(dirtyMon_);

                int wtime = 0;
                if ( !inited_ ) {
                    // we need to stop, no wait
                } else if ( written > shouldBeWrt ) {
                    // too fast, need to wait
                    wtime = int(written/maxSpeed_) - deltat;
                } else {
                    const unsigned maxIdle = idleWaitTime*maxSpeed_;
                    if ( written + maxIdle < shouldBeWrt ) {
                        // we are at idle
                        const unsigned nwr = shouldBeWrt - maxIdle;
                        if (log_) {
                            smsc_log_debug(log_,"too idle: written=%u -> %u",written,nwr);
                        }
                        written = nwr;
                    }
                    if ( dirtyHash_.Count() == 0 ) {
                        wtime = idleWaitTime;
                    }
                }
                if ( wtime > 0 ) {
                    if ( wtime < minWaitTime ) wtime = minWaitTime;
                    dirtyMon_.wait(wtime);
                    continue;
                }

                dhc = unsigned(dirtyHash_.Count());
                if (!dhc) {
                    if (!inited_) break;
                    continue;
                }

                // accessing dirtyList is safe
                di = dirtyList_.begin();
            }

            value_type* dv = cache_->store2val(di->stored);
            assert(dv);
            {
                // NOTE: we have to serialize under the lock
                smsc::core::synchronization::MutexGuardTmpl<value_type> mg(*dv);
                serout_->serialize(di->key,cache_->store2ref(di->stored));
            }
            if (log_) {
                smsc_log_debug(log_,"flushing dirty [dsz=%u,spd=%ukb/s] key=%s ptr=%p bsz=%u",
                               dhc, deltat > 0 ? (written / deltat) : 0U,
                               di->key.toString().c_str(), dv,
                               unsigned(serout_->getOwnedBuffer()->size()));
            }

            try {
                MutexGuard mg(diskLock_);
                disk_->set(di->key,*serout_->getOwnedBuffer(),serout_->getFreeBuffer());
                written += unsigned(serout_->getOwnedBuffer()->size());
            } catch ( std::exception& e ) {
                if (log_) {
                    smsc_log_error(log_,"cannot save key=%s: %s",di->key.toString().c_str(),e.what());
                }
            }

            if ( deltat > maxDeltaT ) {
                const unsigned newdeltat = deltat % maxDeltaT + minWaitTime;
                const unsigned nwr = (written / deltat)*newdeltat;
                if (log_) {
                    smsc_log_debug(log_,"shifting: dt=%d->%d,  written=%u->%u",
                                   deltat, newdeltat, written, nwr);
                }
                written = nwr;
                w0 += deltat - newdeltat;
            }

            // save done, cleanup
            {
                MutexGuard mg(dirtyMon_);
                cleanQueue_.Push(*di);
                dirtyHash_.Delete(di->key);
                dirtyList_.erase(di);
            }

        } // loop while inited
        return 0;
    }


    stored_type popCleanQ( const key_type& k ) {
        Dirty d;
        while ( cleanQueue_.Pop(d) ) {
            if ( d.key == k ) {
                if (log_) smsc_log_debug(log_,"cleanq pop key=%s val=%p",k.toString().c_str(),cache_->store2val(d.stored));
                return d.stored;
            }
            cache_->set(d.key,d.stored);
        }
        return cache_->store2val(0);
    }


    /*
    void flushDirty( util::msectime_type now = 0 )
    {
        if ( !now ) now = util::currentTimeMillis();
        {
            const unsigned dhc = dirtyHash_.Count();
            if ( dhc == 0 ) { return; }
            //if (log_) {
            //smsc_log_debug(log_,"flush dirty check: count=%u, last=%u",
            //dhc, unsigned(now-lastDirtyFlush_));
            //}
            if ( dhc > maxDirtyCount_ ) {}
            else if ( now - lastDirtyFlush_ < minDirtyTime_ ) { return; }
            if (log_) {
                smsc_log_debug(log_,"flushing dirty at %llu: dirties=%u last=%u",
                               now, dhc, unsigned(now-lastDirtyFlush_));
            }
        }
        lastDirtyFlush_ = now;
        do {

            // flushing one element
            typename DirtyList::iterator i = dirtyList_.begin();
            if ( cache_->store2val(i->stored) ) {
                doFlush(i->key,i->stored);
            }
            dirtyHash_.Delete(i->key);
            cache_->set(i->key,i->stored);
            dirtyList_.erase(i);

        } while ( unsigned(dirtyHash_.Count()) > maxDirtyCount_ ||
                  ( !dirtyList_.empty() &&
                    now - dirtyList_.begin()->dirtyTime > maxDirtyTime_ ) );
    }
     */

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

    CachedDelayedThreadedDiskStorage( const CachedDelayedThreadedDiskStorage& );

private:
    smsc::core::synchronization::EventMonitor  dirtyMon_;
    mutable smsc::core::synchronization::Mutex diskLock_;
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
    smsc::logger::Logger* log_;
    bool                  inited_;

    DirtyList           dirtyList_;
    DirtyHash           dirtyHash_;
    unsigned            maxSpeed_;
};

}
}
}

#endif
