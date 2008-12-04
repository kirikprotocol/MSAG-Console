#include <cassert>
#include <string.h>    // for strlen
#include "Serializer.h"
#include "util/crc32.h"

namespace scag {
namespace util {
namespace storage {

    Serializer& Serializer::operator << ( uint8_t i ) {
        if ( wpos() >= size() ) buf_->resize(wpos()+1);
        buf_[wpos_++] = i;
        return *this;
    }

    Serializer& Serializer::operator << ( uint16_t i ) {
        uint8_t* p = cvt.uset(i);
        if ( wpos()+1 >= size() ) buf_->resize(wpos()+2);
        buf_[wpos_++] = *p++;
        buf_[wpos_++] = *p;
        return *this;
    }

    Serializer& Serializer::operator << ( uint32_t i ) {
        uint8_t* p = cvt.uset(i);
        std::copy( p, p+4, ensure(4) );
        return *this;
    }

    Serializer& Serializer::operator << ( uint64_t i ) {
        uint8_t* p = cvt.uset(i);
        std::copy( p, p+8, ensure(8) );
        return *this;
    }

    Serializer& Serializer::operator << ( const char* str ) {
        const uint32_t sz(str ? strlen(str) : 0);
        uint8_t* p = cvt.uset(sz);
        std::copy( str, str+sz, std::copy( p, p+4, ensure(sz+4)) );
        return *this;
    }

    Serializer& Serializer::operator << ( const std::string& i ) {
        const uint32_t sz( i.size() );
        uint8_t* p = cvt.uset(sz);
        std::copy( i.c_str(), i.c_str() + sz,
                   std::copy( p, p+4, ensure(sz+4) ));
        return *this;
    }

    Serializer& Serializer::operator << ( const Buf& i ) {
        const uint32_t sz( i.size() );
        uint8_t* p = cvt.uset(sz);
        std::copy( &(i[0]), &(i[0]) + sz,
                   std::copy( p, p+4, ensure(sz+4) ));
        return *this;
    }


    void Serializer::write( uint32_t sz, const char* buf )
    {
        uint8_t* p = cvt.uset(sz);
        std::copy( buf, buf+sz,
                   std::copy( p, p+4, ensure(sz+4) ));
    }


void Serializer::writeAsIs( uint32_t sz, const char* buf )
{
    std::copy( buf, buf+sz, ensure(sz) );
}


    Deserializer& Deserializer::operator >> ( uint8_t& i ) throw (DeserializerException )
    {
        readbuf( & static_cast<unsigned char&>(i), 1 ); 
        return *this;
    }


    Deserializer& Deserializer::operator >> ( uint16_t& i ) throw ( DeserializerException )
    {
        readbuf( cvt.ubuf(), 2 );
        i = cvt.get16();
        return *this;
    }

    
    Deserializer& Deserializer::operator >> ( uint32_t& i ) throw ( DeserializerException )
    {
        readbuf( cvt.ubuf(), 4 );
        i = cvt.get32();
        return *this;
    }

    
    Deserializer& Deserializer::operator >> ( uint64_t& i ) throw ( DeserializerException )
    {
        readbuf( cvt.ubuf(), 8 );
        i = cvt.get64();
        return *this;
    }


    Deserializer& Deserializer::operator >> ( const char* str ) throw (DeserializerException) {
        uint32_t sz;
        *this >> sz;
        rcheck(sz);
        if ( !str ) {
            if (sz) throw DeserializerException::stringMismatch();
            return *this;
        }
        if ( strncmp( str, curposc(), sz ) )
            throw DeserializerException::stringMismatch();
        return *this;
    }


    Deserializer& Deserializer::operator >> ( std::string& i ) throw ( DeserializerException )
    {
        uint32_t sz;
        *this >> sz;
        rcheck( sz );
        i.assign( curposc(), sz );
        rpos_ += sz;
        return *this;
    }


    Deserializer& Deserializer::operator >> ( Buf& i ) throw ( DeserializerException )
    {
        uint32_t sz;
        *this >> sz;
        rcheck( sz );
        i.assign( curpos(), curpos() + sz );
        rpos_ += sz;
        return *this;
    }


    const char* Deserializer::read( uint32_t& size ) throw (DeserializerException)
    {
        uint32_t sz;
        *this >> sz;
        rcheck( sz );
        size = sz;
        const char* ret = curposc();
        rpos_ += sz;
        return ret;
    }

const char* Deserializer::readAsIs( uint32_t size ) throw (DeserializerException)
{
    rcheck(sz);
    const char* ret = curposc();
    rpos += sz;
    return ret;
}


    uint32_t SerializerBase::dochecksum( const unsigned char* buf, size_t pos1, size_t pos2 ) const
    {
        assert(pos1 <= pos2); // && (pos2 <= buf.size()) );
        return smsc::util::crc32( 0, &(buf[pos1]), pos2 - pos1 );
    }


    Serializer::Buf::iterator Serializer::ensure( uint32_t chunk )
    {
        uint32_t resv = buf_->capacity() - wpos();
        if ( resv < chunk ) {
            const uint32_t need = buf_->capacity() + chunk;
            buf_->reserve( need < 16 ? need+16 :
                           need+128 );
        }
        if ( wpos()+chunk > size() ) {
            buf_->resize(wpos()+chunk);
        }
        return buf_->begin() + wpos();
    }


    void Deserializer::readbuf( unsigned char* ptr, size_t sz ) throw ( DeserializerException )
    {
        rcheck( sz );
        std::copy( curpos(), curpos() + sz, ptr );
        rpos_ += sz;
    }

} // namespace storage
} // namespace util
} // namespace scag
