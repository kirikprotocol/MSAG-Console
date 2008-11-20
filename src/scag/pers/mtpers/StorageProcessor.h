#ifndef __SCAG_MTPERS_STORAGEPROCESSOR_H__
#define __SCAG_MTPERS_STORAGEPROCESSOR_H__

#include <vector>
#include <memory>

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "scag/pers/util/AbntAddr.hpp"
#include "scag/pers/util/Profile.h"
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

#include "PersCommand.h"

namespace scag { namespace mtpers {

using std::vector;
using std::string;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::EventMonitor;

using namespace scag::util::storage;
using scag::pers::util::AbntAddr;
using scag::pers::util::IntProfileKey;
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
  bool addPacket(PersPacket* packet);

protected:
  void initGlossary(const string& path, Glossary* glossary);
  virtual void process(PersPacket* packet) = 0;
  virtual void shutdownStorages() = 0;

protected:
  vector<PersPacket* > waitingProcess_;
  unsigned maxWaitingCount_;
  EventMonitor processMonitor_;
  Logger* logger_;
  Logger* debuglogger_;
};

class AbonentStorageProcessor: public StorageProcessor {
public:
  AbonentStorageProcessor(unsigned maxWaitingCount, unsigned locationNumber, unsigned storagesCount);
  ~AbonentStorageProcessor();
  void initElementStorage(const AbonentStorageConfig& cfg, unsigned index);

protected:
  void process(PersPacket* packet);
  void shutdownStorages();

private:
  typedef HashedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef BHDiskStorage< AbntAddr, Profile > DiskDataStorage;
  typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > AbonentStorage;

  struct ElementStorage {
    ElementStorage(unsigned idx):glossary(0), storage(0), index(idx) {};
    ElementStorage():glossary(0), storage(0), index(0) {};
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
  InfrastructStorageProcessor(unsigned maxWaitingCount):StorageProcessor(maxWaitingCount), provider_(NULL), service_(NULL), operator_(NULL) {};
  ~InfrastructStorageProcessor();
  void init(const InfrastructStorageConfig& cfg);

protected:
  void process(PersPacket* packet);
  void shutdownStorages();

private:
  //typedef HashedMemoryCache< IntProfileKey, Profile > MemStorage;
  //typedef PageFileDiskStorage< IntProfileKey, Profile, PageFile > DiskDataStorage;
  typedef HashedMemoryCache< IntProfileKey, Profile, DataBlockBackupTypeJuggling > MemStorage;
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
  Glossary glossary_;
  Logger* plog_;
  Logger* slog_;
  Logger* olog_;
};

}//mtpers
}//scag

#endif

