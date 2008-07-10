#ifndef _SCAG_UTIL_STORAGE_SERIALIZER_H
#define _SCAG_UTIL_STORAGE_SERIALIZER_H

#include <stdexcept>
#include <string>

namespace scag {
namespace util {
namespace storage {

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
    
    
} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_SERIALIZER_H */
