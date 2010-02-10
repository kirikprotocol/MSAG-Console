#include <memory>
#include "scag/pvss/profile/AbntAddr.hpp"
#include "scag/pvss/profile/Profile.h"
#include "BHDiskStorage2.h"
#include "PageFileDiskStorage.h"
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

// #define ABONENTSTORAGE

#ifdef ABONENTSTORAGE

typedef BHDiskStorage2< AbntAddr, Profile, BlocksHSStorage2 > DiskDataStorage;
typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;

#else // service storage

template < class PF > class PageFileDiskStorage2
{
public:
    static const bool updatable = false;

    typedef smsc::core::buffers::File::offset_type index_type;
    typedef PF                                     storage_type;
    typedef std::vector< unsigned char >           buffer_type;

    PageFileDiskStorage2( PF* pf, smsc::logger::Logger* thelog = 0 ) :
    pf_(pf), log_(thelog), keylogger_(0) {
        if (log_) smsc_log_debug(log_,"pagefilediskstorage created");
    }

    ~PageFileDiskStorage2() { delete pf_; }
    
    inline void setKeyLogger( KeyLogger& kl ) { keylogger_ = &kl; }

    inline index_type invalidIndex() const { return 0; }

    index_type append( const buffer_type& buf,
                       const buffer_type* oldbuf = 0 ) {
        const index_type i = pf_->Append( &(buf[0]), buf.size() );
        if (log_) smsc_log_debug(log_,"append: key=%s buf=%u -> index=%llx",
                                 keylogger_->toString(), unsigned(buf.size()),
                                 static_cast<unsigned long long>(i));
        return i;
    }

    /*
    index_type update( index_type i, const buffer_type& buf, buffer_type* oldbuf = 0 ) {
        if (log_) smsc_log_debug(log_,"update: key=%s index=%llx buf=%u",
                                 keylogger_->toString(),
                                 static_cast<unsigned long long>(i),
                                 unsigned(buf.size()));
        pf_->Update(i,&(buf[0]), buf.size());
    }
     */

    bool read( index_type i, buffer_type& buf ) {
        buf.resize(0);
        index_type j;
        if (log_) smsc_log_debug(log_,"reading key=%s index=%llx",
                                 keylogger_->toString(),
                                 static_cast<unsigned long long>(i));
        pf_->Read(i,buf,&j);
        if ( i != j ) {
            if (log_) smsc_log_warn(log_,"diff index: was=%llx is=%llx buf=%u",
                                    static_cast<unsigned long long>(i),
                                    static_cast<unsigned long long>(j),
                                    unsigned(buf.size()));
        } else {
            if (log_) smsc_log_debug(log_,"ok: buf=%u",
                                     unsigned(buf.size()));
        }
        return true;
    }

    inline void remove( index_type i, const buffer_type* oldbuf = 0 ) {
        if (log_) smsc_log_debug(log_,"remove: key=%s index=%llx",
                                 keylogger_->toString(),
                                 static_cast<unsigned long long>(i));
        pf_->Delete(i);
    }

private:
    PF*                   pf_;
    smsc::logger::Logger* log_;
    KeyLogger*            keylogger_;
};

typedef PageFileDiskStorage2< PageFile > DiskDataStorage;
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
        IntProfileKey pkey(unsigned(Drndm::uniform(100,rnd.getNextNumber())));
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
