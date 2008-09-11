#ifndef __SCAG_MTPERS_STORAGEPROCESSOR_H__
#define __SCAG_MTPERS_STORAGEPROCESSOR_H__

#include <vector>
#include <memory>

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "ConnectionContext.h"

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include "scag/pers/AbntAddr.hpp"
#include "scag/pers/Profile.h"
#include "core/buffers/PageFile.hpp"
#include "core/buffers/IntHash.hpp"

#include "scag/util/storage/BlocksHSStorage.h"
#include "scag/util/storage/RBTreeIndexStorage.h"
#include "scag/util/storage/DiskHashIndexStorage.h"
#include "scag/util/storage/HashedMemoryCache.h"
#include "scag/util/storage/StorageIface.h"
#include "scag/util/storage/PageFileDiskStorage.h"
#include "scag/util/storage/BHDiskStorage.h"
#include "scag/util/storage/Glossary.h"

#include "scag/pers/ProfileStore.h"

namespace scag { namespace mtpers {

using std::vector;
using std::string;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::EventMonitor;

using namespace scag::util::storage;
using scag::pers::AbntAddr;
using scag::pers::IntProfileKey;
using smsc::core::buffers::PageFile;
using smsc::core::buffers::IntHash;
using namespace smsc::util::config;

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

class StorageProcessor: public ThreadedTask {
public:
  StorageProcessor(unsigned maxWaitingCount);
  virtual ~StorageProcessor();
  virtual int Execute();
  virtual void stop();
  virtual const char * taskName();
  bool addContext(ConnectionContext* cx);
  virtual void process(ConnectionContext* cx) = 0;

protected:
  void initGlossary(const string& path, Glossary* glossary);

protected:
  vector<ConnectionContext* > waitingProcess_;
  unsigned maxWaitingCount_;
  EventMonitor processMonitor_;
  Logger* logger_;
};

class AbonentStorageProcessor: public StorageProcessor {
public:
  AbonentStorageProcessor(unsigned maxWaitingCount, unsigned locationNumber, unsigned storagesCount);
  ~AbonentStorageProcessor();
  void process(ConnectionContext* cx);
  void initElementStorage(const AbonentStorageConfig& cfg, unsigned index);

private:
  typedef HashedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef BHDiskStorage< AbntAddr, Profile > DiskDataStorage;
  typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > AbonentStorage;

  struct ElementStorage {
    ElementStorage(unsigned idx):index(idx), glossary(new Glossary()) {};
    ElementStorage():index(0), glossary(new Glossary()) {};
    AbonentStorage* storage;
    Glossary* glossary;
    unsigned index;
  };

private:
  IntHash<ElementStorage> elementStorages_;
  unsigned locationNumber_;
  unsigned storagesCount_;
  Logger* abntlog_;
};

class InfrastructStorageProcessor: public StorageProcessor {
public:
  InfrastructStorageProcessor(unsigned maxWaitingCount):StorageProcessor(maxWaitingCount), provider_(NULL), operator_(NULL), service_(NULL) {};
  ~InfrastructStorageProcessor();
  void init(const InfrastructStorageConfig& cfg);
  void process(ConnectionContext* cx) ;

private:
  typedef HashedMemoryCache< IntProfileKey, Profile > MemStorage;
  typedef PageFileDiskStorage< IntProfileKey, Profile, PageFile > DiskDataStorage;
  typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > InfrastructStorage;

private:
  InfrastructStorage* initStorage(const InfrastructStorageConfig& cfg);

private:
  InfrastructStorage* provider_;
  InfrastructStorage* service_;
  InfrastructStorage* operator_;
  Glossary glossary_;
  Logger* plog_;
  Logger* slog_;
  Logger* olog_;
};

}//mtpers
}//scag

#endif

