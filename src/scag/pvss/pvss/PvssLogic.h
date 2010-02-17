#ifndef _SCAG_PVSS_PVSS_PVSSLOGIC_H_
#define _SCAG_PVSS_PVSS_PVSSLOGIC_H_

#include <vector>
#include <memory>

#include "logger/Logger.h"

#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "scag/pvss/profile/AbntAddr.hpp"
#include "scag/pvss/profile/Profile.h"
#include "core/buffers/PageFile.hpp"
#include "core/buffers/IntHash.hpp"

#include "scag/util/storage/DataBlockBackup.h"

// please uncomment the following line to get the new code
#define PVSSLOGIC_BHS2 1

#ifdef PVSSLOGIC_BHS2

#include "scag/util/storage/BlocksHSStorage2.h"
#include "scag/util/storage/BHDiskStorage2.h"

#else

#include "scag/util/storage/BlocksHSStorage3.h"
#include "scag/util/storage/BHDiskStorage.h"

#endif

#include "scag/util/storage/RBTreeIndexStorage.h"
#include "scag/util/storage/DiskHashIndexStorage.h"
#include "scag/util/storage/ArrayedMemoryCache.h"
#include "scag/util/storage/StorageIface.h"
#include "scag/util/storage/IndexedStorage2.h"
#include "scag/util/storage/PageFileDiskStorage2.h"
#include "scag/util/storage/CachedDelayedDiskStorage.h"
#include "scag/util/storage/Glossary.h"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/pvss/api/core/server/Server.h"
#include "ProfileCommandProcessor.h"
#include "ProfileBackup.h"
#include "scag/util/io/Serializer.h"

namespace scag {
namespace util {
namespace storage {
class DataFileManager;
}
}
}

namespace scag2 {
namespace pvss {

using std::vector;
using std::string;

using namespace scag::util::storage;
using smsc::core::buffers::PageFile;
using smsc::core::buffers::IntHash;
using namespace smsc::util::config;
using core::server::Server;

class Request;
class Response;
class ProfileRequest;
class PvssDispatcher;

class PvssLogic;

class LogicTask : public util::WatchedThreadedTask
{
public:
    LogicTask( PvssLogic* logic ) : logic_(logic) {}
    const std::string& getFailure() const { return failure_; }
protected:
    inline void setFailure( const char* what );
    // failure_ = std::string(what) + " in " + logic_->toString();
protected:
    PvssLogic* logic_;
    std::string failure_;
};


class PvssLogic: public Server::SyncLogic 
{
protected:
    class LogicInitTask : public LogicTask
    {
    public:
        LogicInitTask( PvssLogic* logic, bool checkAtStart = false ) :
        LogicTask(logic), checkAtStart_(checkAtStart) {}
        virtual const char* taskName() { return "pvss.init"; }
        virtual int doExecute() {
            try {
                logic_->init( checkAtStart_ );
            } catch ( std::exception& e ) {
                setFailure(e.what());
            } catch (...) {
                setFailure( "unknown exception" );
            }
            return 0;
        }
    private:
        bool checkAtStart_;
    };

    class LogicRebuildIndexTask : public LogicTask
    {
    public:
        LogicRebuildIndexTask( PvssLogic* logic, unsigned maxSpeed = 0 ) :
        LogicTask(logic), maxSpeed_(maxSpeed) {}
        virtual const char* taskName() { return "pvss.rbld"; }
        virtual int doExecute() {
            try {
                logic_->rebuildIndex(maxSpeed_);
            } catch ( std::exception& e ) {
                setFailure(e.what());
            } catch (...) {
                setFailure( "unknown exception" );
            }
            return 0;
        }
    private:
        unsigned maxSpeed_;
    };

public:
  PvssLogic( PvssDispatcher& dispatcher ) : dispatcher_(dispatcher), logger_(Logger::getInstance("pvss.proc")) {}
  virtual ~PvssLogic() {};
  Response* process(Request& request) /* throw(PvssException) */ ;
  void responseSent(std::auto_ptr<core::server::ServerContext> response) { /*TODO: implement this method*/ };
  void responseFail(std::auto_ptr<core::server::ServerContext> response) { /*TODO: implement this method*/ };

  virtual void shutdownStorages() = 0;

    /// start a task that initialize the logic
    virtual LogicInitTask* startInit( bool checkAtStart = false ) = 0;
    virtual LogicRebuildIndexTask* startRebuildIndex(unsigned maxSpeed = 0) = 0;
    virtual void dumpStorage( int index ) = 0;

    /// return the name of the logic
    virtual std::string toString() const = 0;

protected:
  void initGlossary(const string& path, Glossary* glossary);

    // initialize logic
    virtual void init( bool checkAtStart = false ) = 0;
    virtual void rebuildIndex(unsigned maxSpeed = 0) = 0;

  virtual Response* processProfileRequest(ProfileRequest& request) = 0;

    template < class Key > struct ProfileHeapAllocator
    {
        ProfileHeapAllocator() : plog_(0) {}
        void setProfileLog( smsc::logger::Logger* plog ) { plog_ = plog; }
        inline Profile* alloc( const Key& k ) const {
            return new Profile(k,plog_);
        }
    protected:
        smsc::logger::Logger* plog_;
    };


protected:
  //unsigned maxWaitingCount_;
    PvssDispatcher& dispatcher_;
    Logger* logger_;
  ProfileCommandProcessor commandProcessor_;
    ProfileBackup profileBackup_;
};


inline void LogicTask::setFailure( const char* what )
{
    failure_ = std::string(what) + " in " + logic_->toString();
}


struct AbonentStorageConfig;

class AbonentLogic: public PvssLogic 
{
public:
    AbonentLogic( PvssDispatcher& dispatcher, unsigned locationNumber, const AbonentStorageConfig& cfg, scag::util::storage::DataFileManager& manager ) :
    PvssLogic(dispatcher),
    locationNumber_(locationNumber),
    abntlog_(smsc::logger::Logger::getInstance("pvss.abnt")), config_(cfg), dataFileManager_(manager) {}

    virtual ~AbonentLogic();

    virtual LogicInitTask* startInit( bool checkAtStart = false );
    virtual LogicRebuildIndexTask* startRebuildIndex(unsigned maxSpeed = 0);
    virtual void dumpStorage( int index );

    virtual std::string toString() const {
        char buf[64];
        snprintf(buf,sizeof(buf),"abonent logic #%u", locationNumber_);
        return buf;
    }

    //virtual Response* process(Request& request) /* throw(PvssException) */ ;
    void shutdownStorages();

    /// return the number of abonents in this logic's storages
    unsigned long reportStatistics() const;

    inline unsigned getLocationNumber() const { return locationNumber_; }

protected:
    virtual void init( bool checkAtStart = false ) /* throw (smsc::util::Exception) */;
    virtual void rebuildIndex( unsigned maxSpeed = 0);

    /// init an element storage and return the total number of good nodes in it
    unsigned long initElementStorage(unsigned index,
                                     bool checkAtStart = false,
                                     bool readonly = false ) /* throw (smsc::util::Exception) */;
    unsigned long rebuildElementStorage(unsigned index,unsigned maxSpeed);
    void dumpElementStorage( unsigned index );

    virtual Response* processProfileRequest(ProfileRequest& request);

private:
    
    //typedef HashedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
    // typedef ArrayedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
    // typedef BHDiskStorage< AbntAddr, Profile > DiskDataStorage;
    typedef ArrayedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling2 > MemStorage;
#ifdef PVSSLOGIC_BHS2
    typedef BHDiskStorage2< AbntAddr, Profile, BlocksHSStorage2 > DiskDataStorage;
#else
    typedef BHDiskStorage< AbntAddr, Profile, BlocksHSStorage3<AbntAddr,Profile> > DiskDataStorage;
#endif
    typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
    typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
    typedef CachedDiskStorage< MemStorage, DiskStorage, ProfileHeapAllocator<AbntAddr> > AbonentStorage;

    struct ElementStorage {
        ElementStorage(unsigned idx):glossary(0), storage(0), index(idx) {}
        ElementStorage(): glossary(0), storage(0), index(0) {}
        smsc::core::synchronization::Mutex mutex;
        Glossary* glossary;
        AbonentStorage* storage;
        unsigned index;
    };

private:
    IntHash<ElementStorage*> elementStorages_;
    unsigned locationNumber_;
    Logger* abntlog_;
    const AbonentStorageConfig& config_;
    scag::util::storage::DataFileManager& dataFileManager_;
};







struct InfrastructStorageConfig;

class InfrastructLogic: public PvssLogic {
public:
    InfrastructLogic( PvssDispatcher& dispatcher, const InfrastructStorageConfig& cfg ) :
    PvssLogic(dispatcher),
    provider_(NULL), service_(NULL), operator_(NULL), config_(cfg),
    plog_(smsc::logger::Logger::getInstance("pvss.prov")),
    slog_(smsc::logger::Logger::getInstance("pvss.serv")),
    olog_(smsc::logger::Logger::getInstance("pvss.oper"))
    {}
  ~InfrastructLogic();

    virtual LogicInitTask* startInit( bool checkAtStart = false );
    virtual LogicRebuildIndexTask* startRebuildIndex( unsigned maxSpeed = 0);
    virtual void dumpStorage( int index );
    virtual std::string toString() const { return "infrastruct logic"; }

  //virtual Response* process(Request& request) /* throw(PvssException) */ ;
  void shutdownStorages();

    std::string reportStatistics() const;

    virtual void keepAlive() {
        util::msectime_type now = util::currentTimeMillis();
        MutexGuard mg(statMutex_);
        provider_->flushDirty(now);
        service_->flushDirty(now);
        operator_->flushDirty(now);
    }

protected:
    virtual void init( bool checkAtStart = false ) /* throw (smsc::util::Exception) */;
    virtual void rebuildIndex( unsigned maxSpeed = 0 );

  virtual Response* processProfileRequest(ProfileRequest& request);


    template < class MemStorage, class DiskStorage > struct ProfileSerializer
    {
    public:
        typedef typename MemStorage::stored_type  stored_type;
        typedef typename DiskStorage::buffer_type buffer_type;

        ProfileSerializer( DiskStorage* disk,
                           GlossaryBase* glossary = 0 ) :
        disk_(disk), glossary_(glossary), newbuf_(0), ownbuf_(0) {
            if (!disk) throw smsc::util::Exception("disk storage should be provided");
        }

        ~ProfileSerializer() {
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
        bool deserialize( stored_type& val ) {
            assert(newbuf_ && val.value );
            util::io::Deserializer dsr(*newbuf_,glossary_);
// #ifdef ABONENTSTORAGE
//            disk_->unpackBuffer(*newbuf_,&hdrbuf_);
//            dsr.setrpos(disk_->headerSize());
//            dsr >> *val.value;
//            disk_->packBuffer(*newbuf_,&hdrbuf_);
//#else
            dsr >> *val.value;
//#endif
            
            std::swap(val.backup,newbuf_);
            ownbuf_ = val.backup;
            return true;
        }

        /// --- writing
        void serialize( stored_type& val ) {
            assert( val.value );
            if (!newbuf_) { newbuf_ = new buffer_type; }
            util::io::Serializer ser(*newbuf_,glossary_);
//#ifdef ABONENTSTORAGE
//            ser.setwpos(disk_->headerSize());
//            ser << *val.value;
//            disk_->packBuffer(*newbuf_);
//#else
            ser << *val.value;
//#endif
            std::swap(val.backup,newbuf_);
            ownbuf_ = val.backup;
        }

    private:
        DiskStorage*  disk_;     // not owned
        GlossaryBase* glossary_; // not owned
        buffer_type*  newbuf_;   // owned
        buffer_type*  ownbuf_;   // not owned (owned by other stored_type instance)
        buffer_type   hdrbuf_;
    };

private:
  // typedef ArrayedMemoryCache< IntProfileKey, Profile, DataBlockBackupTypeJuggling > MemStorage;
  // typedef PageFileDiskStorage< IntProfileKey, DataBlockBackup<Profile>, PageFile > DiskDataStorage;
  // typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
  // typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  // typedef CachedDiskStorage< MemStorage, DiskStorage, ProfileHeapAllocator< IntProfileKey > > InfrastructStorage;
    typedef PageFileDiskStorage2 DiskDataStorage;
    typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
    typedef ArrayedMemoryCache< IntProfileKey, Profile, DataBlockBackupTypeJuggling2 > MemStorage;
    typedef IndexedStorage2< DiskIndexStorage, DiskDataStorage > DiskStorage;
    typedef ProfileSerializer< MemStorage, DiskStorage > DataSerializer;
    typedef CachedDelayedDiskStorage< MemStorage, DiskStorage, DataSerializer, ProfileHeapAllocator< MemStorage::key_type > > InfrastructStorage;

private:
  InfrastructStorage* initStorage( const InfrastructStorageConfig& cfg,
                                   bool checkAtStart = false,
                                   const std::string& logsfx = "" );

private:
  InfrastructStorage* provider_;
  InfrastructStorage* service_;
  InfrastructStorage* operator_;
    const InfrastructStorageConfig& config_;
  Glossary glossary_;
  Logger* plog_;
  Logger* slog_;
  Logger* olog_;
    // used to access statistics of disk storages
    mutable smsc::core::synchronization::Mutex statMutex_;
};

struct AbonentStorageConfig {

  AbonentStorageConfig();
  AbonentStorageConfig(ConfigView& cfg, const char* storageType, Logger* logger);

  string dbName;
  string dbPath;
  unsigned indexGrowth;
  unsigned fileSize;
  unsigned blockSize;
  unsigned cacheSize;
  bool     checkAtStart;
  struct Location {
    Location(const string& locpath, unsigned locdisk):path(locpath), disk(locdisk) {};
    string path;
    unsigned disk;
  };
  vector<Location> locations;
};

struct InfrastructStorageConfig {
  InfrastructStorageConfig();
  InfrastructStorageConfig(ConfigView& cfg, const char* storageType, Logger* logger);
  string dbName;
  string dbPath;
  string localPath;
  uint32_t cacheSize;
  uint32_t recordCount;
    uint32_t minDirtyTime;  // 10
    uint32_t maxDirtyTime;  // 1000
    uint32_t maxDirtyCount; // 100
  bool    checkAtStart;
};

}//mtpers
}//scag

#endif

