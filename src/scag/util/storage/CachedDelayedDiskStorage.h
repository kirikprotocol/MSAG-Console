#ifndef _SCAG_UTIL_STORAGE_CACHEDDELAYEDDISKSTORAGE_H
#define _SCAG_UTIL_STORAGE_CACHEDDELAYEDDISKSTORAGE_H

#include "logger/Logger.h"
#include "core/buffers/XHash.hpp"
#include "scag/util/Time.h"

namespace scag2 {
namespace util {
namespace storage {

template < class Key, class Val > class HeapAllocator
{
protected:
    ~HeapAllocator() {}
    inline Val* alloc( const Key& k ) const { return new Val(k); }
};


template <
    class MemStorage,
    class DiskStorage,
    class Serializer,
    class Allocator = HeapAllocator< typename MemStorage::key_type, typename MemStorage::value_type > >
    class CachedDelayedDiskStorage : public Allocator
{
public:
    typedef typename MemStorage::key_type       key_type;
    typedef typename MemStorage::value_type     value_type;
    typedef typename MemStorage::stored_type    stored_type;
    typedef typename MemStorage::hash_function  hash_function;

private:
    // dirty things
    struct Dirty {
        Dirty(util::msectime_type t, key_type k, const stored_type& s) :
        dirtyTime(t), key(k), stored(s) {}
        util::msectime_type dirtyTime;
        key_type            key;
        stored_type         stored;
    };
    typedef std::list< Dirty >  DirtyList;
    typedef smsc::core::buffers::XHash< key_type, typename DirtyList::iterator, hash_function > DirtyHash;

public:
    CachedDelayedDiskStorage( MemStorage* ms,
                              DiskStorage* ds,
                              Serializer* sr,
                              smsc::logger::Logger* thelog = 0 ) :
    cache_(ms), disk_( ds ), ser_(sr), hitcount_(0), log_(thelog), inited_(false),
    lastDirtyFlush_(0), minDirtyTime_(10), maxDirtyTime_(10000), maxDirtyCount_(100)
    {
        if ( !ms || !sr || !ds ) {
            delete ms;
            delete sr;
            delete ds;
            throw smsc::util::Exception("CachedDiskStorage2: all storages and serializer should be provided!");
        }
        spare_ = cache_->val2store(0);
    }


    void init( unsigned minDirtyTime,
               unsigned maxDirtyTime,
               unsigned maxDirtyCount )
    {
        if (inited_) return;
        inited_ = true;
        lastDirtyFlush_ = time(0);
        minDirtyTime_ = std::min(minDirtyTime,1000U);
        maxDirtyTime_ = std::min(std::max(maxDirtyTime,10U),100000U);
        maxDirtyCount_ = std::min(std::max(maxDirtyCount,10U),10000U);
        if ( minDirtyTime_ >= maxDirtyTime_ ) maxDirtyTime_ = minDirtyTime_ + 100U;
        if (log_) {
            smsc_log_debug(log_,"inited minTime: %u, maxTime: %u, maxCount: %u",
                           unsigned(minDirtyTime_), unsigned(maxDirtyTime_),
                           unsigned(maxDirtyCount_));
        }
    }


    ~CachedDelayedDiskStorage() {
        flushDirty( util::currentTimeMillis() + 2*maxDirtyTime_ );
        cache_->dealloc(spare_);
        cache_->clean();
        delete cache_;
        delete ser_;
        delete disk_;
    }

    bool set( const key_type& k, value_type* v ) {
        if (!inited_) {
            if (log_) { smsc_log_warn(log_,"DELAYED STORAGE IS NOT INITED"); }
            return false;
        }
        typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
        if (di) {
            if (log_) { smsc_log_debug(log_,"dirty key=%s updated",k.toString().c_str()); }
            value_type* ov = cache_->setval((*di)->stored,v);
            delete ov;
            return ov;
        }
        stored_type sv = cache_->release(k);
        value_type* ov = cache_->setval(sv,v);
        delete ov;
        addDirty(k,sv);
        return ov;
    }


    value_type* get( const key_type& k, bool create = false ) {
        if (!inited_) {
            if (log_) { smsc_log_warn(log_,"DELAYED STORAGE IS NOT INITED"); }
            return 0;
        }
        // first check in dirty
        typename DirtyList::iterator* di = dirtyHash_.GetPtr(k);
        if (di) {
            if (log_) { smsc_log_debug(log_,"dirty key=%s found",k.toString().c_str()); }
            return cache_->store2val((*di)->stored);
        }
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
        stored_type v = faultHandler(k, create);
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

    /// flush item to disk
    bool flush( const key_type& k ) {
        if (!inited_) {
            if (log_) { smsc_log_warn(log_,"DELAYED STORAGE IS NOT INITED"); }
            return false;
        }
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
    }

    //TODO: flush without cache_->get()

    /// Only necessary for get + modifications
    void markDirty( const key_type& k )
    {
        typename DirtyList::iterator* i = dirtyHash_.GetPtr(k);
        if (!i) {
            stored_type sv = cache_->release(k);
            if (cache_->store2val(sv)) {
                addDirty(k,sv);
            }
        }
        flushDirty();
    }


    /// This method should be invoked periodically!
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


    /// NOTE: it is your responsibility to delete the return value.
    value_type* release( const key_type& k, bool fromdiskalso = false ) {
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

    inline bool doFlush( const key_type& k, stored_type& sv )
    {
        if (log_) {
            smsc_log_debug(log_,"flushing key=%s ptr=%p",
                           k.toString().c_str(),cache_->store2val(sv));
        }
        ser_->serialize(cache_->store2ref(sv));
        return disk_->set(k,*ser_->getOwnedBuffer(),ser_->getFreeBuffer());
    }


    inline void addDirty( const key_type& k, stored_type sv )
    {
        util::msectime_type now = util::currentTimeMillis();
        if (log_) {
            smsc_log_debug(log_,"adding dirty key=%s now=%llu",k.toString().c_str(),now);
        }
        dirtyHash_.Insert(k,dirtyList_.insert(dirtyList_.end(), Dirty(now,k,sv)));
        // flushDirty(now);
    }


    stored_type faultHandler( const key_type& k, bool create ) const
    {
        if (!cache_->store2val(spare_)) spare_ = cache_->val2store( alloc(k) );
        if ( disk_->get(k,*ser_->getFreeBuffer(true)) ) {
            stored_type& ref = cache_->store2ref(spare_);
            if (!ser_->deserialize(ref) && !create ) {
                return cache_->val2store(0);
            }
        } else if (!create) {
            return cache_->val2store(0);
        }
        stored_type v = spare_;
        spare_ = cache_->val2store(0);
        return v;
    }

private:
    CachedDelayedDiskStorage( const CachedDelayedDiskStorage& );

    mutable MemStorage*  cache_;
    DiskStorage*         disk_;
    Serializer*          ser_;

    // NOTE: for optimization purposes we store a pointer to a temporary instance.
    //  it is used only in faultHandler to avoid unnecessary new/delete
    //  when item is not found.
    mutable stored_type spare_;
    mutable unsigned int hitcount_;
    smsc::logger::Logger* log_;
    bool                  inited_;

    DirtyList dirtyList_;
    DirtyHash dirtyHash_;
    util::msectime_type lastDirtyFlush_;
    util::msectime_type minDirtyTime_;    // minimum time to relax after dirty flash
    util::msectime_type maxDirtyTime_;    // maximum time to allow dirty to exist
    unsigned            maxDirtyCount_;   // maximum number of dirty to
};

}
}
}

#endif
