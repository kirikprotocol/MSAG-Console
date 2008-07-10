#ifndef _SCAG_UTIL_STORAGE_STORAGE_H
#define _SCAG_UTIL_STORAGE_STORAGE_H

#include "StorageIface.h"
#include "RBTree.h"
#include "RBTreeHSAllocator.h"
#include "core/buffers/PageFile.hpp"
#include "logger/Logger.h"

#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <cassert>

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



class BufferUnderrunException : public std::exception {
public:
    BufferUnderrunException() {}
    virtual ~BufferUnderrunException() throw () {}
    virtual const char* what() const throw () { return "Deserializer has not enough data in buffer"; }
};


class SerializerBase
{
public:
    typedef std::vector< unsigned char > Buf;

protected:
    SerializerBase() {}

    uint32_t dochecksum( const Buf& buf, size_t pos1, size_t pos2 ) const;

    // converter
    union {
        uint8_t  bytes[8];
        uint16_t words[4];
        uint32_t longs[2];
        uint64_t quads[1];
    } cvt;

};


class Serializer : public SerializerBase
{
public:
    Serializer( Buf& b ) : buf_(b) {}

    Serializer& operator << ( uint8_t );
    Serializer& operator << ( uint16_t );
    Serializer& operator << ( uint32_t );
    Serializer& operator << ( uint64_t );
    Serializer& operator << ( const std::string& );
    Serializer& operator << ( const Buf& );

    size_t size() const {
        return buf_.size();
    }
    
    uint32_t checksum( size_t pos1, size_t pos2 ) const {
        return dochecksum( buf_, pos1, pos2 );
    }

private:
    // returns an iterator to a chunk of given size
    Buf::iterator ensure( uint32_t chunk );

private:
    Buf& buf_;  // NOTE the reference
};


class Deserializer : public SerializerBase
{
public:
    Deserializer( const Buf& buf ) : buf_(buf), rpos_(0) {}

    Deserializer& operator >> ( uint8_t& ) throw ( BufferUnderrunException );
    Deserializer& operator >> ( uint16_t& ) throw ( BufferUnderrunException );
    Deserializer& operator >> ( uint32_t& ) throw ( BufferUnderrunException );
    Deserializer& operator >> ( uint64_t& ) throw ( BufferUnderrunException );
    Deserializer& operator >> ( std::string& ) throw ( BufferUnderrunException );
    Deserializer& operator >> ( Buf& ) throw ( BufferUnderrunException );

    size_t size() const {
        return buf_.size();
    }
    
    size_t rpos() const {
        return rpos_;
    }

    uint32_t checksum( size_t pos1, size_t pos2 ) const {
        return dochecksum( buf_, pos1, pos2 );
    }

private:
    /// read from buffer into \ptr.
    void readbuf( unsigned char* ptr, size_t size ) throw ( BufferUnderrunException );
    
    /// 1. checks that buffer has enough data to read;
    /// 2. if not raise an exception.
    inline void rcheck( size_t sz ) const throw ( BufferUnderrunException ) {
        if ( rpos_ + sz > buf_.size() )
            throw BufferUnderrunException();
    }

private:
    const Buf&   buf_;  // NOTE the reference
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
template < class Val, class PF = PageFile >
class PageFileDiskStorage : public MutableBufferHolder // : public DataStorageIface< File::offset_type, Val >
{
public:
    // typedef DataStorageIface< File::offset_type, Val >   Base;
    typedef File::offset_type         index_type;
    typedef Val                       value_type;

    PageFileDiskStorage( PF* pf ) : pf_(pf), disklog_(NULL)
    {
        if ( ! pf_ )
            throw std::runtime_error( "PageFileDiskStorage: pagefile should be provided!" );
        disklog_ = smsc::logger::Logger::getInstance( "disk" );
    }

    ~PageFileDiskStorage() {
        delete pf_;
    }

    /// serialize the value into an internal buffer
    void serialize( const value_type& v ) {
        buf.reset();
        Serializer s( buf.buffer() );
        s << v;
        assert( (buf.size() < 10000) && (buf.buffer().capacity() < 10000) );
        // key_ = v.getKey().toString();
    }
        
    /// append data from internal buffer to the storage
    index_type append( ) {
        const index_type i = pf_->Append( buf.data(), buf.size() );
        // smsc_log_debug( disklog_, "append: index=%llx val=%s",
        // static_cast<unsigned long long>(i), key_.c_str() );
        // key_ = "destroyed";
        return i;
    }

    // update data from internal buffer to the storage
    /*
    index_type update( index_type i ) {
        // smsc_log_debug( disklog_, "update: index=%llx val=%s",
        // static_cast<unsigned long long>(i), key_.c_str() );
        // pf_->Delete( i );
        // i = pf_->Append( buf.data(), buf.size() );
        pf_->Update( i, buf.data(), buf.size() );
        // key_ = "destroyed";
        return i;
    }
     */

    /// read data from storage into internal (mutable) buffer
    bool read( index_type i ) const {
        buf.reset();
        index_type j;
        pf_->Read( i, buf.buffer(), &j );
        if ( i != j ) {
            smsc_log_warn( disklog_, "read: different index read: was=%llx is=%llx",
                           static_cast< unsigned long long >( i ),
                           static_cast< unsigned long long >( j ) );
        }
        assert( buf.size() < 10000 && buf.buffer().capacity() < 10000 );
        return true;
    }

    /// deserialize value from internal buffer
    /// @return true if successfully deserialized, otherwise v is broken
    bool deserialize( value_type& v ) const {
        try {
            Deserializer s( buf.buffer() );
            s >> v;
            // key_ = "destroyed";
        } catch ( BufferUnderrunException& ) {
            return false;
        }
        return true;
    }

    /// delete data from the store
    void remove( index_type i ) {
        // key_ = "destroyed";
        pf_->Delete( i );
    }

private:
    PF* pf_;                            // owned
    // mutable std::string key_;
    smsc::logger::Logger* disklog_;
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
        if ( allocator_->Init( dbpath + '/' + dbname + '/' + dbname + "-index",
                               indexGrowth,
                               cleanup ) < 0 ) {
            throw smsc::util::Exception( "cannot initialize RBTreeAllocator" );
        }

        index_.SetAllocator( allocator_.get() );
        index_.SetChangesObserver( allocator_.get() );
    }

    ~RBTreeIndexStorage() {
    }


    /// number of items
    unsigned long size() const {
        return allocator_->getSize() - 1; // for nilnode
    }


    /// @return the index of the key, or 0 if not found
    index_type getIndex( const key_type& k ) const {
        IndexNode* node = getNode( k );
        return node ? node->value : 0;
    }
    
    /// set the index of the key with possible replacement.
    bool setIndex( const key_type& k, index_type i ) {
        IndexNode* node = getNode( k );
        if ( node ) {
            index_.setNodeValue( node, i );
        } else {
            // the tree may be changed by reallocation
            invalidateCache();
            index_.Insert( k, i );
        }
        return true;
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


    class Iterator {
    public:
        void reset() {
            if (s_) s_->index_.Reset();
        }
        bool next( key_type& k, index_type& i ) {
            return (s_ ? s_->index_.Next( k, i ) : false);
        }
        Iterator() : s_(NULL) {}
        Iterator( const Iterator& s ) : s_(s.s_) {
            const_cast<Iterator&>(s).s_ = NULL;
        }
        Iterator& operator = ( const Iterator& s ) {
            if ( &s != this ) {
                Iterator& ss = const_cast< Iterator& >( s );
                s_ = ss.s_;
                ss.s_ = NULL;
            }
            return *this;
        }

    private:
        friend class RBTreeIndexStorage<Key,Idx>;

        Iterator( RBTreeIndexStorage<Key,Idx>* s ) :
        s_(s) { reset(); }

    private:
        RBTreeIndexStorage<Key,Idx>* s_;
    };
    typedef Iterator iterator_type;
    friend struct Iterator;

    Iterator begin() {
        return Iterator( const_cast< RBTreeIndexStorage<Key,Idx>* >( this ) );
    }

private:
    inline IndexNode* getNode( const key_type& k ) const
    {
        if ( cache_ && cache_->key == k ) {
            return cache_;
        } else if ( negativekey_ == k ) {
            return NULL;
        } else {
            IndexNode* node = const_cast< IndexStorage& >(index_).Get( k );
            if ( node )
                cache_ = node;
            else
                negativekey_ = k;
            return node;
        }
    }

    inline void invalidateCache() const {
        cache_ = NULL;
        key_type k;
        negativekey_ = k;
    }

private:
    IndexStorage                    index_;
    std::auto_ptr< IndexAllocator > allocator_;
    mutable IndexNode*              cache_;       // for successive get/set
    mutable key_type                negativekey_; // for absent key
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_STORAGE_H */
