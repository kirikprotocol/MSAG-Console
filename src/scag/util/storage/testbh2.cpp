// #include "BlocksHSStorage2.h"
#include "scag/util/HexDump.h"
#include "logger/Logger.h"
#include "HSPacker.h"
#include "Serializer.h"

using namespace scag2::util::storage;
using namespace scag2::util;
using namespace smsc::logger;

struct DummyKey
{
public:
    DummyKey( uint64_t aKey ) : key(aKey) {}
    std::string toString() const {
        char buf[30];
        snprintf(buf,sizeof(buf),"%llu",key);
        return buf;
    }
    uint64_t key;
};

Deserializer& operator >> ( Deserializer& dsr, DummyKey& key ) {
    return dsr >> key.key;
}
Serializer& operator << ( Serializer& ser, const DummyKey& key ) {
    return ser << key.key;
}


struct DummyData
{
public:
    DummyData( const std::string& from ) : value(from) {}
    const std::string& toString() const { return value; }
    std::string value;
};

Deserializer& operator >> ( Deserializer& dsr, DummyData& data ) {
    return dsr >> data.value;
}
Serializer& operator << ( Serializer& ser, const DummyData& data ) {
    return ser << data.value;
}




// typedef BlocksHSStorage2 storage_type;
typedef HSPacker::buffer_type buffer_type;

int main()
{
    const size_t blockSize = 50;

    Logger::Init();
    HSPacker bhs(blockSize,0);

    DummyKey key(79137654079ULL);

    for ( size_t i = 0; i < blockSize; ++i ) {

        std::string sdata("hello, world, bye life");
        for ( size_t j = 0; j < i; ++j ) {
            sdata.push_back( char('A'+j) );
        }
        DummyData data(sdata);

        buffer_type buf;
        const size_t initialPos = 0;
        Serializer ser(buf);
        ser.setwpos( initialPos + bhs.navSize() );
        ser << key << data;

        buffer_type headers;
        {
            // headers preparation
            Serializer hser(headers);
            const size_t trailBlocks = bhs.trailingBlocks(buf.size()-initialPos);
            for ( size_t i = 1; i <= trailBlocks; ++i ) {
                BlockNavigation bn;
                if ( i == trailBlocks ) {
                    // last block
                    bn.setNextBlock( bhs.idx2pos(bhs.invalidIndex()) );
                } else {
                    bn.setNextBlock( bhs.idx2pos(i+1) );
                }
                bn.setRefBlock(0x7777);
                bn.save(hser);
            }
        }

        buffer_type copybuf(buf);
        HexDump hd;
        {
            std::string hex,hhex;
            hd.hexdump(hex,&buf[0],buf.size());
            hd.strdump(hex,&buf[0],buf.size());
            hd.hexdump(hhex,&headers[0],headers.size());
            hd.strdump(hhex,&headers[0],headers.size());
            fprintf(stderr,"- buffer before packing: %s\n- headers before packing: %s\n\n", hex.c_str(), hhex.c_str());
        }
        bhs.packBuffer(buf,&headers,initialPos);
        {
            std::string hex;
            hd.hexdump(hex,&buf[0],buf.size());
            hd.strdump(hex,&buf[0],buf.size());
            fprintf(stderr,"- buffer after packing: %s\n\n", hex.c_str());
        }
        buffer_type newHeaders;
        bhs.unpackBuffer(buf,&newHeaders,initialPos);
        {
            std::string hex,hhex;
            hd.hexdump(hex,&buf[0],buf.size());
            hd.strdump(hex,&buf[0],buf.size());
            hd.hexdump(hhex,&newHeaders[0],newHeaders.size());
            hd.strdump(hhex,&newHeaders[0],newHeaders.size());
            fprintf(stderr,"- buffer after unpack: %s\n- headers after unpack: %s\n\n- buf_equals=%d head_equals=%d\n",
                    hex.c_str(), hhex.c_str(),
                    buf == copybuf ? 1 : 0,
                    headers == newHeaders ? 1 : 0 );
        }

        if ( buf != copybuf || headers != newHeaders ) {
            fprintf(stderr,"-- regression detected\n");
            ::abort();
        }
    }
    return 0;
}
