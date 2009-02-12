#ifndef _SCAG_PVSS_PVAP_BUFFERWRITER_H
#define _SCAG_PVSS_PVAP_BUFFERWRITER_H

#include "scag/pvss/packets/Protocol.h"
#include "Exceptions.h"
#include "scag/util/HexDump.h"
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
    void writeShortLV( uint16_t val ) {
        writeLength(2);
        writeShort(val);
    }
    void writeIntLV( uint32_t val ) {
        writeLength(4);
        writeInt(val);
    }
    void writeLongLV( uint64_t val ) {
        writeLength(8);
        writeInt(uint32_t(val >> 32));
        writeInt(uint32_t(val));
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
    void writeUTFLV( const std::string& val ) {
        int sz = val.size();
        writeLength(sz);
        buf_->Append( val.c_str(), sz );
    }
    void writeAsciiLV( const std::string& val ) throw (IOException) {
        int sz = val.size();
        const unsigned char* p = reinterpret_cast<const unsigned char*>(val.c_str());
        for ( ; sz > 0; --sz ) {
            if ( *p > 126 || *p < 32 ) {
                // disallowed char
                throw IOException( "disallowed char %u at pos=%d in '%s'",
                                   unsigned(*p) & 0xff, val.size() - sz, val.c_str() );
            }
        }
        writeUTFLV( val );
    }
    
    void write( const BufferWriter& other ) {
        buf_->reserve( buf_->GetPos() + other.buf_->GetPos() + 4 );
        writeInt( other.buf_->GetPos() );
        buf_->Append( other.buf_->get(), other.buf_->GetPos() );
    }

    void writeLength( int sz ) {
        buf_->reserve( buf_->GetPos() + 2 + sz );
        writeShort(short(sz));
    }
    void writeByte( uint8_t val ) {
        buf_->Append( reinterpret_cast<const char*>(&val), 1 );
    }
    void writeShort( uint16_t val ) {
        uint16_t netval = htons(val);
        buf_->Append( reinterpret_cast<const char*>(&netval), 2 );
    }
    void writeInt( uint32_t val ) {
        uint32_t netval = htonl(val);
        buf_->Append( reinterpret_cast<const char*>(&netval), 4 );
    }

    std::string dump() const {
        util::HexDump hd;
        std::string s;
        hd.hexdump( s, buf_->get(), buf_->GetPos() );
        return s;
    }

private:
    Protocol::Buffer* buf_;
};

} // namespace pvap
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_PVAP_BUFFERWRITER_H */
