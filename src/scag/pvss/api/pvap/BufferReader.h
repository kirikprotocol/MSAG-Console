#ifndef _SCAG_PVSS_PVAP_BUFFERREADER_H
#define _SCAG_PVSS_PVAP_BUFFERREADER_H

#include "scag/pvss/api/packets/Protocol.h"
#include "Exceptions.h"
#include "scag/util/HexDump.h"
#include <arpa/inet.h>  // htons

namespace scag2 {
namespace pvss {
namespace pvap {

class BufferReader
{
public:
    BufferReader() : buf_(0,0), pos_(0) {}
    BufferReader( const Protocol::Buffer& buf ) : buf_(const_cast<char*>(buf.get()), buf.GetPos() ), pos_(0) {
        buf_.SetPos(buf.GetPos());
    }
    BufferReader( const char* extbuf, int sz ) : buf_(const_cast<char*>(extbuf),sz), pos_(0) {
        buf_.SetPos(sz);
    }

    int readTag() throw (exceptions::IOException) {
        if ( pos_ == size() ) { return -1; }
        int rv = int(readShort()) & 0xffff;
        if ( rv == 0xffff ) return -1;
        return rv;
    }
    bool readBoolLV() throw (exceptions::IOException) {
        uint8_t b = readByteLV();
        return ( b != 0 );
    }
    uint8_t readByteLV() throw (exceptions::IOException) {
        int sz = readShort();
        if ( sz != 1 ) throw InvalidValueLength("byte", sz);
        return readByte();
    }
    uint16_t readShortLV() throw (exceptions::IOException) {
        int sz = readShort();
        if ( sz != 2 ) throw InvalidValueLength("short", sz);
        return readShort();
    }
    uint32_t readIntLV() throw (exceptions::IOException) {
        int sz = readShort();
        if ( sz != 4 ) throw InvalidValueLength("int", sz);
        return readInt();
    }
    uint64_t readLongLV() throw (exceptions::IOException) {
        int sz = readShort();
        if ( sz != 8 ) throw InvalidValueLength("long", sz);
        uint32_t i1 = readInt();
        uint32_t i0 = readInt();
        return (uint64_t(i1) << 32) | i0;
    }
    /*
    std::string readUCSLV() throw (exceptions::IOException) {
        // FIXME
    }
     */
    std::string readUTFLV() throw (exceptions::IOException) {
        int sz = readLength();
        int oldpos = pos_;
        pos_ += sz;
        return std::string( buf_.get() + oldpos, sz );
    }
    std::string readAsciiLV() throw (exceptions::IOException) {
        std::string val = readUTFLV();
        size_t sz = val.size();
        const unsigned char* p = reinterpret_cast<const unsigned char*>(val.c_str());
        for ( ; sz > 0; --sz ) {
            if ( *p > 126 || *p < 32 ) {
                // disallowed char
                throw exceptions::IOException( "disallowed char %u at pos=%d in '%s'",
                                   unsigned(*p) & 0xff, val.size() - sz, val.c_str() );
            }
        }
        return val;
    }
    
    void read( BufferReader& other ) throw(exceptions::IOException) {
        int sz = readLength();
        // rcheck(sz);
        other.buf_.setExtBuf( buf_.get() + pos_, sz );
        other.buf_.SetPos(sz);
        other.pos_ = 0;
        pos_ += sz;
    }

    int readLength() throw (exceptions::IOException) {
        int rv = readShort();
        rcheck(rv);
        return rv;
    }

    uint8_t readByte() throw (exceptions::IOException) {
        rcheck(1);
        uint8_t rv = *reinterpret_cast<const uint8_t*>(buf_.get() + pos_);
        ++pos_;
        return rv;
    }
    uint16_t readShort() throw (exceptions::IOException) {
        rcheck(2);
        uint16_t rv;
        memcpy(&rv, buf_.get()+pos_, 2);
        pos_ += 2;
        return ntohs(rv);
    }
    uint32_t readInt() throw (exceptions::IOException) {
        rcheck(4);
        uint32_t rv;
        memcpy(&rv, buf_.get()+pos_, 4);
        pos_ += 4;
        return ntohl(rv);
    }

    std::string dump() const {
        util::HexDump hd;
        std::string s;
        hd.hexdump( s, buf_.get(), buf_.GetPos() );
        return s;
    }

    size_t getPos() const { return pos_; }

protected:
    inline void rcheck( int sz ) const throw (exceptions::IOException) {
        if ( pos_+sz > size() ) throw ReadBeyondEof();
    }

    inline size_t size() const { return buf_.GetPos(); }

private:
    Protocol::Buffer buf_;
    size_t pos_;
};

} // namespace pvap
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_PVAP_BUFFERWRITER_H */
