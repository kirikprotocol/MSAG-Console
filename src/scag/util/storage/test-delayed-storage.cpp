#include <memory>
#include "scag/util/io/Drndm.h"
#include "scag/util/io/HexDump.h"
#include "scag/pvss/profile/AbntAddr.hpp"
#include "scag/pvss/profile/Profile.h"
#include "scag/pvss/data/Property.h"

#include "PageFileDiskStorage2.h"
#include "DiskHashIndexStorage.h"

#include "BlocksHSStorage2.h"
#include "BHDiskStorage3.h"

#include "RBTreeIndexStorage.h"
#include "IndexedStorage2.h"

#include "Glossary.h"
#include "ArrayedMemoryCache.h"

#include "DataBlockBackup2.h"
#include "CachedDelayedDiskStorage.h"
#include "scag/util/io/Serializer.h"
#include "scag/util/io/GlossaryBase.h"

using namespace scag2::util::storage;
using scag2::pvss::AbntAddr;
using scag2::pvss::IntProfileKey;
using scag2::pvss::Profile;
using scag2::pvss::ProfileBackup;
using scag2::pvss::Property;
using scag2::util::Drndm;
using scag2::util::HexDump;
using scag2::util::io::Serializer;
using scag2::util::io::Deserializer;
using scag2::util::io::GlossaryBase;

// #define ABONENTSTORAGE

template < class Key > struct ProfileHeapAllocator
{
public:
    void setProfileBackup( ProfileBackup* b ) { plog_ = b; }
protected:
    ProfileHeapAllocator() : plog_(0) {}
    ~ProfileHeapAllocator() {}
    inline Profile* alloc( const Key& k ) const { return new Profile(k,plog_); }
private:
    ProfileBackup* plog_;
};


template < class MemStorage, class DiskStorage > struct ProfileSerializer
{
public:
    typedef typename MemStorage::key_type     key_type;
    typedef typename MemStorage::stored_type  stored_type;
    typedef typename DiskStorage::buffer_type buffer_type;

    ProfileSerializer( DiskStorage* disk,
                       GlossaryBase* glossary = 0 ) :
    disk_(disk), glossary_(glossary), newbuf_(0), ownbuf_(0) {
        if (!disk) throw smsc::util::Exception("disk storage should be provided");
    }

    ~ProfileSerializer() {
        delete glossary_;
        if (newbuf_) delete newbuf_;
    }

    /// --- reading

    /// making a new buffer
    buffer_type* getFreeBuffer( bool create = false ) {
        if (!newbuf_ && create) { newbuf_ = new buffer_type; }
        return newbuf_; 
    }

    buffer_type* getOwnedBuffer() {
        return ownbuf_;
    }

    /// deserialization && attaching the buffer
    bool deserialize( const key_type&, stored_type& val ) {
        assert(newbuf_ && val.value );
        Deserializer dsr(*newbuf_,glossary_);
#ifdef ABONENTSTORAGE
        disk_->unpackBuffer(*newbuf_,&hdrbuf_);
        dsr.setrpos(disk_->headerSize());
        dsr >> *val.value;
        disk_->packBuffer(*newbuf_,&hdrbuf_);
#else
        dsr >> *val.value;
#endif
        std::swap(val.backup,newbuf_);
        ownbuf_ = val.backup;
        return true;
    }

    /// --- writing
    void serialize( const key_type&, stored_type& val ) {
        assert( val.value );
        if (!newbuf_) { newbuf_ = new buffer_type; }
        Serializer ser(*newbuf_,glossary_);
#ifdef ABONENTSTORAGE
        ser.setwpos(disk_->headerSize());
        ser << *val.value;
        disk_->packBuffer(*newbuf_);
#else
        ser << *val.value;
#endif
        std::swap(val.backup,newbuf_);
        ownbuf_ = val.backup;
    }

private:
    DiskStorage*  disk_;     // not owned
    GlossaryBase* glossary_; // owned
    buffer_type*  newbuf_;   // owned
    buffer_type*  ownbuf_;   // not owned (owned by other stored_type instance)
    buffer_type   hdrbuf_;
};


#ifdef ABONENTSTORAGE

typedef BHDiskStorage3 DiskDataStorage;
typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
typedef ArrayedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling2 > MemStorage;

#else // service storage

typedef PageFileDiskStorage2 DiskDataStorage;
typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
typedef ArrayedMemoryCache< IntProfileKey, Profile, DataBlockBackupTypeJuggling2 > MemStorage;

#endif // service storage

typedef IndexedStorage2< DiskIndexStorage, DiskDataStorage >  DiskStorage;
typedef ProfileSerializer< MemStorage, DiskStorage > DataSerializer;
typedef CachedDelayedDiskStorage< MemStorage, DiskStorage, DataSerializer, ProfileHeapAllocator< MemStorage::key_type > > CachedStorage;

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

    std::auto_ptr<DiskStorage> ds(new DiskStorage(dis.release(),dds.release()));

    smsc_log_info(logger,"disk storage created, size=%llu\n",static_cast<unsigned long long>(ds->size()));

    std::auto_ptr<MemStorage> ms(new MemStorage(smsc::logger::Logger::getInstance("pvssmc")));
    
    Glossary* glossary = new Glossary();
    glossary->Open(path + '/' + glossaryName);
    std::auto_ptr<DataSerializer> ps( new DataSerializer(ds.get(),glossary) );

    CachedStorage st(ms.release(),ds.release(),ps.release(),
                     smsc::logger::Logger::getInstance("pvssst"));
    // st.setProfileLog(smsc::logger::Logger::getInstance("prof"));

    Drndm rnd;
    for ( unsigned i = 0; i < 200; ++i ) {

        {
            struct timespec ts = {0,1000000};
            nanosleep(&ts,0);
        }

        uint64_t choice = Drndm::uniform(100,rnd.getNextNumber());

#ifdef ABONENTSTORAGE
        char pkeybuf[30];
        sprintf(pkeybuf,"+7913%07u",unsigned(Drndm::uniform(100,rnd.getNextNumber())));
        AbntAddr pkey;
        pkey.fromString(pkeybuf);
#else
        IntProfileKey pkey(unsigned(Drndm::uniform(100,rnd.getNextNumber())));
#endif

        char propname[30];
        sprintf(propname,"prop%02u",unsigned(Drndm::uniform(100,rnd.getNextNumber())));

        smsc_log_debug(logger,"--------------------");

        if ( choice < 10 ) {

            // removing property
            smsc_log_debug(logger,"remove key=%s prop=%s",pkey.toString().c_str(),propname);
            Profile* pf = st.get(pkey);
            if (!pf) {
                smsc_log_debug(logger,"profile key=%s not found",pkey.toString().c_str());
                continue;
            }
            if (!pf->DeleteProperty(propname)) {
                smsc_log_debug(logger,"property=%s is not found",propname);
                continue;
            }
            smsc_log_debug(logger,"property=%s is removed",propname);
            st.markDirty(pkey);

        } else if ( choice < 40 ) {

            smsc_log_debug(logger, "get key=%s prop=%s",pkey.toString().c_str(),propname);
            Profile* pf = st.get(pkey);
            if (!pf) {
                smsc_log_debug(logger,"profile key=%s not found",pkey.toString().c_str());
                continue;
            }
            Property* prop = pf->GetProperty(propname);
            if (!prop) {
                smsc_log_debug(logger,"property=%s is not found",propname);
                continue;
            }
            smsc_log_debug(logger,"property=%s got: %s",propname,prop->toString().c_str());
            st.flushDirty();
            
        } else {

            const unsigned bsize = randomSize(rnd);
            std::vector<uint64_t> newdata;
            rnd.getVector(bsize/8+1,newdata);
            HexDump::string_type str;
            HexDump hd;
            hd.hexdump(str,&(newdata[0]),bsize);

            smsc_log_debug( logger, "setting key=%s prop=%s to val=%u:%s",
                            pkey.toString().c_str(), propname, unsigned(bsize),
                            hd.c_str(str) );

            Profile* pf = st.get(pkey,true);
            if (!pf) {
                smsc_log_error(logger,"profile key=%s CANNOT BE CREATED!",pkey.toString().c_str());
                continue;
            }
            Property* prop = pf->GetProperty(propname);
            if ( !prop ) {
                pf->AddProperty(Property(propname,hd.c_str(str),
                                         scag2::pvss::perstypes::FIXED,
                                         time_t(-1),100));
            } else {
                prop->setStringValue(hd.c_str(str));
            }
            st.markDirty(pkey);
        }

    } // loop
    return 0;
}
