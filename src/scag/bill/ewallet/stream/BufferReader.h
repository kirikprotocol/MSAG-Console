#ifndef SCAG_BILL_EWALLET_STREAM_BUFFERREADER_H
#define SCAG_BILL_EWALLET_STREAM_BUFFERREADER_H

#include "scag/exc/IOException.h"
#include "scag/util/io/HexDump.h"
#include "scag/util/io/EndianConverter.h"
#include "scag/bill/ewallet/Streamer.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace stream {

class BufferReader
{
public:
    BufferReader() : buf_(0,0), pos_(0) {}
    BufferReader( const Streamer::Buffer& buf ) : buf_(const_cast<char*>(buf.get()), buf.GetPos() ), pos_(0) {
        buf_.SetPos(buf.GetPos());
    }
    BufferReader( const char* extbuf, int sz ) : buf_(const_cast<char*>(extbuf),sz), pos_(0) {
        buf_.SetPos(sz);
    }

    int readTag() /* throw (exceptions::IOException) */ {
        if ( pos_ == size() ) { return -1; }
        int rv = int(readShort()) & 0xffff;
        if ( rv == 0xffff ) return -1;
        return rv;
    }
    bool readBoolLV() /* throw (exceptions::IOException) */ {
        uint8_t b = readByteLV();
        return ( b != 0 );
    }
    uint8_t readByteLV() /* throw (exceptions::IOException) */ {
        int sz = readShort();
        if ( sz != 1 ) throw exceptions::IOException("invalid length=%u for byte",sz);
        return readByte();
    }
    uint16_t readShortLV() /* throw (exceptions::IOException) */ {
        int sz = readShort();
        if ( sz != 2 ) throw exceptions::IOException("invalid length=%u for short",sz);
        return readShort();
    }
    uint32_t readIntLV() /* throw (exceptions::IOException) */ {
        int sz = readShort();
        if ( sz != 4 ) throw exceptions::IOException("invalid length=%u for int",sz);
        return readInt();
    }
    uint64_t readLongLV() /* throw (exceptions::IOException) */ {
        int sz = readShort();
        if ( sz != 8 ) throw exceptions::IOException("invalid length=%u for long",sz);
        uint32_t i1 = readInt();
        uint32_t i0 = readInt();
        return (uint64_t(i1) << 32) | i0;
    }
    std::string readUTFLV() /* throw (exceptions::IOException) */ {
        int sz = readLength();
        int oldpos = pos_;
        pos_ += sz;
        return std::string( buf_.get() + oldpos, sz );
    }
    std::string readAsciiLV() /* throw (exceptions::IOException) */ {
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
    
    void read( BufferReader& other ) /* throw(exceptions::IOException) */ {
        int sz = readLength();
        // rcheck(sz);
        other.buf_.setExtBuf( buf_.get() + pos_, sz );
        other.buf_.SetPos(sz);
        other.pos_ = 0;
        pos_ += sz;
    }

    int readLength() /* throw (exceptions::IOException) */ {
        int rv = readShort();
        rcheck(rv);
        return rv;
    }

    uint8_t readByte() /* throw (exceptions::IOException) */ {
        rcheck(1);
        uint8_t rv = *reinterpret_cast<const uint8_t*>(buf_.get() + pos_);
        ++pos_;
        return rv;
    }
    uint16_t readShort() /* throw (exceptions::IOException) */ {
        rcheck(2);
        const char* p = buf_.get()+pos_;
        pos_ += 2;
        return util::io::EndianConverter::get16(p);
    }
    uint32_t readInt() /* throw (exceptions::IOException) */ {
        rcheck(4);
        const char* p = buf_.get()+pos_;
        pos_ += 4;
        return util::io::EndianConverter::get32(p);
    }

    std::string dump() const {
        util::HexDump hd;
        util::HexDump::string_type s;
        hd.hexdump( s, buf_.get(), buf_.GetPos() );
        return hd.c_str(s);
    }

    size_t getPos() const { return pos_; }

    const Streamer::Buffer& getBuffer() const { return buf_; }

protected:
    inline void rcheck( int sz ) const /* throw (exceptions::IOException) */ {
        if ( pos_+sz > size() ) throw exceptions::IOException("read beyond eof: size=%llu pos=%llu rsz=%u",
                                                              uint64_t(size()),uint64_t(pos_),sz);
    }

    inline size_t size() const { return buf_.GetPos(); }

private:
    Streamer::Buffer buf_;
    size_t pos_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_BUFFERWRITER_H */
