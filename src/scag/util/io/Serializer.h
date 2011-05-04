#ifndef _SCAG_UTIL_STORAGE_SERIALIZER_H
#define _SCAG_UTIL_STORAGE_SERIALIZER_H

#include <stdexcept>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <cassert>

#include "GlossaryBase.h"
#include "EndianConverter.h"
#include "util/Exception.hpp"

namespace scag {
namespace util {
namespace io {

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
    inline static DeserializerException dataTooBig() {
        return DeserializerException("Deserializer: data length is too big");
    }

    virtual ~DeserializerException() throw () {}
    virtual const char* what() const throw () { return what_; }

private:
    const char* what_;
};


class SerializerException : public std::exception
{
public:
    SerializerException( const char* fmt, ... ) {
        va_list args;
        va_start(args,fmt);
        fill(fmt,args);
        va_end(args);
    }
    virtual ~SerializerException() throw () {}
    virtual const char* what() const throw () { return message_.c_str(); }
private:
    inline void fill( const char* fmt, va_list& arglist ) {
        smsc::util::vformat(message_, fmt, arglist);
    }
private:
    std::string message_;
};



class SerializerBase
{
public:
    typedef std::vector< unsigned char > Buf;
    GlossaryBase* getGlossary() const { return glossary_; }
    virtual ~SerializerBase() {};
    
    /// get the version which is requested by the serialized data (if written externally).
    inline int32_t version() const {
        return version_;
    }
    inline void setVersion( int32_t v ) {
        version_ = v;
    }

protected:
    SerializerBase( GlossaryBase* glossary ) : glossary_(glossary), version_(0) {}

    uint32_t dochecksum( const unsigned char* buf, size_t pos1, size_t pos2 ) const;

private:
    GlossaryBase* glossary_;
    int32_t       version_;
};


class Serializer : public SerializerBase
{
public:
    Serializer( Buf& b, GlossaryBase* glossary = 0 ) :
    SerializerBase(glossary) ,buf_(&b), wpos_(0), ebuf_(0), bufSize_(0) {}

    // special ctor to a chunk of memory to avoid extra copying
    Serializer( void* buf, size_t bufSize, GlossaryBase* glossary = 0 ) :
    SerializerBase(glossary), buf_(0), wpos_(0),
    ebuf_(reinterpret_cast<unsigned char*>(buf)), bufSize_(bufSize) {}

    // writing at wpos
    inline Serializer& operator << ( int8_t x ) { return *this << uint8_t(x); }
    Serializer& operator << ( uint8_t );
    inline Serializer& operator << ( int16_t x ) { return *this << uint16_t(x); }
    Serializer& operator << ( uint16_t );
    inline Serializer& operator << ( int32_t x ) { return *this << uint32_t(x); }
    Serializer& operator << ( uint32_t );
    inline Serializer& operator << ( int64_t x ) { return *this << uint64_t(x); }
    Serializer& operator << ( uint64_t );
    Serializer& operator << ( const char* );
    Serializer& operator << ( const std::string& );
    Serializer& operator << ( const Buf& );

    void reset();
    size_t size() const;
    void reserve( size_t sz );

    inline size_t wpos() const {
        return wpos_;
    }

    void setwpos( size_t wp );

    inline const unsigned char* data() const {
        return buf_ ? (buf_->size() ? &(buf_->front()) : 0) :
        reinterpret_cast<const unsigned char*>(ebuf_);
    }

    inline unsigned char* data() {
        return buf_ ? (buf_->size() ? &(buf_->front()) : 0) :
        reinterpret_cast<unsigned char*>(ebuf_);
    }

    /// write buffer of size sz.
    /// This method is provided to co-work with other serializer types.
    /// NOTE: see also Deserializer::read().
    /// NOTE: this method adds a length to buffer.
    void write( uint32_t sz, const char* buf );

    /// NOTE: this method does not add a length to the buffer.
    /// NOTE: see also Deserializer::readAsIs().
    /// NOTE: this method does not add a length to buffer.
    void writeAsIs( uint32_t sz, const char* buf );

    uint32_t checksum( size_t pos1, size_t pos2 ) const {
        assert( pos2 < size() );
        return dochecksum( data(), pos1, pos2 );
    }


private:
    // returns a pointer to a given position, chunk is added to wpos_.
    uint8_t* ensure( uint32_t chunk );
    Serializer();

private:
    Buf*   buf_;
    size_t wpos_;    // current writing position
    unsigned char* ebuf_;    // not owned
    size_t         bufSize_; // size of ebuf_
};


class Deserializer : public SerializerBase
{
public:
    // a special constructor from a chunk of data in memory to avoid an extra copying
    Deserializer( const void* buf, size_t bufsize, GlossaryBase* glossary = NULL ) :
    SerializerBase(glossary), buf_(0), rpos_(0),
    ebuf_(reinterpret_cast<const unsigned char*>(buf)), esize_(bufsize) {}

    // a ctor from standard buffer
    Deserializer( const Buf& buf, GlossaryBase* glossary = NULL ) :
    SerializerBase(glossary), buf_(&buf), rpos_(0), ebuf_(0), esize_(buf_->size()) {}

    inline Deserializer& operator >> ( int8_t& x ) throw ( DeserializerException ) { return *this >> (uint8_t&)x; }
    Deserializer& operator >> ( uint8_t& ) throw ( DeserializerException );
    inline Deserializer& operator >> ( int16_t& x ) throw ( DeserializerException ) { return *this >> (uint16_t&)x; }
    Deserializer& operator >> ( uint16_t& ) throw ( DeserializerException );
    inline Deserializer& operator >> ( int32_t& x ) throw ( DeserializerException ) { return *this >> (uint32_t&)x; }
    Deserializer& operator >> ( uint32_t& ) throw ( DeserializerException );
    inline Deserializer& operator >> ( int64_t& x ) throw ( DeserializerException ) { return *this >> (uint64_t&)x; }
    Deserializer& operator >> ( uint64_t& ) throw ( DeserializerException );
    Deserializer& operator >> ( const char* ) throw ( DeserializerException );
    Deserializer& operator >> ( std::string& ) throw ( DeserializerException );
    Deserializer& operator >> ( Buf& ) throw ( DeserializerException );

    /// read buffer previously written via Serializer::write().
    /// you have to treat the return value as a buffer of length sz!
    const char* read( uint32_t& sz ) throw (DeserializerException);

    const char* readAsIs( uint32_t sz ) throw (DeserializerException);

    inline size_t size() const {
        return buf_ ? buf_->size() : esize_;
    }
    
    inline size_t rpos() const {
        return rpos_;
    }

    inline void setrpos( size_t r ) throw ( DeserializerException ) {
        if ( r > size() ) throw DeserializerException::bufferUnderrun();
        rpos_ = r;
    }

    uint32_t checksum( size_t pos1, size_t pos2 ) const {
        return dochecksum( buf_ ? &(buf_->front()) : ebuf_, pos1, pos2 );
    }

    inline const unsigned char* curpos() const {
        return buf_ ? &((*buf_)[rpos_]) : &(ebuf_[rpos_]);
    }
    inline const char* curposc() const {
        return reinterpret_cast< const char* >(curpos());
    }

private:
    // read size bytes (actually move buffer pos, checking for ability to do so)
    // unsigned char* readbuf(size_t size) throw ( DeserializerException );
    
    /// 1. checks that buffer has enough data to read;
    /// 2. if not raise an exception.
    inline void rcheck( size_t sz ) const throw ( DeserializerException ) {
        if ( rpos_ + sz > size() )
            throw DeserializerException::bufferUnderrun();
    }

private:
    Deserializer();

private:
    const Buf*           buf_;
    size_t               rpos_;
    const unsigned char* ebuf_;
    size_t               esize_;
};
    
    
} // namespace storage
} // namespace util
} // namespace scag

namespace scag2 {
namespace util {
namespace io {
using namespace scag::util::io;
}
}
}

#endif /* _SCAG_UTIL_STORAGE_SERIALIZER_H */
