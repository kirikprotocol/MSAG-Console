#ifndef _SCAG_UTIL_STORAGE_SERIALIZER_H
#define _SCAG_UTIL_STORAGE_SERIALIZER_H

#include <stdexcept>
#include <string>
#include <vector>
#include <netinet/in.h>

namespace scag {
namespace util {
namespace storage {

class DeserializerException : public std::exception {
private:
    DeserializerException();
    // make sure w lives enough
    DeserializerException( const char* w ) : what_(w) {}

public:
    inline static DeserializerException bufferUnderrun() {
        return DeserializerException("Deserializer: buffer underrun");
    }
    inline static DeserializerException stringMismatch() {
        return DeserializerException("Deserializer: c-string mismatch");
    }

    virtual ~DeserializerException() throw () {}
    virtual const char* what() const throw () { return what_; }

private:
    const char* what_;
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
    Serializer& operator << ( const char* );
    Serializer& operator << ( const std::string& );
    Serializer& operator << ( const Buf& );

    size_t size() const {
        return buf_.size();
    }

    /// write buffer of size sz.
    /// This method is provided to co-work with other serializer types.
    /// NOTE: see also Deserializer::read().
    void write( uint32_t sz, const char* buf );
    
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

    Deserializer& operator >> ( uint8_t& ) throw ( DeserializerException );
    Deserializer& operator >> ( uint16_t& ) throw ( DeserializerException );
    Deserializer& operator >> ( uint32_t& ) throw ( DeserializerException );
    Deserializer& operator >> ( uint64_t& ) throw ( DeserializerException );
    Deserializer& operator >> ( const char* ) throw ( DeserializerException );
    Deserializer& operator >> ( std::string& ) throw ( DeserializerException );
    Deserializer& operator >> ( Buf& ) throw ( DeserializerException );

    /// read buffer previously written via Serializer::write().
    /// you have to treat the return value as a buffer of length sz!
    const char* read( uint32_t& sz ) throw (DeserializerException);

    inline size_t size() const {
        return buf_.size();
    }
    
    inline size_t rpos() const {
        return rpos_;
    }

    inline void setrpos( size_t r ) throw ( DeserializerException ) {
        if ( r > size() ) throw DeserializerException::bufferUnderrun();
        rpos_ = r;
    }

    uint32_t checksum( size_t pos1, size_t pos2 ) const {
        return dochecksum( buf_, pos1, pos2 );
    }

private:
    /// read from buffer into \ptr.
    void readbuf( unsigned char* ptr, size_t size ) throw ( DeserializerException );
    
    /// 1. checks that buffer has enough data to read;
    /// 2. if not raise an exception.
    inline void rcheck( size_t sz ) const throw ( DeserializerException ) {
        if ( rpos_ + sz > buf_.size() )
            throw DeserializerException::bufferUnderrun();
    }

private:
    const Buf&   buf_;  // NOTE the reference
    size_t rpos_;
};
    
    
} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_SERIALIZER_H */
