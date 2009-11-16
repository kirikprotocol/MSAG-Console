#ifndef _SCAG_PVSS_PVAP_BUFFERWRITER_H
#define _SCAG_PVSS_PVAP_BUFFERWRITER_H

#include "scag/pvss/api/packets/Protocol.h"
#include "Exceptions.h"
#include "scag/util/io/HexDump.h"
#include <arpa/inet.h>  // htons

namespace scag2 {
namespace pvss {
namespace pvap {

class BufferWriter
{
public:
    BufferWriter( Protocol::Buffer& buffer ) : buf_(&buffer) {}

    void writeTag( int tag ) throw( PvapException ) {
        writeShort( uint16_t(tag) );
    }
    void writeBoolLV( bool val ) {
        writeByteLV( uint8_t(val?1:0) );
    }
    void writeByteLV( uint8_t val ) {
        writeLength(1);
        writeByte(val);
    }
    void writeShortLV( int16_t val ) {
        writeLength(2);
        writeShort(val);
    }
    void writeIntLV( int32_t val ) {
        writeLength(4);
        writeInt(val);
    }
    void writeLongLV( int64_t val ) {
        writeLength(8);
        writeInt(int32_t(val >> 32));
        writeInt(int32_t(val));
    }
    /*
    void writeUCSLV( const std::string& val ) {
        int sz = val.size();
        writeLength( sz*2 );
        const unsigned char* p = reinterpret_cast<const unsigned char*>(val.c_str());
        for ( ; sz > 0; --sz ) {
            writeShort( btowc(*p++) );
        }
    }
     */
    void writeUTFLV( const std::string& val ) throw (exceptions::IOException) {
        size_t sz = val.size();
        if ( sz > 0xffff ) throw exceptions::IOException( "too long string in writeUTFLV: %llu", static_cast<unsigned long long>(sz) );
        writeLength(int(sz));
        buf_->Append( val.c_str(), sz );
    }
    void writeAsciiLV( const std::string& val ) throw (exceptions::IOException) {
        size_t sz = val.size();
        const unsigned char* p = reinterpret_cast<const unsigned char*>(val.c_str());
        for ( ; sz > 0; --sz ) {
            if ( *p > 126 || *p < 32 ) {
                // disallowed char
                throw exceptions::IOException( "disallowed char %u at pos=%d in '%s'",
                                               unsigned(*p) & 0xff, val.size() - sz, val.c_str() );
            }
        }
        writeUTFLV( val );
    }
    
    void write( const BufferWriter& other ) throw (exceptions::IOException) {
        if ( other.buf_->GetPos() > 0xffffU ) 
            throw exceptions::IOException("too large buffer is written: size=%lld", static_cast<unsigned long long>(other.buf_->GetPos()));
        // buf_->reserve( buf_->GetPos() + other.buf_->GetPos() + 2 );
        writeLength( int(other.buf_->GetPos()) );
        buf_->Append( other.buf_->get(), other.buf_->GetPos() );
    }

    void writeLength( int sz ) {
        buf_->reserve( buf_->GetPos() + 2 + sz );
        writeShort(short(sz));
    }
    void writeByte( uint8_t val ) {
        buf_->Append( reinterpret_cast<const char*>(&val), 1 );
    }
    void writeShort( int16_t val ) {
        uint16_t netval = htons(val);
        buf_->Append( reinterpret_cast<const char*>(&netval), 2 );
    }
    void writeInt( int32_t val ) {
        uint32_t netval = htonl(val);
        buf_->Append( reinterpret_cast<const char*>(&netval), 4 );
    }

    std::string dump() const {
        util::HexDump hd;
        util::HexDump::string_type s;
        hd.hexdump( s, buf_->get(), buf_->GetPos() );
        return hd.c_str(s);
    }

private:
    Protocol::Buffer* buf_;
};

} // namespace pvap
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_PVAP_BUFFERWRITER_H */
