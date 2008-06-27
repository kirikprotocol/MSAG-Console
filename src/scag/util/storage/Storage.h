#ifndef _SCAG_UTIL_STORAGE_STORAGE_H
#define _SCAG_UTIL_STORAGE_STORAGE_H


#include <exception>
#include <memory>
#include <vector>

#include "RBTree.h"
#include "RBTreeHSAllocator.h"
#include "StorageIface.h"
#include "core/buffers/PageFile.hpp"

namespace scag {
namespace util {
namespace storage {

using smsc::core::buffers::File;
using smsc::core::buffers::PageFile;

class PageFileBuffer
{
public:
    PageFileBuffer() : v_() {}
    ~PageFileBuffer() {}

    inline const unsigned char* data() const {
        return &(v_.front());
    }
    
    inline const size_t size() const {
        return v_.size();
    }

    inline void reserve( size_t newsize ) {
        v_.reserve( newsize );
    }

    /// used for reading from PageFile
    inline std::vector< unsigned char >& buffer() {
        return v_;
    }

    inline void swap( PageFileBuffer& b ) {
        v_.swap( b.v_ );
    }

private:
    std::vector< unsigned char > v_;
};




/// implementation of storage ifaces
template < class Val >
class PageFileDiskStorage // : public DataStorageIface< File::offset_type, Val >
{
public:
    // typedef DataStorageIface< File::offset_type, Val >   Base;
    typedef File::offset_type         index_type;
    typedef Val                       value_type;

    PageFileDiskStorage( PageFile* pf ) : pf_(pf) {
        if ( ! pf_ )
            throw std::runtime_exception( "PageFileDiskStorage: pagefile should be provided!" );
    }

    ~PageFileDiskStorage() {
        delete pf_;
    }

    /// serialize the value into an internal buffer
    void serialize( const value_type& v ) {
        v.Serialize( b_ );
    }
        
    /// append data from internal buffer to the storage
    index_type append() {
        pf_->Append( b_.data(), b_.size() );
    }

    /// update data from internal buffer to the storage
    void update( index_type i ) {
        pf_->Update( i, b_.data(), b_.size() );
    }

    /// read data from storage into internal (mutable) buffer
    void read( index_type i ) const {
        pf_->Read( i, b_.vector(), NULL );
    }

    /// deserialize value from internal buffer
    /// @return true if successfully deserialized, otherwise v is broken
    bool deserialize( value_type& v ) const {
        v.Deserialize( b_ );
    }

    /// delete data from the store
    void delete( index_type i ) {
        pf_->Delete( i );
    }

    /// copy internal buffer from another storage
    void copyBuffer( const DataStorageIface<Idx,Val>& s ) {
        b_.swap( s.b_ );
    }

private:
    PageFile* pf_;     // owned
    mutable PageFileBuffer b_;
};


/// index disk storage
template < class Key, class Idx >
class RBTreeIndexStorage
{
private:
    typedef RBTree<Key,Idx>                       IndexStorage;    // transient
    typedef RBTreeHSAllocator<Key,Idx>            IndexAllocator;
    typedef typename IndexStorage::RBTreeNode     IndexNode;

public:
    typedef Key key_type;
    typedef Idx index_type;

    RBTreeIndexStorage( const std::string& dbname,
                        const std::string& dbpath = ".",
                        int indexGrowth = 1000000,
                        bool cleanup = false )
    {
        allocator_.set( new IndexAllocator() );
        allocator_->Init( dbpath + '/' + dbname + '/' + dbname + "-index",
                          indexGrowth,
                          cleanup );

        index_->SetAllocator( allocator_.get() );
        index_->SetChangesObserver( allocator_.get() );
        
    }

    ~RBTreeIndexStorage() {
    }

    /// @return the index of the key, or 0 if not found
    index_type getIndex( const key_type& k ) const {
        IndexNode* node = getNode( k );
        return node ? node->value : 0;
    }
    
    /// set the index of the key with possible replacement.
    /// @return false if previous value was replaced.
    bool setIndex( const key_type& k, index_type i ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_->setNodeValue( node, i );
        } else {
            index_->Insert( k, i );
        }
    }

    /// remove index from the storage.
    /// @return the value of removed index.
    index_type removeIndex( const key_type& k ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_type i = node->value;
            index_->setNodeValue( node, 0 );
            return i;
        }
        return 0;
    }

private:
    inline IndexNode* getNode( const key_type& k ) const
    {
        if ( cache_ && cache_->key == k ) {
            return cache_;
        } else {
            IndexNode* node = index_->Get( k );
            if ( node ) cache_ = node;
            return node;
        }
    }


private:
    IndexStorage                    index_;
    std::auto_ptr< IndexAllocator > allocator_;
    IndexNode*                      cache_;  // for successive get/set
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_STORAGE_H */
