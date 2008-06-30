#ifndef _SCAG_UTIL_STORAGE_STORAGE_H
#define _SCAG_UTIL_STORAGE_STORAGE_H

#include <stdexcept>
#include <memory>
#include <vector>

#include "StorageIface.h"
#include "RBTree.h"
#include "RBTreeHSAllocator.h"
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

    inline void reset() { 
        std::vector< unsigned char > tmp;
        v_.assign( tmp.begin(), tmp.end() );
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



class Serializer
{
public:
    typedef std::vector< unsigned char > Buf;
    Serializer( Buf& b ) : buf_(b), rpos_(0) {}

    Serializer& operator << ( uint8_t );
    Serializer& operator << ( uint16_t );
    Serializer& operator << ( uint32_t );
    Serializer& operator << ( uint64_t );
    Serializer& operator << ( const std::string& );
    Serializer& operator << ( const Buf& );

    class BufferUnderrunException : public std::exception {
    public:
        BufferUnderrunException() {}
        virtual ~BufferUnderrunException() throw () {}
        virtual const char* what() const throw () { return "Serializer has not enough data in buffer"; }
    };

    Serializer& operator >> ( uint8_t& ) throw ( BufferUnderrunException );
    Serializer& operator >> ( uint16_t& ) throw ( BufferUnderrunException );
    Serializer& operator >> ( uint32_t& ) throw ( BufferUnderrunException );
    Serializer& operator >> ( uint64_t& ) throw ( BufferUnderrunException );
    Serializer& operator >> ( std::string& ) throw ( BufferUnderrunException );
    Serializer& operator >> ( Buf& ) throw ( BufferUnderrunException );

    size_t size() const {
        return buf_.size();
    }
    
    size_t rpos() const {
        return rpos_;
    }

    uint32_t checksum( size_t pos1, size_t pos2 ) const;

private:
    // returns an iterator to a chunk of given size
    Buf::iterator ensure( uint32_t chunk );

    /// read from buffer into \ptr.
    void readbuf( unsigned char* ptr, size_t size ) throw ( BufferUnderrunException );
    
    /// 1. checks that buffer has enough data to read;
    /// 2. if not raise an exception.
    inline void rcheck( size_t sz ) const throw ( BufferUnderrunException ) {
        if ( rpos_ + sz > buf_.size() )
            throw BufferUnderrunException();
    }

private:
    // converter
    union {
        uint8_t  bytes[8];
        uint16_t words[4];
        uint32_t longs[2];
        uint64_t quads[1];
    } cvt;

    Buf&   buf_;  // NOTE the reference
    size_t rpos_;
};



class MutableBufferHolder
{
protected:
    MutableBufferHolder() {}
    ~MutableBufferHolder() {}

    void copyBuffer( const MutableBufferHolder& h ) {
        buf.swap( h.buf );
    }

private:
    MutableBufferHolder( const MutableBufferHolder& );
protected:
    mutable PageFileBuffer buf;
};


/// implementation of storage ifaces
template < class Val >
class PageFileDiskStorage : public MutableBufferHolder // : public DataStorageIface< File::offset_type, Val >
{
public:
    // typedef DataStorageIface< File::offset_type, Val >   Base;
    typedef File::offset_type         index_type;
    typedef Val                       value_type;

    PageFileDiskStorage( PageFile* pf ) : pf_(pf) {
        if ( ! pf_ )
            throw std::runtime_error( "PageFileDiskStorage: pagefile should be provided!" );
    }

    ~PageFileDiskStorage() {
        delete pf_;
    }

    /// serialize the value into an internal buffer
    void serialize( const value_type& v ) {
        buf.reset();
        Serializer s( buf );
        s << v;
    }
        
    /// append data from internal buffer to the storage
    index_type append() {
        return pf_->Append( buf.data(), buf.size() );
    }

    /// update data from internal buffer to the storage
    void update( index_type i ) {
        pf_->Update( i, buf.data(), buf.size() );
    }

    /// read data from storage into internal (mutable) buffer
    void read( index_type i ) const {
        buf.reset();
        pf_->Read( i, buf.buffer(), NULL );
    }

    /// deserialize value from internal buffer
    /// @return true if successfully deserialized, otherwise v is broken
    bool deserialize( value_type& v ) const {
        try {
            Serializer s( buf );
            s >> v;
        } catch ( Serializer::BufferUnderrunException& ) {
            return false;
        }
        return true;
    }

    /// delete data from the store
    void remove( index_type i ) {
        pf_->Delete( i );
    }

private:
    PageFile* pf_;     // owned
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
                        bool cleanup = false ) :
    cache_(0)
    {
        allocator_.reset( new IndexAllocator() );
        allocator_->Init( dbpath + '/' + dbname + '/' + dbname + "-index",
                          indexGrowth,
                          cleanup );

        index_.SetAllocator( allocator_.get() );
        index_.SetChangesObserver( allocator_.get() );
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
            index_.setNodeValue( node, i );
            return false;
        } else {
            index_.Insert( k, i );
            cache_ = NULL; // the pointer may be invalidated by reallocation
            return true;
        }
    }

    /// remove index from the storage.
    /// @return the value of removed index.
    index_type removeIndex( const key_type& k ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_type i = node->value;
            index_.setNodeValue( node, 0 );
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
            IndexNode* node = const_cast< IndexStorage& >(index_).Get( k );
            if ( node ) cache_ = node;
            return node;
        }
    }


private:
    IndexStorage                    index_;
    std::auto_ptr< IndexAllocator > allocator_;
    mutable IndexNode*              cache_;  // for successive get/set
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_STORAGE_H */
