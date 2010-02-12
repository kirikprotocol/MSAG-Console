#include <memory>
#include "scag/pvss/profile/AbntAddr.hpp"
#include "scag/pvss/profile/Profile.h"
#include "BHDiskStorage3.h"
#include "PageFileDiskStorage2.h"
#include "BlocksHSStorage2.h"
#include "RBTreeIndexStorage.h"
#include "IndexedStorage2.h"
#include "Glossary.h"
#include "DataBlockBackup.h"
#include "DiskHashIndexStorage.h"
#include "scag/util/io/Drndm.h"
#include "scag/util/io/HexDump.h"

using namespace scag2::util::storage;
using scag2::pvss::AbntAddr;
using scag2::pvss::IntProfileKey;
using scag2::pvss::Profile;
using scag2::util::Drndm;
using scag2::util::HexDump;

#define ABONENTSTORAGE

#ifdef ABONENTSTORAGE

typedef BHDiskStorage3 DiskDataStorage;
typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;

#else // service storage

typedef PageFileDiskStorage2 DiskDataStorage;
typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;

#endif // service storage

typedef IndexedStorage2< DiskIndexStorage, DiskDataStorage >  DiskStorage;


unsigned randomSize( Drndm& rnd )
{
    return unsigned(10+rnd.uniform(100,rnd.getNextNumber()));
}


int main()
{
    const std::string path = "temp";
    const char* indexName = "index";
    const unsigned indexGrowth = 10000;
    const bool  indexCleanup = false;
    const char* dataName = "data";
    const char* glossaryName = "glossary";

    unsigned dataAddSpeed = 10000;
    unsigned dataCountThr = 5000;
    unsigned dataPageSize = 0x80;
    unsigned dataFileSize = 0x1000;

    smsc::logger::Logger::initForTest(smsc::logger::Logger::LEVEL_DEBUG);

    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");
    smsc_log_info(logger,"started");

    try {
        smsc::core::buffers::File::RmDir( path.c_str() );
    } catch (...) {}
    try {
        smsc::core::buffers::File::MkDir( path.c_str(), 0777 );
    } catch (...) {}

    Glossary glossary;
    glossary.Open(path + '/' + glossaryName);

#ifdef ABONENTSTORAGE
    DataFileManager dfm(1, dataAddSpeed, dataCountThr );

    std::auto_ptr<DiskIndexStorage> dis
        (new DiskIndexStorage(indexName,
                              path,
                              indexGrowth,
                              indexCleanup,
                              smsc::logger::Logger::getInstance("pvssix")) );

    std::auto_ptr<DiskDataStorage::storage_type> bs
        (new DiskDataStorage::storage_type
         (dfm,smsc::logger::Logger::getInstance("pvssbh")));
    try {
        int ret = bs->open(dataName,path);
        if (ret < 0) throw smsc::util::Exception("cannot open dds");
    } catch ( std::exception& ) {
        int ret = bs->create(dataName,path, dataFileSize, dataPageSize );
        if (ret<0) {
            throw smsc::util::Exception("cannot create data disk storage");
        }
    }
        

#else

    std::auto_ptr<DiskIndexStorage> dis
        (new DiskIndexStorage(indexName,
                              path,
                              indexGrowth,
                              smsc::logger::Logger::getInstance("pvssix")) );

    std::auto_ptr<DiskDataStorage::storage_type> bs
        (new DiskDataStorage::storage_type());
    const std::string bsFileName(path + '/' + dataName);
    try {
        bs->Open(bsFileName);
    } catch ( std::exception& ) {
        bs->Create(path + '/' + dataName, dataPageSize, dataFileSize );
    }

#endif

    std::auto_ptr<DiskDataStorage> dds
        ( new DiskDataStorage(bs.release(),
                              smsc::logger::Logger::getInstance("pvssdd")) );

    DiskStorage ds(dis.release(),dds.release());

    smsc_log_info(logger,"disk storage created, size=%llu\n",static_cast<unsigned long long>(ds.size()));
    Drndm rnd;
    for ( unsigned i = 0; i < 200; ++i ) {
        uint64_t choice = Drndm::uniform(100,rnd.getNextNumber());

#ifdef ABONENTSTORAGE
        char pkeybuf[30];
        sprintf(pkeybuf,"+7913%07u",unsigned(Drndm::uniform(100,rnd.getNextNumber())));
        AbntAddr pkey;
        pkey.fromString(pkeybuf);
#else
        IntProfileKey pkey(unsigned(Drndm::uniform(100,rnd.getNextNumber())));
#endif

        if ( choice < 30 ) {
            smsc_log_debug(logger,"remove key=%s",pkey.toString().c_str());
            ds.remove(pkey);
        } else if ( choice < 60 ) {
            smsc_log_debug(logger,"reading key=%s",pkey.toString().c_str());
            DiskStorage::buffer_type buf;
            if ( ds.get(pkey,buf) ) {
                HexDump::string_type str;
                HexDump hd;
                hd.hexdump(str,&(buf[0]),buf.size());
                smsc_log_debug(logger,"ok buf=%u:%s",
                               unsigned(buf.size()), hd.c_str(str));
            } else {
                smsc_log_debug(logger,"not found");
            }
        } else {
            std::vector<uint64_t> newdata;
            const unsigned bsize = randomSize(rnd);
            rnd.getVector(bsize/8+1,newdata);
            DiskStorage::buffer_type buf;
            buf.resize(bsize);
            memcpy(&(buf[0]),&(newdata[0]),buf.size());
            HexDump::string_type str;
            HexDump hd;
            hd.hexdump(str,&(buf[0]),buf.size());
            smsc_log_debug(logger,"setting key=%s buf=%u:%s",
                           pkey.toString().c_str(), unsigned(buf.size()),
                           hd.c_str(str));
            if (ds.set(pkey,buf)) {
                smsc_log_debug(logger,"ok, was set");
            } else {
                smsc_log_debug(logger,"not set");
            }
        }
    }
    return 0;
}
