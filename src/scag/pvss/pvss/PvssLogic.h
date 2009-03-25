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

#include "scag/pvss/api/core/server/Server.h"
#include "ProfileCommandProcessor.h"

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
class AbstractProfileRequest;

class PvssLogic: public Server::SyncLogic {
public:
  PvssLogic():logger_(Logger::getInstance("storeproc")) {};
  virtual ~PvssLogic() {};
  Response* process(Request& request) throw(PvssException);
  void responseSent(std::auto_ptr<core::server::ServerContext> response) { /*TODO: implement this method*/ };
  void responseFail(std::auto_ptr<core::server::ServerContext> response) { /*TODO: implement this method*/ };

  virtual void shutdownStorages() = 0;

    /// initialize logic
    virtual void init() throw (smsc::util::Exception) = 0;
    /// return the name of the logic
    virtual std::string toString() const = 0;

protected:
  void initGlossary(const string& path, Glossary* glossary);
  virtual Response* processProfileRequest(AbstractProfileRequest& request) = 0;

protected:
  //unsigned maxWaitingCount_;
  Logger* logger_;
  ProfileCommandProcessor commandProcessor_;
};

struct AbonentStorageConfig;

class AbonentLogic: public PvssLogic {
public:
    AbonentLogic( unsigned nodeNumber, unsigned locationNumber, unsigned storagesCount, const AbonentStorageConfig& cfg ) :
    nodeNumber_(nodeNumber), locationNumber_(locationNumber), storagesCount_(storagesCount),
    abntlog_(smsc::logger::Logger::getInstance("pvss.abnt")), config_(cfg) {}
  ~AbonentLogic();

    virtual void init() throw (smsc::util::Exception);
    virtual std::string toString() const {
        char buf[64];
        snprintf(buf,sizeof(buf),"abonent logic #%u", locationNumber_);
        return buf;
    }

  //virtual Response* process(Request& request) throw(PvssException);
  void shutdownStorages();

protected:
    void initElementStorage(unsigned index) throw (smsc::util::Exception);

  virtual Response* processProfileRequest(AbstractProfileRequest& request);

private:
  //typedef HashedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef ArrayedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef BHDiskStorage< AbntAddr, Profile > DiskDataStorage;
  typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > AbonentStorage;

  struct ElementStorage {
    ElementStorage(unsigned idx):glossary(0), storage(0), index(idx) {};
    ElementStorage():glossary(0), storage(0), index(0) {};
    Glossary* glossary;
    AbonentStorage* storage;
    unsigned index;
  };

private:
  IntHash<ElementStorage> elementStorages_;
    unsigned nodeNumber_;
  unsigned locationNumber_;
  unsigned storagesCount_;
  Logger* abntlog_;
    const AbonentStorageConfig& config_;
};

struct InfrastructStorageConfig;

class InfrastructLogic: public PvssLogic {
public:
    InfrastructLogic( const InfrastructStorageConfig& cfg ) : 
    provider_(NULL), service_(NULL), operator_(NULL), config_(cfg),
    plog_(smsc::logger::Logger::getInstance("pvss.prov")),
    slog_(smsc::logger::Logger::getInstance("pvss.serv")),
    olog_(smsc::logger::Logger::getInstance("pvss.oper"))
    {}
  ~InfrastructLogic();

    virtual void init() throw (smsc::util::Exception);
    virtual std::strint toString() const { return "infrastruct logic"; }

  //virtual Response* process(Request& request) throw(PvssException);
  void shutdownStorages();

protected:
  virtual Response* processProfileRequest(AbstractProfileRequest& request);

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
  vector<string> locationPath;
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

