#include "BlocksHSStorage2.h"
#include "scag/util/io/HexDump.h"
#include "logger/Logger.h"
#include "HSPacker.h"
#include "scag/util/io/Serializer.h"

using namespace scag2::util::storage;
using namespace scag2::util;
using namespace smsc::logger;
using scag2::util::io::Serializer;
using scag2::util::io::Deserializer;

struct DummyKey
{
public:
    DummyKey() {}
    DummyKey( uint64_t aKey ) : key(aKey) {}
    std::string toString() const {
        char buf[30];
        snprintf(buf,sizeof(buf),"%llu",key);
        return buf;
    }
    uint64_t key;
};


bool operator == ( const DummyKey& k, const DummyKey& l )
{
    return k.key == l.key;
}
bool operator != ( const DummyKey& k, const DummyKey& l )
{
    return ! (k == l);
}


Deserializer& operator >> ( Deserializer& dsr, DummyKey& key ) {
    return dsr >> key.key;
}
Serializer& operator << ( Serializer& ser, const DummyKey& key ) {
    return ser << key.key;
}


struct DummyData
{
public:
    DummyData() {}
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


bool operator == ( const DummyData& d, const DummyData& e )
{
    return d.value == e.value;
}
bool operator != ( const DummyData& d, const DummyData& e )
{
    return !(d == e);
}


// typedef BlocksHSStorage2 storage_type;
typedef HSPacker::buffer_type buffer_type;



void testPackerRun( HSPacker& bhs, buffer_type& buf, size_t initialPos )
{
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
        HexDump::string_type hex,hhex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        hd.hexdump(hhex,&headers[0],headers.size());
        hd.strdump(hhex,&headers[0],headers.size());
        fprintf(stderr,"- buffer before packing: %s\n- headers before packing: %s\n\n", hd.c_str(hex), hd.c_str(hhex));
    }
    bhs.packBuffer(buf,&headers,initialPos);
    {
        HexDump::string_type hex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        fprintf(stderr,"- buffer after packing: %s\n\n", hd.c_str(hex));
    }
    buffer_type newHeaders;
    bhs.unpackBuffer(buf,&newHeaders,initialPos);
    {
        HexDump::string_type hex,hhex;
        hd.hexdump(hex,&buf[0],buf.size());
        hd.strdump(hex,&buf[0],buf.size());
        hd.hexdump(hhex,&newHeaders[0],newHeaders.size());
        hd.strdump(hhex,&newHeaders[0],newHeaders.size());
        fprintf(stderr,"- buffer after unpack: %s\n- headers after unpack: %s\n\n- buf_equals=%d head_equals=%d\n",
                hd.c_str(hex), hd.c_str(hhex),
                buf == copybuf ? 1 : 0,
                headers == newHeaders ? 1 : 0 );
    }
    
    if ( buf != copybuf || headers != newHeaders ) {
        fprintf(stderr,"-- regression detected\n");
        ::abort();
    }
}


void testPacker()
{
    const size_t blockSize = 50;
    HSPacker bhs(blockSize,0);
    // BlocksHSStorage2 bhs(blockSize,32,smsc::logger::Logger::getInstance("bhs"));

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
        ser.setwpos( initialPos + bhs.idxSize() + bhs.navSize() );
        ser << key << data;
        
        testPackerRun( bhs, buf, initialPos );

    }
}


void testBHS2()
{
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");

    typedef BlocksHSStorage2 BHS;
    const size_t fileSize = 0x1000;
    const size_t blockSize = 0x100;
    const std::string dbname = "abonent-data";
    const std::string dbpath = "storage/0/000";
    const unsigned blockSpeed = 100;

    DataFileManager dfm(1,blockSpeed,fileSize/2);

    BHS bhs(dfm,smsc::logger::Logger::getInstance("bhs"));
    if ( 0 != bhs.open(dbname, dbpath) ) {
        if ( 0 != bhs.create(dbname,dbpath,fileSize,blockSize) ) {
            smsc_log_fatal(logger,"cannot create storage: cannot proceed");
            return;
        } else {
            smsc_log_info(logger,"storage has been created");
            return;
        }
    }

    for ( size_t i = 0; i < blockSize; ++i ) {

        std::string sdata("hello, world");
        sdata.reserve(i*10);
        for ( size_t j = 0; j < i; ++j ) {
            sdata.push_back(char('A'+j));
        }

        DummyKey key(79137654079ULL);
        DummyData data(sdata);

        buffer_type buf;
        {
            Serializer ser(buf);
            ser.setVersion(bhs.version());
            ser.setwpos(bhs.headerSize());
            ser << key << data;
        }
        smsc_log_info(logger,"\n---------------------------- buffer size=%u -------------------",
                      unsigned(buf.size()));
        bhs.packBuffer(buf,0);
        BHS::index_type idx = bhs.change(bhs.invalidIndex(),0,&buf);
        const buffer_type postchangebuf(buf);
        if ( idx == bhs.invalidIndex() ) {
            smsc_log_error(logger,"cannot create at %u", unsigned(i));
            ::abort();
        }

        // reading
        buffer_type newbuf;
        if ( ! bhs.read(idx,newbuf) ) {
            smsc_log_error(logger,"cannot read at %llx", idx);
            ::abort();
        }
        
        if ( newbuf != postchangebuf ) {
            smsc_log_error(logger,"postchange buffers are not equal");
            ::abort();
        }
        
        DummyKey newkey;
        DummyData newdata;
        {
            const buffer_type preunpack(newbuf);
            buffer_type headers;
            bhs.unpackBuffer(newbuf,&headers);
            Deserializer dsr(newbuf);
            dsr.setVersion(bhs.version());
            dsr.setrpos(bhs.headerSize());
            dsr >> newkey >> newdata;
            bhs.packBuffer(newbuf,&headers);
            if ( preunpack != newbuf ) {
                smsc_log_error(logger,"unpack*pack != 1");
                ::abort();
            }
        }

        if ( key != newkey ) {
            smsc_log_error(logger,"keys are not equal: %s != %s",
                           key.toString().c_str(),
                           newkey.toString().c_str() );
            ::abort();
        }
        if ( data != newdata ) {
            smsc_log_error(logger,"data are not equal: %s != %s",
                           data.toString().c_str(),
                           newdata.toString().c_str() );
            ::abort();
        }
    }

    dfm.shutdown();

}


int main()
{
    Logger::Init();
    // testPacker();
    testBHS2();
    return 0;
}
