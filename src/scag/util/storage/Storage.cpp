#include <cassert>
#include "Storage.h"
#include "util/crc32.h"

namespace scag {
namespace util {
namespace storage {

    Serializer& Serializer::operator << ( uint8_t i ) {
        buf_.push_back( i );
        return *this;
    }

    Serializer& Serializer::operator << ( uint16_t i ) {
        cvt.words[0] = htons( i );
        buf_.push_back( cvt.bytes[0] );
        buf_.push_back( cvt.bytes[1] );
        return *this;
    }

    Serializer& Serializer::operator << ( uint32_t i ) {
        cvt.longs[0] = htonl( i );
        std::copy( cvt.bytes, cvt.bytes+4, ensure(4) );
        return *this;
    }

    Serializer& Serializer::operator << ( uint64_t i ) {
        cvt.longs[0] = htonl( uint32_t(i >> 32) );
        cvt.longs[1] = htonl( uint32_t(i) );
        std::copy( cvt.bytes, cvt.bytes+8, ensure(8) );
        return *this;
    }

    Serializer& Serializer::operator << ( const std::string& i ) {
        const uint32_t sz( i.size() );
        cvt.longs[0] = htonl(sz);
        std::copy( i.c_str(), i.c_str() + sz,
                   std::copy( cvt.bytes, cvt.bytes+4, ensure(sz+4) ));
        return *this;
    }

    Serializer& Serializer::operator << ( const Buf& i ) {
        const uint32_t sz( i.size() );
        cvt.longs[0] = htonl(sz);
        std::copy( &(i[0]), &(i[0]) + sz,
                   std::copy( cvt.bytes, cvt.bytes+4, ensure(sz+4) ));
        return *this;
    }


    Deserializer& Deserializer::operator >> ( uint8_t& i ) throw ( BufferUnderrunException )
    {
        readbuf( & static_cast<unsigned char&>(i), 1 ); 
        return *this;
    }


    Deserializer& Deserializer::operator >> ( uint16_t& i ) throw ( BufferUnderrunException )
    {
        readbuf( cvt.bytes, 2 );
        i = ntohs( cvt.words[0] );
        return *this;
    }

    
    Deserializer& Deserializer::operator >> ( uint32_t& i ) throw ( BufferUnderrunException )
    {
        readbuf( cvt.bytes, 4 );
        i = ntohl( cvt.longs[0] );
        return *this;
    }

    
    Deserializer& Deserializer::operator >> ( uint64_t& i ) throw ( BufferUnderrunException )
    {
        readbuf( cvt.bytes, 8 );
        i = (uint64_t(ntohl(cvt.longs[0])) << 32) + ntohl(cvt.longs[1]);
        return *this;
    }


    Deserializer& Deserializer::operator >> ( std::string& i ) throw ( BufferUnderrunException )
    {
        uint32_t sz;
        *this >> sz;
        rcheck( sz );
        i.assign( reinterpret_cast< const char* >( &(buf_[rpos_]) ), sz );
        rpos_ += sz;
        return *this;
    }


    Deserializer& Deserializer::operator >> ( Buf& i ) throw ( BufferUnderrunException )
    {
        uint32_t sz;
        *this >> sz;
        rcheck( sz );
        i.assign( &(buf_[rpos_]), &(buf_[rpos_]) + sz );
        rpos_ += sz;
        return *this;
    }


    uint32_t SerializerBase::dochecksum( const Buf& buf, size_t pos1, size_t pos2 ) const
    {
        assert( (pos1 <= pos2) && (pos2 <= buf.size()) );
        return smsc::util::crc32( 0, &(buf[pos1]), pos2 - pos1 );
    }


    Serializer::Buf::iterator Serializer::ensure( uint32_t chunk )
    {
        uint32_t resv = buf_.capacity() - buf_.size();
        if ( resv < chunk ) {
            buf_.reserve( buf_.capacity() + chunk + 1024 );
        }
        Serializer::Buf::iterator res = buf_.end();
        buf_.resize( buf_.size() + chunk );
        return res;
    }


    void Deserializer::readbuf( unsigned char* ptr, size_t sz ) throw ( BufferUnderrunException )
    {
        rcheck( sz );
        std::copy( &(buf_[rpos_]), &(buf_[rpos_]) + sz, ptr );
        rpos_ += sz;
    }

} // namespace storage
} // namespace util
} // namespace scag
