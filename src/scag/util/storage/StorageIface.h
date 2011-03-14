#ifndef _SCAG_UTIL_STORAGE_STORAGEIFACE_H
#define _SCAG_UTIL_STORAGE_STORAGEIFACE_H

#include <memory>
#include <stdexcept>
#include "logger/Logger.h"
#include "HeapAllocator.h"

namespace scag {
namespace util {
namespace storage {

/*
template < class Key, class Val >
class StorageIface
{
public:
    typedef Key key_type;
    typedef Val value_type;

    virtual ~StorageIface() {}

    /// set the item in storage.
    /// @return false if item has been replaced.
    virtual bool set( const key_type& k, const value_type& v ) = 0;

    /// get item from storage.
    /// @return true if found.
    virtual bool get( const key_type& k, value_type& v ) const = 0;
    
    /// check if the storage has an item with given key.
    virtual bool has( const key_type& k ) const = 0;

    /// remove the item from storage.
    /// @return true if the item has been found.
    virtual bool remove( const key_type& k ) = 0;

    // copy element from s
    // NOTE: work is a working place to avoid using a ctor inside
    // virtual bool copy( const key_type& k,
    // const StorageIface<Key,Val>& s,
    // value_type& work ) = 0;

};

template < typename Key, typename Val >
struct InplaceIterator
{
public:
    typedef Key key_type;
    typedef Val value_type;
    virtual ~InplaceIterator() {}
    virtual bool reset() = 0;
    virtual bool next( key_type& k, value_type& v ) = 0;
};

// interface of the index storage
template < class Key, typename Idx >
class IndexStorageIface
{
public:
    typedef Key key_type;
    typedef Idx index_type;

    virtual ~IndexStorageIface() {}

    /// @return the index of the key, or 0 if not found
    virtual index_type getIndex( const key_type& k ) const = 0;
    
    /// set the index of the key with possible replacement.
    /// @return false if previous value was replaced.
    virtual bool setIndex( const key_type& k, index_type i ) = 0;

    /// remove index from the storage.
    /// @return the value of removed index.
    virtual index_type removeIndex( const key_type& k ) = 0;

};


template < typename Idx, class Val >
class DataStorageIface
{
public:
    typedef Idx index_type;
    typedef Val value_type;

    virtual ~DataStorageIface() {}

    /// serialize the value into an internal buffer
    virtual void serialize( const value_type& v ) = 0;

    /// append data from internal buffer to the storage
    virtual index_type append() = 0;

    /// update data from internal buffer to the storage
    virtual void update( index_type i ) = 0;

    /// read data from storage into internal (mutable) buffer
    virtual void read( index_type i ) const = 0;

    /// deserialize value from internal buffer
    /// @return true if successfully deserialized, otherwise v is broken
    virtual bool deserialize( value_type& v ) const = 0;

    /// remove data from the store
    virtual void remove( index_type i ) = 0;

    /// copy internal buffer from another storage
    virtual void copyBuffer( const DataStorageIface<Idx,Val>& s ) = 0;

};
*/


class EmptyMutex {
public:
    inline void Lock() {}
    inline void Unlock() {}
};


template < class T >
class MutexCopyGuardTmpl
{
public:
    MutexCopyGuardTmpl() : lock_(0) {}
    MutexCopyGuardTmpl( T& l ) : lock_(&l) { lock_->Lock(); }
    ~MutexCopyGuardTmpl() { if (lock_) lock_->Unlock(); }
    MutexCopyGuardTmpl< T >& operator = ( const MutexCopyGuardTmpl< T >& o ) {
        if ( this != &o ) {
            if ( lock_ ) lock_->Unlock();
            lock_ = o.lock_;
            o.lock_ = 0;
        }
        return *this;
    }

    MutexCopyGuardTmpl( const MutexCopyGuardTmpl< T >& o ) : lock_(o.lock_) {
        o.lock_ = 0;
    }
protected:
    mutable T* lock_;
};


namespace details {

// indexed storage updater
template < class IS, bool UPD = false >
    struct IndexedStorageUpdater
{
    typedef typename IS::key_type   key_type;
    typedef typename IS::index_type index_type;
    
    inline static bool update( IS& is, const key_type& k, index_type i )
    {
        index_type j = is.data_->append();
        if ( j != is.data_->invalidIndex() )
            is.index_->setIndex(k,j);
        else
            is.index_->removeIndex(k);
        is.data_->remove(i);
        return (j != is.data_->invalidIndex());
    }

};

template < class IS >
    struct IndexedStorageUpdater< IS, true >
{
    typedef typename IS::key_type   key_type;
    typedef typename IS::index_type index_type;

    inline static bool update( IS& is, const key_type& k, index_type i )
    {
        index_type j = is.data_->update(i);
        if ( j != is.data_->invalidIndex() ) {
            is.index_->setIndex(k,j);
            return true;
        } else {
            is.index_->removeIndex(k);
            return false;
        }
    }
};

} // namespace details


/// Indexed storage template class.
/// Template parameters are IStorage -- a storage for indices
/// and DStorage -- a storage for data.
/// Requirements on the template parameters:
/// IStorage {
///     key_type;
///     index_type = POD integral type (?);
///     iterator_type {
///       bool next( key_type&, index_type& );
///     }
///     ~IStorage();
///     iterator_type begin() const;
///     unsigned long size() const;
///     index_type getIndex( const key_type& ) const;
///     index_type removeIndex( const key_type& );
///     bool setIndex( const key_type&, index_type);
///     void setInvalidIndex( index_type );    // to set invalid index
/// };
/// DStorage {
///     static const bool updatable;           // updata policy
///     index_type = POD integral type (?);
///     value_type;
///     bool read( index_type ) const;         // into internal (mutable) buffer
///     bool deserialize( value_type& ) const; // from intern. buffer
///     ~DStorage();
///     setKey( ... );
///     serialize( const value_type& );
///     append( index_type );
///     update( index_type );                  // required only if updatable = true
///     copyBuffer( OtherDStorage& );          // copy internal buffer
///     index_type append();                   // internal buffer
///     index_type invalidIndex() const;       // return invalid index
///     remove( index_type );
/// };
template < class IStorage, class DStorage, class LockType = EmptyMutex >
class IndexedStorage
{
    typedef IndexedStorage< IStorage, DStorage, LockType >  Self;
    friend class details::IndexedStorageUpdater< Self, DStorage::updatable >;
private:
    typedef typename IStorage::index_type        index_type;
public:
    typedef typename IStorage::key_type          key_type;
    typedef typename DStorage::value_type        value_type;
    typedef MutexCopyGuardTmpl< LockType >       mutexguard_type;

    /// NOTE: please use external locking for iteration!
    class Iterator {
        friend class IndexedStorage< IStorage, DStorage >;
    public:
        void reset() { if (s_) iter_ = s_->index_->begin(); }
        bool next( key_type& k, value_type& v ) {
            if ( !s_ ) return false;
            // index_type i;
            while ( iter_.next() ) {
                k = iter_.key();
                index_type i = iter_.idx();
                if ( i != s_->data_->invalidIndex() && 
                     s_->data_->read(i) &&
                     s_->data_->deserialize(v) ) {
                    return true;
                }
            }
            return false;
        }

        Iterator() : s_(NULL) {}
    private:
        Iterator( const IndexedStorage< IStorage, DStorage >& s ) :
        s_(&s), iter_(s.index_->begin()) {}
    private:
        const IndexedStorage< IStorage, DStorage >* s_;
        typename IStorage::iterator_type            iter_;
    };
    friend class Iterator;
    typedef Iterator                             iterator_type;

public:
    IndexedStorage( IStorage* is, DStorage* ds ) :
    index_(is), data_(ds) {
        if ( !is || !ds ) {
            delete is;
            delete ds;
            throw std::runtime_error("IndexedStorage: both storages should be provided!");
        }
        index_->setInvalidIndex( data_->invalidIndex() );
    }


    ~IndexedStorage() {
        delete index_;
        delete data_;
    }


    unsigned long size() const {
        mutexguard_type mg(lock_);
        return index_->size();
    }


    unsigned long filledSize() const {
        mutexguard_type mg(lock_);
        return index_->filledSize();
    }


    // return true if successful
    bool set( const key_type& k, const value_type& v ) {
        mutexguard_type mg(lock_);
        data_->setKey(k);
        data_->serialize(v); // into internal buffer
        return do_set( k );
    }


    bool get( const key_type& k, value_type& v ) const {
        mutexguard_type mg(lock_);
        index_type i = index_->getIndex( k );
        if ( i != data_->invalidIndex() &&
             data_->setKey(k) && 
             data_->read(i) && 
             data_->deserialize(v) )
            return true;
        return false;
    }


    bool has( const key_type& k ) const {
        mutexguard_type mg(lock_);
        return ( index_->getIndex(k) != data_->invalidIndex() );
    }


    bool remove( const key_type& k ) {
        mutexguard_type mg(lock_);
        index_type i = index_->removeIndex( k );
        if ( i != data_->invalidIndex() ) {
            data_->setKey( k );
            data_->remove( i );
            return true;
        }
        return false;
    }


    bool recoverFromBackup( value_type& v ) {
        return data_->recoverFromBackup(v);
    }


    /// NOTE: this method gives some optimization
    /// by avoiding extra deserialization/serialization steps.
    /// NOTE: use external locking for this method!
    template < class IS, class DS > bool copy
        ( const key_type& k,
          const IndexedStorage<IS,DS>& s )
    {
        if ( &s != this ) {
            index_type i = s.index_->getIndex(k);
            if ( i != data_->invalidIndex() &&
                 s.data_->setKey(k) &&
                 s.data_->read(i) ) {
                data_->setKey(k);
                data_->copyBuffer( s.data_ );
                return do_set(k);
            }
        }
        return false;
    }

    // iteration
    iterator_type begin() const {
        return iterator_type( *this );
    }


    /// obtain lock on the storage for iteration, copying, etc.
    mutexguard_type getLock() const
    {
        return mutexguard_type(lock_);
    }

private:

    /// set internal buffer to the data storage
    bool do_set( const key_type& k ) {
        index_type i = index_->getIndex( k );
        if ( i != data_->invalidIndex() ) {
            return details::IndexedStorageUpdater< Self, DStorage::updatable >::update( *this, k, i );
        } else {
            i = data_->append(); // from internal buffer
            if ( i != data_->invalidIndex() ) return index_->setIndex(k,i);
        }
        return false;
    }


private:
    mutable LockType   lock_;
    IStorage*  index_;
    DStorage*  data_;
};



/*
/// class storing transient objects in memory.
/// access is provided by key only.
/// it owns all its objects!
/// to delete some special object use:
///    delete store.release( key );
template < class Key, class Val >
class MemoryStorage : Iterable< InplaceIterator<Key, Val*> >
{
public:
    typedef Key                           key_type;
    typedef Val                           value_type;
    typedef InplaceIterator< Key, Val* >  iterator_type;

    virtual ~MemoryStorage() {}

    /// set the item (with possible replacement)
    /// NOTE: item gets owned
    virtual bool set( const key_type& k, value_type* v ) = 0;

    /// get the item
    virtual value_type* get( const key_type& k ) const = 0;

    /// release the item
    virtual value_type* release( const key_type& k ) = 0;
};
*/


template < class MemStorage, class DiskStorage,
        class Allocator = HeapAllocator< typename MemStorage::key_type, typename MemStorage::value_type > >
class CachedDiskStorage : public Allocator
{
public:
    // typedef MemStorage                   memstorage_type;
    // typedef DiskStorage                  diskstorage_type;
    // typedef MemoryStorage< typename MemStorage::key_type, typename MemStorage::value_type >  Base;
    typedef typename MemStorage::key_type       key_type;
    typedef typename MemStorage::value_type     value_type;
    typedef typename MemStorage::stored_type    stored_type;

    CachedDiskStorage( MemStorage* ms,
                       DiskStorage* ds,
                       smsc::logger::Logger* thelog = 0 ) :
    cache_( ms ), disk_( ds ), hitcount_(0), cachelog_(thelog)
    {
        if ( !ms || !ds ) {
            delete ms;
            delete ds;
            throw std::runtime_error("CachedDiskStorage: both storages should be provided!");
        }
        spare_ = cache_->val2store(NULL);
        // cachelog_ = smsc::logger::Logger::getInstance("memcache");
    }


    ~CachedDiskStorage() {
        //flushAll();
        cache_->dealloc(spare_);
        cache_->clean();
        delete disk_;
        delete cache_;
    }

    /*
    value_type* create( const key_type& k ) const {
        value_type* = new DiskStorage::value_type( k );
        if ( v ) cache_->set( k, v );
        return v;
    }
     */


    bool set( const key_type& k, value_type* v ) {
        return cache_->set(k, cache_->val2store(v) );
    }

    value_type* get( const key_type& k, bool create = false ) const {
        stored_type* const vv = cache_->get( k );
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

    /// NOTE: use only if type of stored_type is DataBlockBackup
    void backup2Profile ( const key_type& k ) {
        stored_type* const vv = cache_->get( k );
        if ( !vv ) {return;}
        if ( !vv->value || !vv->backup ||
             !disk_->recoverFromBackup(cache_->store2ref(*vv)) ) {
            // we have to delete such an entry from cache
            delete cache_->release(k);
        }
    }

    /// flush item to disk
    bool flush( const key_type& k ) {
        const stored_type* vv = cache_->get(k);
        if ( vv && cache_->store2val(*vv) )
            return disk_->set( k, cache_->store2ref(*vv) );
        return false;
    }

    //TODO: flush without cache_->get()


    /// NOTE: it is your responsibility to delete the return value.
    value_type* release( const key_type& k, bool fromdiskalso = false ) {
        stored_type sv = cache_->release(k);
        if ( cache_->store2val(sv) && fromdiskalso ) disk_->remove(k);
        value_type* v = cache_->setval(sv,0);
        cache_->dealloc(sv);
        return v;
    }


    /// flush all cached data to disk
    /// @return number of flushed items
    unsigned int flushAll() {
        // key_type k;
        // stored_type v;
        unsigned int count = 0;
        if (cachelog_) { smsc_log_debug( cachelog_, "FLUSH STARTED" ); }
        // cache_->preflush();
        for ( typename MemStorage::iterator_type i = cache_->begin();
              i.next(); ) {
            if ( cache_->store2val(i.value()) ) {
                ++count;
                /*
                smsc_log_debug( cachelog_, "item: key=%s valkey=%s",
                                k.toString().c_str(),
                                v->getKey().toString().c_str() );
                if ( k != v->getKey() )
                    smsc_log_warn( cachelog_, "WARNING: key mismatch!" );
                 */
                disk_->set( i.key(), cache_->store2ref(i.value()) );
            }
        }
        if (cachelog_) { smsc_log_debug( cachelog_, "FLUSH FINISHED, count=%d", count ); }
        return count;
    }


    /// clean cache only, it will destroy all items
    void clean() {
        cache_->clean();
    }


    class Iterator {
    protected:
        Iterator( const MemStorage& cache ) : cache_(&cache), iter_(cache.begin()) {}
        friend class CachedDiskStorage< MemStorage, DiskStorage, Allocator >;
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
    stored_type faultHandler( const key_type& k, bool create ) const
    {
        if (!cache_->store2val(spare_)) spare_ = cache_->val2store( alloc(k) );
        // new typename DiskStorage::value_type( k );
        if ( disk_->get(k,cache_->store2ref(spare_)) || create ) {
            stored_type v = spare_;
            spare_ = cache_->val2store(NULL);
            return v;
        }
        return cache_->val2store(NULL);
    }

private:
    CachedDiskStorage( const CachedDiskStorage& );

    mutable MemStorage*  cache_;
    DiskStorage* disk_;

    // NOTE: for optimization purposes we store a pointer to a temporary instance.
    //  it is used only in faultHandler to avoid unnecessary new/delete
    //  when item is not found.
    mutable stored_type spare_;
    mutable unsigned int hitcount_;
    smsc::logger::Logger* cachelog_;
};


// template < class MemStorage, class DiskStorage >
//        smsc::logger::Logger* CachedDiskStorage< MemStorage, DiskStorage >::cachelog = NULL;

} // namespace storage
} // namespace util
} // namespace scag

namespace scag2 {
namespace util {
namespace storage {
using namespace scag::util::storage;
}
}
}

#endif /* !_SCAG_UTIL_STORAGE_STORAGEIFACE_H */
