#ifndef _SCAG_UTIL_STORAGE_STORAGEIFACE_H
#define _SCAG_UTIL_STORAGE_STORAGEIFACE_H

#include <memory>
#include <stdexcept>
#include "logger/Logger.h"

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
///     void setIndex( const key_type&, index_type);
/// };
/// DStorage {
///     index_type = POD integral type (?);
///     value_type;
///     bool read( index_type ) const;         // into internal (mutable) buffer
///     bool deserialize( value_type& ) const; // from intern. buffer
///     ~DStorage();
///     serialize( const value_type& );
///     remove( index_type );
///     copyBuffer( OtherDStorage& );          // copy internal buffer
///     index_type append();                   // internal buffer
///     remove( index_type );
/// };
template < class IStorage, class DStorage >
class IndexedStorage
{
public:
    typedef typename IStorage::key_type          key_type;
    typedef typename IStorage::index_type        index_type;
    typedef typename DStorage::value_type        value_type;

    class Iterator {
        friend class IndexedStorage< IStorage, DStorage >;
    public:
        void reset() { iter_ = s_->index_->begin(); }
        // FIXME: index_type return is temporary
        bool next( key_type& k, index_type& i, value_type& v ) {
            // index_type i;
            while ( iter_.next(k,i) ) {
                if ( i && s_->data_->read(i) && s_->data_->deserialize(v) ) {
                    return true;
                }
            }
            return false;
        }

    private:
        Iterator( const IndexedStorage< IStorage, DStorage >& s ) :
        s_(&s), iter_(s.index_->begin()) {}
        Iterator() {}
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
    }


    ~IndexedStorage() {
        delete index_;
        delete data_;
    }


    unsigned long size() const {
        return index_->size();
    }


    bool set( const key_type& k, const value_type& v ) {
        data_->serialize( v ); // into internal buffer
        return do_set( k );
    }


    bool get( const key_type& k, value_type& v ) const {
        index_type i = index_->getIndex( k );
        if ( i && data_->read(i) && data_->deserialize(v) )
            return true;
        return false;
    }


    bool has( const key_type& k ) const {
        return ( index_->getIndex( k ) != 0 );
    }


    bool remove( const key_type& k ) {
        index_type i = index_->removeIndex( k );
        if ( i != 0 ) {
            data_->remove( i );
            return true;
        }
        return false;
    }


    /// NOTE: this method gives some optimization
    /// by avoiding extra deserialization/serialization steps.
    template < class IS, class DS > bool copy
        ( const key_type& k,
          const IndexedStorage<IS,DS>& s )
    {
        if ( &s != this ) {
            index_type i = s.index_->getIndex( k );
            if ( i == 0 || !s.data_->read(i) ) return false;
            data_->copyBuffer( s.data_ );
            do_set( k );
        }
        return false;
    }

    // iteration
    iterator_type begin() const {
        return iterator_type( *this );
    }

private:

    /// set internal buffer to the data storage
    bool do_set( const key_type& k ) {
        index_type i = index_->getIndex( k );
        if ( i == 0 ) {
            i = data_->append(); // from internal buffer
            index_->setIndex( k, i );
            return true;
        } else {
            index_type j = data_->append();
            index_->setIndex( k, j );
            data_->remove( i );
            return false;
        }
    }

private:
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


template < class Key, class Val >
class SimpleCachedStorageAllocator
{
protected:
    ~SimpleCachedStorageAllocator() {}
    inline Val* alloc( const Key& k ) const { return new Val(k); }
};


template < class MemStorage, class DiskStorage,
        template <class,class> class Allocator = SimpleCachedStorageAllocator >
class CachedDiskStorage :
protected Allocator<
        typename MemStorage::key_type,
        typename MemStorage::value_type >
{
public:
    // typedef MemStorage                   memstorage_type;
    // typedef DiskStorage                  diskstorage_type;
    // typedef MemoryStorage< typename MemStorage::key_type, typename MemStorage::value_type >  Base;
    typedef typename MemStorage::key_type       key_type;
    typedef typename MemStorage::value_type     value_type;
    typedef typename MemStorage::stored_type    stored_type;

    CachedDiskStorage( MemStorage* ms,
                       DiskStorage* ds ) :
    cache_( ms ), disk_( ds ), hitcount_(0), cachelog_(NULL)
    {
        if ( !ms || !ds ) {
            delete ms;
            delete ds;
            throw std::runtime_error("CachedDiskStorage: both storages should be provided!");
        }
        spare_ = cache_->val2store(NULL);
        cachelog_ = smsc::logger::Logger::getInstance("memcache");
    }

    ~CachedDiskStorage() {
        flush();
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

    value_type* get( const key_type& k ) const {
        stored_type v = cache_->get( k );
        if ( cache_->store2val(v) ) 
            ++this->hitcount_;
        else {
            v = faultHandler( k );
            if ( cache_->store2val(v) ) cache_->set(k,v);
        }
        return cache_->store2val(v);
    }

    /// NOTE: it is your responsibility to delete the return value.
    value_type* release( const key_type& k ) {
        std::auto_ptr<value_type> v( cache_->release(k) );
        disk_->remove( k );
        return v.release();
    }

    /// purge element from cache only
    /// NOTE: it is your responsibility to delete the return value.
    value_type* purge( const key_type& k ) {
        return cache_->release(k);
    }

    /// flush all cached data to disk
    /// @return number of flushed items
    unsigned int flush() {
        key_type k;
        stored_type v;
        unsigned int count = 0;
        smsc_log_debug( cachelog_, "FLUSH STARTED" );
        for ( iterator_type i = this->begin();
              i.next(k,v); ) {
            if ( cache_->store2val(v) ) {
                ++count;
                /*
                smsc_log_debug( cachelog_, "item: key=%s valkey=%s",
                                k.toString().c_str(),
                                v->getKey().toString().c_str() );
                if ( k != v->getKey() )
                    smsc_log_warn( cachelog_, "WARNING: key mismatch!" );
                 */
                disk_->set(k,cache_->store2ref(v));
                // tatic_cast< const typename DiskStorage::value_type& >( *v ) );
            }
        }
        smsc_log_debug( cachelog_, "FLUSH FINISHED, count=%d", count );
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
        void reset() { iter_ = cache_->begin(); }
        bool next( key_type& k, value_type*& v ) {
            stored_type x;
            bool res = iter_.next(k,x);
            if ( res ) v = cache_->store2val(x);
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
    /// FIXME: should we allow this one?
    typename DiskStorage::iterator_type dataBegin() const {
        return disk_->begin();
    }


    /// return the number of items on disk
    unsigned long dataSize() const {
        return disk_->size();
    }


    /// for statistics
    unsigned int hitcount() const {
        return hitcount_;
    }

private:
    stored_type faultHandler( const key_type& k ) const
    {
        if (!cache_->store2val(spare_)) spare_ = cache_->val2store( alloc(k) );
        // new typename DiskStorage::value_type( k );
        if ( disk_->get(k,cache_->store2ref(spare_)) ) {
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

#endif /* !_SCAG_UTIL_STORAGE_STORAGEIFACE_H */
