#ifndef _SCAG_UTIL_STORAGE_PAGEFILEDISKSTORAGE_H
#define _SCAG_UTIL_STORAGE_PAGEFILEDISKSTORAGE_H

#include <string>
#include <vector>
#include <cassert>
#include "logger/Logger.h"
#include "core/buffers/PageFile.hpp"
#include "Serializer.h"

namespace scag {
namespace util {
namespace storage {

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
    mutable std::vector< unsigned char > buf;
};


/// implementation of storage ifaces
template < class Key, class Val, class PF = smsc::core::buffers::PageFile >
class PageFileDiskStorage : public MutableBufferHolder
{
public:
    static const bool                                 updatable = false;
    typedef PF                                        storage_type;
    typedef smsc::core::buffers::File::offset_type    index_type;
    typedef Key                                       key_type;
    typedef Val                                       value_type;

    PageFileDiskStorage( PF* pf, GlossaryBase* g = NULL ) : pf_(pf), disklog_(NULL), glossary_(g)
    {
        if ( ! pf_ )
            throw std::runtime_error( "PageFileDiskStorage: pagefile should be provided!" );
        // if ( ! glossary_ )
        // throw std::runtime_error( "PageFileDiskStorage: glossary should be provided!" );
        disklog_ = smsc::logger::Logger::getInstance( "disk" );
    }

    ~PageFileDiskStorage() {
        delete pf_;
    }


    bool setKey( const key_type& ) {
        return true;
    }


    /// serialize the value into an internal buffer
    void serialize( const value_type& v ) {
        buf.resize(0);
        Serializer s( buf, glossary_ );
        s << v;
        // assert( (buf.size() < 10000) && (buf.buffer().capacity() < 10000) );
        // key_ = v.getKey().toString();
    }
        
    /// append data from internal buffer to the storage
    index_type append( ) {
        const index_type i = pf_->Append( &(buf[0]), buf.size() );
        // smsc_log_debug( disklog_, "append: index=%llx val=%s",
        // static_cast<unsigned long long>(i), key_.c_str() );
        // key_ = "destroyed";
        return i;
    }

    /*
    // update data from internal buffer to the storage
    index_type update( index_type i ) {
        // NOTE: this method should not be invoked
        ::abort();
        // smsc_log_debug( disklog_, "update: index=%llx val=%s",
        // static_cast<unsigned long long>(i), key_.c_str() );
        // pf_->Delete( i );
        // i = pf_->Append( buf.data(), buf.size() );
        // pf_->Update( i, buf.data(), buf.size() );
        // key_ = "destroyed";
        return i;
    }
     */

    /// read data from storage into internal (mutable) buffer
    bool read( index_type i ) const {
        buf.resize(0);
        index_type j;
        pf_->Read( i, buf, &j );
        if ( i != j ) {
            smsc_log_warn( disklog_, "read: different index read: was=%llx is=%llx",
                           static_cast< unsigned long long >( i ),
                           static_cast< unsigned long long >( j ) );
        }
        // assert( buf.size() < 10000 && buf.buffer().capacity() < 10000 );
        return true;
    }

    /// deserialize value from internal buffer
    /// @return true if successfully deserialized, otherwise v is broken
    bool deserialize( value_type& v ) const {
        try {
            Deserializer s( buf, glossary_ );
            s >> v;
            // key_ = "destroyed";
        } catch ( DeserializerException& e ) {
            smsc_log_error( disklog_, "exception occurred: %s", e.what() );
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
    GlossaryBase* glossary_;
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_PAGEFILEDISKSTORAGE_H */
