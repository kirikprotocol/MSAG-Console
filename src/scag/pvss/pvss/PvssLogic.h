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

#include "scag/util/storage/BlocksHSStorage.h"
#include "scag/util/storage/RBTreeIndexStorage.h"
#include "scag/util/storage/DiskHashIndexStorage.h"
//#include "scag/util/storage/HashedMemoryCache.h"
#include "scag/util/storage/ArrayedMemoryCache.h"
#include "scag/util/storage/StorageIface.h"
#include "scag/util/storage/PageFileDiskStorage.h"
#include "scag/util/storage/BHDiskStorage.h"
#include "scag/util/storage/Glossary.h"
#include "scag/util/WatchedThreadedTask.h"

#include "scag/pvss/api/core/server/Server.h"
#include "ProfileCommandProcessor.h"

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
        LogicInitTask( PvssLogic* logic ) : LogicTask(logic) {}
        virtual const char* taskName() { return "pvss.init"; }
        virtual int doExecute() {
            try {
                logic_->init();
            } catch ( std::exception& e ) {
                setFailure(e.what());
            } catch (...) {
                setFailure( "unknown exception" );
            }
            return 0;
        }
    };

    class LogicRebuildIndexTask : public LogicTask
    {
    public:
        LogicRebuildIndexTask( PvssLogic* logic ) : LogicTask(logic) {}
        virtual const char* taskName() { return "pvss.rbld"; }
        virtual int doExecute() {
            try {
                logic_->rebuildIndex();
            } catch ( std::exception& e ) {
                setFailure(e.what());
            } catch (...) {
                setFailure( "unknown exception" );
            }
            return 0;
        }
    };

public:
  PvssLogic( PvssDispatcher& dispatcher ) : dispatcher_(dispatcher), logger_(Logger::getInstance("storeproc")) {}
  virtual ~PvssLogic() {};
  Response* process(Request& request) /* throw(PvssException) */ ;
  void responseSent(std::auto_ptr<core::server::ServerContext> response) { /*TODO: implement this method*/ };
  void responseFail(std::auto_ptr<core::server::ServerContext> response) { /*TODO: implement this method*/ };

  virtual void shutdownStorages() = 0;

    /// start a task that initialize the logic
    virtual LogicInitTask* startInit() = 0;
    virtual LogicRebuildIndexTask* startRebuildIndex() = 0;

    /// return the name of the logic
    virtual std::string toString() const = 0;

protected:
  void initGlossary(const string& path, Glossary* glossary);

    // initialize logic
    virtual void init() = 0;
    virtual void rebuildIndex() = 0;

  virtual Response* processProfileRequest(ProfileRequest& request) = 0;

protected:
  //unsigned maxWaitingCount_;
    PvssDispatcher& dispatcher_;
    Logger* logger_;
  ProfileCommandProcessor commandProcessor_;
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

    virtual LogicInitTask* startInit();
    virtual LogicRebuildIndexTask* startRebuildIndex();

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
    virtual void init() /* throw (smsc::util::Exception) */;
    virtual void rebuildIndex();

    /// init an element storage and return the total number of good nodes in it
    unsigned long initElementStorage(unsigned index) /* throw (smsc::util::Exception) */;
    unsigned long rebuildElementStorage(unsigned index);

    virtual Response* processProfileRequest(ProfileRequest& request);

private:
    //typedef HashedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
    typedef ArrayedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
    typedef BHDiskStorage< AbntAddr, Profile > DiskDataStorage;
    typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
    typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
    typedef CachedDiskStorage< MemStorage, DiskStorage > AbonentStorage;

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

    virtual LogicInitTask* startInit();
    virtual LogicRebuildIndexTask* startRebuildIndex();
    virtual std::string toString() const { return "infrastruct logic"; }

  //virtual Response* process(Request& request) /* throw(PvssException) */ ;
  void shutdownStorages();

    std::string reportStatistics() const;

protected:
    virtual void init() /* throw (smsc::util::Exception) */;
    virtual void rebuildIndex();

  virtual Response* processProfileRequest(ProfileRequest& request);

private:
  //typedef HashedMemoryCache< IntProfileKey, Profile > MemStorage;
  //typedef PageFileDiskStorage< IntProfileKey, Profile, PageFile > DiskDataStorage;
  //typedef HashedMemoryCache< IntProfileKey, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef ArrayedMemoryCache< IntProfileKey, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef PageFileDiskStorage< IntProfileKey, DataBlockBackup<Profile>, PageFile > DiskDataStorage;
  typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > InfrastructStorage;

private:
  InfrastructStorage* initStorage(const InfrastructStorageConfig& cfg);

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
};

}//mtpers
}//scag

#endif

