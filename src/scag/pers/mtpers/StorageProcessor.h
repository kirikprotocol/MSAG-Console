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
using namespace smsc::util::config;

struct StorageConfig {

  StorageConfig();
  StorageConfig(ConfigView& cfg, const char* storageType, Logger* logger);

  string dbName;
  string dbPath;
  uint32_t indexGrowth;
  uint32_t fileSize;
  uint32_t blockSize;
  uint32_t cacheSize;
  uint32_t recordCount;

};

class StorageManager;

class StorageProcessor: public ThreadedTask {
public:
  StorageProcessor(uint16_t maxWaitingCount);
  virtual ~StorageProcessor();
  virtual int Execute();
  virtual void stop();
  virtual const char * taskName();
  bool addContext(ConnectionContext* cx);
  virtual void process(ConnectionContext* cx) = 0;
  virtual void init(const StorageConfig& cfg) = 0;

protected:
  void initGlossary(const string& dbPath);

protected:
  vector<ConnectionContext* > waitingProcess_;
  uint16_t maxWaitingCount_;
  EventMonitor processMonitor_;
  Logger* logger_;
  Glossary glossary_;
};

class AbonentStorageProcessor: public StorageProcessor {
public:
  AbonentStorageProcessor(uint16_t maxWaitingCount, uint16_t storageIndex);
  ~AbonentStorageProcessor() {};
  void init(const StorageConfig& cfg);
  void process(ConnectionContext* cx) ;

private:
  typedef HashedMemoryCache< AbntAddr, Profile, DataBlockBackupTypeJuggling > MemStorage;
  typedef BHDiskStorage< AbntAddr, Profile > DiskDataStorage;
  typedef RBTreeIndexStorage< AbntAddr, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > AbonentStorage;

private:
  std::auto_ptr<AbonentStorage> storage_;
  uint16_t storageIndex_;
  Logger* abntlog_;
};

class InfrastructStorageProcessor: public StorageProcessor {
public:
  InfrastructStorageProcessor(uint16_t maxWaitingCount):StorageProcessor(maxWaitingCount) {};
  ~InfrastructStorageProcessor() {};
  void init(const StorageConfig& cfg);
  void process(ConnectionContext* cx) ;

private:
  typedef HashedMemoryCache< IntProfileKey, Profile > MemStorage;
  typedef PageFileDiskStorage< IntProfileKey, Profile, PageFile > DiskDataStorage;
  typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
  typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
  typedef CachedDiskStorage< MemStorage, DiskStorage > InfrastructStorage;

private:
  void initStorage(const StorageConfig& cfg, std::auto_ptr<InfrastructStorage>& storage);

private:
  std::auto_ptr<InfrastructStorage> provider_;
  std::auto_ptr<InfrastructStorage> service_;
  std::auto_ptr<InfrastructStorage> operator_;
  Logger* plog_;
  Logger* slog_;
  Logger* olog_;
};

}//mtpers
}//scag

#endif

