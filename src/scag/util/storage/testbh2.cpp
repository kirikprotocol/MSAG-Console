#include "BlocksHSStorage2.h"
#include "scag/util/HexDump.h"
#include "logger/Logger.h"

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




typedef BlocksHSStorage2 storage_type;

int main()
{
    Logger::Init();
    storage_type bhs( 40, 0 );
    storage_type::buffer_type buf;

    DummyKey key(79137654079ULL);
    DummyData data("hello, world, bye life, abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ");
    Serializer ser(buf);
    ser.setwpos( bhs.headerSize() );
    ser << key << data;
    storage_type::buffer_type copybuf(buf);
    HexDump hd;
    {
        std::string hex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        fprintf(stderr,"- buffer before packing: %s\n", hex.c_str());
    }
    bhs.packBuffer(buf);
    {
        std::string hex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        fprintf(stderr,"- buffer before write: %s\n", hex.c_str());
    }
    const storage_type::index_type ix = bhs.change( key.toString().c_str(), 0, 0, &buf );
    {
        std::string hex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        fprintf(stderr,"- buffer after write: %s, index %llx\n", hex.c_str(), ix);
    }
    bhs.unpackBuffer(buf);
    {
        std::string hex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        fprintf(stderr,"- buffer after unpack: %s, equal=%d\n", hex.c_str(), buf == copybuf ? 1 : 0);
    }
    return 0;
}
