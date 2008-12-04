#ifndef _SCAG_UTIL_STORAGE_SERIALIZER_H
#define _SCAG_UTIL_STORAGE_SERIALIZER_H

#include <stdexcept>
#include <string>
#include <vector>
#include <netinet/in.h>

#include "GlossaryBase.h"
#include "EndianConverter.h"

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
    inline static DeserializerException dataTooBig() {
        return DeserializerException("Deserializer: data length is too big");
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

    EndianConverter cvt;

private:
    GlossaryBase* glossary_;
    int32_t       version_;
};


class Serializer : public SerializerBase
{
public:
    Serializer( Buf& b, GlossaryBase* glossary = NULL ) : SerializerBase(glossary) ,buf_(&b), wpos_(0) {}

    // writing at wpos
    Serializer& operator << ( uint8_t );
    Serializer& operator << ( uint16_t );
    Serializer& operator << ( uint32_t );
    Serializer& operator << ( uint64_t );
    Serializer& operator << ( const char* );
    Serializer& operator << ( const std::string& );
    Serializer& operator << ( const Buf& );

    inline size_t size() const {
        return buf_->size();
    }

    inline void reserve( size_t sz ) {
        buf_->reserve(sz);
    }

    inline size_t wpos() const {
        return wpos_;
    }

    inline void setWpos( size_t wp ) {
        wpos_ = wp;
        if ( wpos_ > size() ) buf_->resize(wpos_); // do we need this?
    }

    inline const unsigned char* data() const {
        return buf_->size() ? &(buf_->front()) : 0;
    }

    /// write buffer of size sz.
    /// This method is provided to co-work with other serializer types.
    /// NOTE: see also Deserializer::read().
    /// NOTE: this method adds a length to buffer.
    void write( uint32_t sz, const char* buf );

    /// NOTE: this method does not add a length to the buffer.
    /// NOTE: see also Deserializer::readAsIs().
    void writeAsIs( uint32_t sz, const char* buf );

    uint32_t checksum( size_t pos1, size_t pos2 ) const {
        assert( pos2 < buf_->size() );
        return dochecksum( &(buf_->front()), pos1, pos2 );
    }


private:
    // returns an iterator to a chunk of given size
    Buf::iterator ensure( uint32_t chunk );
    Serializer();

private:
    Buf*   buf_;
    size_t wpos_;  // current writing position
};


class Deserializer : public SerializerBase
{
public:
    // a special constructor from a chunk of data in memory to avoid an extra copying
    Deserializer( const unsigned char* buf, size_t bufsize, GlossaryBase* glossary = NULL ) :
    SerializerBase(glossary), buf_(0), rpos_(0), ebuf_(buf), esize_(bufsize) {}

    // a ctor from standard buffer
    Deserializer( const Buf& buf, GlossaryBase* glossary = NULL ) :
    SerializerBase(glossary), buf_(&buf), rpos_(0), ebuf_(0), esize_(buf_->size()) {}

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

    const char* readAsIs( uint32_t sz ) throw (DeserializerException);

    inline size_t size() const {
        return esize_;
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

private:
    /// read from buffer into \ptr.
    void readbuf( unsigned char* ptr, size_t size ) throw ( DeserializerException );
    
    /// 1. checks that buffer has enough data to read;
    /// 2. if not raise an exception.
    inline void rcheck( size_t sz ) const throw ( DeserializerException ) {
        if ( rpos_ + sz > size() )
            throw DeserializerException::bufferUnderrun();
    }

private:
    Deserializer();

    inline const unsigned char* curpos() const {
        return buf_ ? &((*buf_)[rpos_]) : &(ebuf_[rpos_]);
    }
    inline const char* curposc() const {
        return reinterpret_cast< const char* >(curpos());
    }

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
namespace storage = scag::util::storage;
}
}

#endif /* _SCAG_UTIL_STORAGE_SERIALIZER_H */
