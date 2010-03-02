#include <cassert>
#include <string.h>    // for strlen
#include "Serializer.h"
#include "util/crc32.h"

namespace scag {
namespace util {
namespace io {

uint32_t SerializerBase::dochecksum( const unsigned char* buf, size_t pos1, size_t pos2 ) const
{
    assert(pos1 <= pos2); // && (pos2 <= buf.size()) );
    return smsc::util::crc32( 0, &(buf[pos1]), pos2 - pos1 );
}


Serializer& Serializer::operator << ( uint8_t i ) {
    /*
    if (buf_) {
        if ( wpos() >= size() ) buf_->resize(wpos()+1);
        (*buf_)[wpos_++] = i;
    } else {
        if ( wpos() >= bufSize_ ) {
            throw SerializerException("output beyoud buf");
        }
        (*ebuf_)[wpos_++] = i;
    }
     */
    *ensure(1) = i;
    return *this;
}

Serializer& Serializer::operator << ( uint16_t i ) {
    EndianConverter::set16(ensure(2),i);
    /*
    uint8_t* p = cvt.uset(i);
    if ( wpos()+1 >= size() ) buf_->resize(wpos()+2);
    (*buf_)[wpos_++] = *p++;
    (*buf_)[wpos_++] = *p;
     */
    return *this;
}

Serializer& Serializer::operator << ( uint32_t i ) {
    EndianConverter::set32(ensure(4),i);
    /*
    uint8_t* p = cvt.uset(i);
    std::copy( p, p+4, ensure(4) );
     */
    return *this;
}

Serializer& Serializer::operator << ( uint64_t i ) {
    EndianConverter::set64(ensure(8),i);
    /*
    uint8_t* p = cvt.uset(i);
    std::copy( p, p+8, ensure(8) );
     */
    return *this;
}

Serializer& Serializer::operator << ( const char* str ) {
    write( str ? uint32_t(strlen(str)) : 0, str );
    return *this;
}

Serializer& Serializer::operator << ( const std::string& i ) {
    write( uint32_t(i.size()), i.c_str() );
    return *this;
}

Serializer& Serializer::operator << ( const Buf& i ) {
    write( uint32_t(i.size()), reinterpret_cast<const char*>(&i[0]) );
    return *this;
}

void Serializer::reset() {
    if (buf_) {buf_->clear();}
    wpos_ = 0;
}

size_t Serializer::size() const {
    return ( buf_ ? buf_->size() : bufSize_ );
}

void Serializer::reserve( size_t sz ) {
    if (buf_) {buf_->reserve(sz);}
    else if (sz > bufSize_) {
        throw SerializerException("reserve(%u) on memory chunk of sz=%u",
                                  unsigned(sz), unsigned(bufSize_));
    }
}

void Serializer::setwpos( size_t wp )
{
    if ( wp > size() ) {
        if (buf_) { buf_->resize(wp); }
        else {
            throw SerializerException("setwpos(%u) on memchunk of sz=%u",
                                      unsigned(wp), unsigned(bufSize_) );
        }
    }
    wpos_ = wp;
}

void Serializer::write( uint32_t sz, const char* buf )
{
    uint8_t* p = ensure(sz+4);
    EndianConverter::set32(p,sz);
    if (buf) std::copy(buf,buf+sz,p+4);
}


void Serializer::writeAsIs( uint32_t sz, const char* buf )
{
    std::copy(buf,buf+sz,ensure(sz));
}


uint8_t* Serializer::ensure( uint32_t chunk )
{
    if ( buf_ ) {
        uint32_t resv = uint32_t(buf_->capacity() - wpos());
        if ( resv < chunk ) {
            const uint32_t need = uint32_t(buf_->capacity() + chunk);
            buf_->reserve( need < 16 ? need+16 :
                           need+128 );
        }
        if ( wpos()+chunk > buf_->size() ) {
            buf_->resize(wpos()+chunk);
        }
        uint32_t w = uint32_t(wpos());
        wpos_ += chunk;
        return &((*buf_)[w]);
    } else if ( wpos() + chunk > bufSize_ ) {
        throw SerializerException("ensure(%u+%u) on buffer sz=%u",
                                  unsigned(wpos_), unsigned(chunk), unsigned(bufSize_) );
    } else {
        uint32_t w = uint32_t(wpos());
        wpos_ += chunk;
        return ebuf_ + w;
    }
}


// =================================


Deserializer& Deserializer::operator >> ( uint8_t& i ) throw (DeserializerException )
{
    i = *readAsIs(1);
    return *this;
}


Deserializer& Deserializer::operator >> ( uint16_t& i ) throw ( DeserializerException )
{
    i = EndianConverter::get16(readAsIs(2));
    return *this;
}

    
Deserializer& Deserializer::operator >> ( uint32_t& i ) throw ( DeserializerException )
{
    i = EndianConverter::get32(readAsIs(4));
    return *this;
}

    
Deserializer& Deserializer::operator >> ( uint64_t& i ) throw ( DeserializerException )
{
    i = EndianConverter::get64(readAsIs(8));
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

const char* Deserializer::readAsIs( uint32_t sz ) throw (DeserializerException)
{
    rcheck(sz);
    const char* ret = curposc();
    rpos_ += sz;
    return ret;
}


/*
void Deserializer::readbuf( unsigned char* ptr, size_t sz ) throw ( DeserializerException )
{
    rcheck( sz );
    std::copy( curpos(), curpos() + sz, ptr );
    rpos_ += sz;
}
 */

} // namespace io
} // namespace util
} // namespace scag
