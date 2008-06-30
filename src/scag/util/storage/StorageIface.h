#ifndef _SCAG_UTIL_STORAGE_STORAGEIFACE_H
#define _SCAG_UTIL_STORAGE_STORAGEIFACE_H

#include <memory>
#include <stdexcept>

namespace scag {
namespace util {
namespace storage {

#if 0
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
#endif


template < class IStorage, class DStorage > class IndexedStorageIterator;

template < class IStorage, class DStorage >
class IndexedStorageBase
{
public:
    friend class IndexedStorageIterator< IStorage, DStorage >;

    typedef typename IStorage::key_type          key_type;
    typedef typename IStorage::index_type        index_type;
    typedef typename DStorage::value_type        value_type;

protected:
    IndexedStorageBase( IStorage* is, DStorage* ds ) :
    index_(is), data_(ds) {
        if ( !is || !ds ) {
            delete is;
            delete ds;
            throw std::runtime_error("IndexedStorage: both storages should be provided!");
        }
    }
    ~IndexedStorageBase() {
        delete index_;
        delete data_;
    }

public:

    bool set( const key_type& k, const value_type& v ) {
        data_->serialize( v ); // into internal buffer
        return do_set( k );
    }


    bool get( const key_type& k, value_type& v ) const {
        index_type i = index_->getIndex( k );
        if ( i == 0 ) return false;
        data_->read( i ); // into internal buffer
        data_->deserialize( v );
        return true;
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
          const IndexedStorageBase<IS,DS>& s )
    {
        if ( &s != this ) {
            index_type i = s.index_->getIndex( k );
            if ( i == 0 ) return false;
            s.data_->read( i );
            data_->copyBuffer( s );
            do_set( k );
        }
        return false;
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
            data_->update( i ); // from internal buffer
            return false;
        }
    }

private:
    IStorage*  index_;
    DStorage*  data_;
};


template < class IStorage, class DStorage > class IndexedStorage;

template < class IStorage, class DStorage >
class IndexedStorageIterator 
{
public:
    friend class IndexedStorage< IStorage, DStorage >;

private:
    typedef IndexedStorageBase< IStorage, DStorage >  Base;
public:
    typedef typename Base::key_type   key_type;
    typedef typename Base::index_type index_type;
    typedef typename Base::value_type value_type;

    void reset() { iter_ = s_->index_->begin(); }
    bool next( key_type& k, value_type& v ) {
        index_type i;
        if ( ! iter_->next( k, i ) ) return false;
        return s_->data_->get( i, v );
    }

private:
    IndexedStorageIterator( const IndexedStorageBase< IStorage, DStorage >& s ) :
    s_(&s), iter_(s.index_->begin()) {}
    IndexedStorageIterator() {}

private:
    const IndexedStorageBase< IStorage, DStorage >* s_;
    typename IStorage::iterator_type  iter_;
};


template < class IStorage, class DStorage >
class IndexedStorage : public IndexedStorageBase< IStorage, DStorage >
{
public:
    typedef typename IStorage::key_type                  key_type;
    typedef typename IStorage::index_type                index_type;
    typedef typename DStorage::value_type                value_type;
    typedef IndexedStorageIterator< IStorage, DStorage > iterator_type;

    IndexedStorage( IStorage* is, DStorage* ds ) :
    IndexedStorageBase< IStorage, DStorage >(is,ds)
    {
    }

    iterator_type begin() const {
        return iterator_type( *this );
    }

};


#if 0
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
#endif



template < class MemStorage, class DiskStorage >
class CachedDiskStorage
{
public:
    // typedef MemStorage                   memstorage_type;
    // typedef DiskStorage                  diskstorage_type;
    // typedef MemoryStorage< typename MemStorage::key_type, typename MemStorage::value_type >  Base;
    typedef typename MemStorage::key_type       key_type;
    typedef typename MemStorage::value_type     value_type;
    typedef typename MemStorage::iterator_type  iterator_type;
    // typedef typename DiskStorage::iterator_type store_iterator_type;

    CachedDiskStorage( MemStorage* ms,
                       DiskStorage* ds ) :
    cache_( ms ), disk_( ds ), spare_(NULL) {
        if ( !ms || !ds ) {
            delete ms;
            delete ds;
            throw std::runtime_error("CachedDiskStorage: both storages should be provided!");
        }
    }

    ~CachedDiskStorage() {
        flush();
        delete spare_;
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
        return cache_->set(k,v);
    }

    value_type* get( const key_type& k ) const {
        value_type* v = cache_->get( k );
        if ( !v ) v = faultHandler( k );
        return v;
    }

    /// NOTE: it is your responsibility to delete the return value.
    value_type* release( const key_type& k ) {
        std::auto_ptr<value_type> v( cache_->release( k ) );
        // should we remove the item from disk?
        // the answer is yes, as otherwise we could leak disk resources.
        disk_->remove( k );
        return v.release();
    }

    /// purge element from cache only
    /// NOTE: it is your responsibility to delete the return value.
    value_type* purge( const key_type& k ) {
        return cache_->release( k );
    }

    /// flush all cached data to disk
    void flush() {
        key_type k;
        value_type* v;
        for ( iterator_type i = this->begin();
              i.next(k,v); ) {
            if (v) disk_->set( k, static_cast< const typename DiskStorage::value_type& >( *v ) );
        }
    }

    /// NOTE: iterator is only for cached data
    iterator_type begin() const {
        return cache_->begin();
    }

protected:
private:
    value_type* faultHandler( const key_type& k ) const
    {
        if ( !spare_ ) spare_ = new typename DiskStorage::value_type( k );
        if ( disk_->get( k, *spare_ ) ) {
            value_type* v = spare_;
            spare_ = NULL;
            cache_->set( k, v );
            return v;
        }
        return NULL;
    }

private:
    CachedDiskStorage( const CachedDiskStorage& );

    mutable MemStorage*  cache_;
    DiskStorage* disk_;

    // NOTE: for optimization purposes we store a pointer to a temporary instance.
    //  it is used only in faultHandler to avoid unnecessary new/delete
    //  when item is not found.
    mutable typename DiskStorage::value_type*  spare_;
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_STORAGEIFACE_H */
