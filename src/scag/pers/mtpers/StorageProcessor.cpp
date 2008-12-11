#include "StorageProcessor.h"
#include "core/synchronization/MutexGuard.hpp"
#include "core/buffers/File.hpp"
#include <exception>
#include "scag/util/RelockMutexGuard.h"
#include "StorageManager.h"


namespace scag { namespace mtpers {

using smsc::core::synchronization::MutexGuard;
using smsc::core::buffers::File;
using smsc::core::buffers::FileException;
using scag::util::RelockMutexGuard;

const char* DEF_STORAGE_NAME    = "abonent";
const char* DEF_LOCAL_PATH      = "infrastruct";
const char* DEF_STORAGE_PATH    = "./storage";
const char* DEF_GLOSSARY_NAME   = "/glossary";
const uint32_t DEF_INDEX_GROWTH = 1000;
const uint32_t DEF_BLOCK_SIZE   = 2048;
const uint32_t DEF_FILE_SIZE    = 50000;
const uint32_t DEF_CACHE_SIZE   = 1000;
const uint32_t DEF_RECORD_COUNT = 1000;

StorageProcessor::StorageProcessor(unsigned maxWaitingCount, StorageManager* manager): maxWaitingCount_(maxWaitingCount), manager_(manager)
{
  logger_ = Logger::getInstance("storeproc");
  debuglogger_ = Logger::getInstance("ctx");
}


void StorageProcessor::initGlossary(const string& path, Glossary* glossary) {
  if (!File::Exists(path.c_str())) {
    smsc_log_debug(logger_, "create storage dir '%s'", path.c_str());
    File::MkDir(path.c_str());
  }
  string fn = path + DEF_GLOSSARY_NAME;
  if ( GlossaryBase::SUCCESS != glossary->Open(fn) ) {
    throw std::runtime_error("StorageProcessor: glossary open error");
  }
}

StorageProcessor::~StorageProcessor() {
}


int StorageProcessor::Execute() {
  vector<PersPacket*> process;
  smsc_log_debug(logger_, "%p started", this);
  for (;;) {
    {
      MutexGuard g(processMonitor_);

      while (waitingProcess_.empty() && !isStopping) {
        smsc_log_debug(logger_, "%p idle", this);
        processMonitor_.wait();         
        smsc_log_debug(logger_, "%p notified", this);
      }
      if (isStopping) {
        break;
      }
      process.assign(waitingProcess_.begin(), waitingProcess_.end());
      waitingProcess_.clear();
    }
    smsc_log_debug(logger_, "%p: %d packets ready for process", this, process.size());
    int pn = 1;
    for (vector<PersPacket*>::iterator i = process.begin(); i != process.end(); ++i) {
      smsc_log_debug(logger_, "%p: process packet %p - number %d", this, *i, pn);
      ++pn;
      try {

        this->process(*i);

      } catch (const SerialBufferOutOfBounds &e) {
        smsc_log_warn(logger_, "%p: %p processing error: SerialBufferOutOfBounds", this, *i);
        (*i)->createResponse(scag::pers::util::RESPONSE_ERROR);
      } catch (const std::runtime_error &e) {
        smsc_log_warn(logger_, "%p: %p processing error: std::runtime_error: %s", this, *i, e.what());
        (*i)->createResponse(scag::pers::util::RESPONSE_ERROR);
      } catch (const FileException &e) {
        smsc_log_warn(logger_, "%p: %p processing error: FileException: %s", this, *i, e.what());
        (*i)->createResponse(scag::pers::util::RESPONSE_ERROR);
      } catch (const std::exception &e) {
        smsc_log_warn(logger_, "%p: %p processing error: std::exception: %s", this, *i, e.what());
        (*i)->createResponse(scag::pers::util::RESPONSE_ERROR);
      } catch (...) {
        smsc_log_warn(logger_, "%p: %p processing error: unknown exception", this, *i);      
        (*i)->createResponse(scag::pers::util::RESPONSE_ERROR);
      }
      (*i)->sendResponse();
      smsc_log_info(debuglogger_, "complete seq.number:%d packet:%p key:%d/'%s'",
                    (*i)->getSequenceNumber(), *i, (*i)->intKey, (*i)->address.toString().c_str());      
      delete *i;
    }
    process.clear();
  }
  smsc_log_debug(logger_, "stopping storage processor %p ...", this);
  {
    MutexGuard g(processMonitor_);
    for (vector<PersPacket*>::iterator i = waitingProcess_.begin(); i != waitingProcess_.end(); ++i) {
      if (*i) {
        delete *i;
      }
    }
    waitingProcess_.clear();
  }
  shutdownStorages();
  smsc_log_debug(logger_, "stoped storage processor %p", this);
  manager_->procStopped();
  return 0;
}

void StorageProcessor::stop() {
  smsc_log_debug(logger_, "stop storage processor %p", this);

  MutexGuard g(processMonitor_);
  isStopping = true;
  processMonitor_.notify();
}

const char * StorageProcessor::taskName() {
  return "StorageProcessor";
}

bool StorageProcessor::addPacket(PersPacket* packet) {
  RelockMutexGuard g(processMonitor_);
  if (waitingProcess_.size() >= maxWaitingCount_) {
    g.Unlock();
    smsc_log_warn(logger_, "%p: %p waiting process queue limit %d", this, packet, maxWaitingCount_);
    return false;
  }
  waitingProcess_.push_back(packet);
  processMonitor_.notify();  
  return true;
}

AbonentStorageProcessor::AbonentStorageProcessor(unsigned maxWaitingCount, unsigned locationNumber, unsigned storagesCount, StorageManager* manager):
                         StorageProcessor(maxWaitingCount, manager), locationNumber_(locationNumber), storagesCount_(storagesCount) {
  abntlog_ = Logger::getInstance("pvss.abnt");
}

void AbonentStorageProcessor::initElementStorage(const AbonentStorageConfig& cfg, unsigned index) {
  char pathSuffix[4];
  sprintf(pathSuffix, "%03d", index);			
  string path = string(cfg.locationPath[locationNumber_] + "/") + pathSuffix;
  ElementStorage elStorage(index);
  elStorage.glossary = new Glossary();
  initGlossary(path, elStorage.glossary);

  std::auto_ptr< DiskIndexStorage > dis(new DiskIndexStorage( cfg.dbName, path, cfg.indexGrowth,
                                                              false, smsc::logger::Logger::getInstance("pvss.idx")));
  smsc_log_debug(logger_, "data index storage %d is created", index);
  std::auto_ptr< DiskDataStorage::storage_type > bs
        (new DiskDataStorage::storage_type(elStorage.glossary,
                                           smsc::logger::Logger::getInstance("pvss.bhdisk")));
  int ret = -1;
  const string fn(cfg.dbName + "-data");

  ret = bs->Open(fn, path);

  if (ret == BlocksHSStorage< AbntAddr, Profile >::DESCR_FILE_OPEN_FAILED) {
    if (bs->Create(fn, path, cfg.fileSize, cfg.blockSize) < 0) {
      throw Exception("can't create data disk storage: %s", path.c_str());
    }
    ret = 0;
  }
  if (ret < 0) {
    throw Exception("can't open data disk storage: %s", path.c_str());
  }
  std::auto_ptr< DiskDataStorage > dds
        (new DiskDataStorage(bs.release(),
                             smsc::logger::Logger::getInstance("pvss.disk.d")));
  smsc_log_debug(logger_, "data disk storage %d is created", index);

  std::auto_ptr< DiskStorage > ds(new DiskStorage(dis.release(), dds.release()));
  smsc_log_debug(logger_, "disk storage is assembled");

  std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance("cache"), cfg.cacheSize));
  smsc_log_debug(logger_, "memory storage is created");

  elStorage.storage = new AbonentStorage(ms.release(), ds.release());

  elementStorages_.Insert(elStorage.index, elStorage);
  smsc_log_debug(logger_, "abonent storage is assembled");
}

void AbonentStorageProcessor::process(PersPacket* packet) {
  smsc_log_debug(logger_, "%p: %p createProfile=%d", this, packet, packet->createProfile);
  unsigned elstorageIndex = static_cast<unsigned>(packet->address.getNumber() % storagesCount_);
  smsc_log_debug(logger_, "%p: %p process profile key='%s' in location: %d, storage: %d",
                  this, packet, packet->address.toString().c_str(), locationNumber_, elstorageIndex);
  ElementStorage *elstorage = elementStorages_.GetPtr(elstorageIndex);
  if (!elstorage) {
    smsc_log_warn(logger_, "%p: %p element storage %d not found in location %d", this, packet, elstorageIndex, locationNumber_);
    packet->createResponse(scag::pers::util::RESPONSE_ERROR);
    return;
  }
  Profile *pf = elstorage->storage->get(packet->address, packet->createProfile);
  if (!pf) {
    packet->createResponse(scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND);
    return;
  }
  packet->execCommand(pf);
  if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile %s", this, packet, pf->getKey().c_str());
    elstorage->storage->flush(packet->address);
    packet->flushLogs(abntlog_);
  } else if (packet->rollback) {
    smsc_log_debug(logger_, "%p: %p rollback profile %s changes", this, packet, pf->getKey().c_str());
    elstorage->storage->backup2Profile(packet->address, elstorage->glossary);
  }
}

void AbonentStorageProcessor::shutdownStorages() {
  if (elementStorages_.Count() <= 0) {
    return;
  }
  IntHash<ElementStorage>::Iterator it(elementStorages_);
  ElementStorage elstorage;
  int key = 0;
  while (it.Next(key, elstorage)) {
    smsc_log_debug(logger_, "delete %d element storage index=%d", key, elstorage.index);
    if (elstorage.storage) {
      delete elstorage.storage;
    }
    if (elstorage.glossary) {
      delete elstorage.glossary;
    }
  }
  elementStorages_.Empty();
}

AbonentStorageProcessor::~AbonentStorageProcessor() {
  shutdownStorages();
  smsc_log_debug(logger_, "storage processor %d deleted", locationNumber_);
}

void InfrastructStorageProcessor::process(PersPacket* packet) {
  Logger *dblog = 0;
  InfrastructStorage* storage = 0;
  switch (packet->profileType) {
  case scag::pers::util::PT_OPERATOR: dblog = olog_; storage = operator_; break;
  case scag::pers::util::PT_PROVIDER: dblog = plog_; storage = provider_; break;
  case scag::pers::util::PT_SERVICE:  dblog = slog_; storage = service_; break;
  default: 
    smsc_log_error(logger_, "cx %p unknown profile type %d", packet, packet->profileType);
    packet->createResponse(scag::pers::util::RESPONSE_BAD_REQUEST);
    return;
  }
  IntProfileKey key(packet->intKey);
  Profile *pf = storage->get(key, packet->createProfile);
  if (!pf) {
    packet->createResponse(scag::pers::util::RESPONSE_PROPERTY_NOT_FOUND);
    return;
  }
  packet->execCommand(pf);
  if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile %s", this, packet, pf->getKey().c_str());
    storage->flush(key);
    packet->flushLogs(dblog);
  } else if (packet->rollback){
    smsc_log_debug(logger_, "%p: %p rollback profile %s changes", this, packet, pf->getKey().c_str());
    storage->backup2Profile(key, &glossary_);
  }
}

void InfrastructStorageProcessor::init(const InfrastructStorageConfig& cfg) {
  InfrastructStorageConfig locCfg(cfg);
  initGlossary(locCfg.dbPath, &glossary_);
  locCfg.dbName = "provider";
  provider_ = initStorage(locCfg);
  smsc_log_debug(logger_, "provider storage is created");
  locCfg.dbName = "service";
  service_ = initStorage(locCfg);
  smsc_log_debug(logger_, "service storage is created");
  locCfg.dbName = "operator";
  operator_ = initStorage(locCfg);
  smsc_log_debug(logger_, "operator storage is created");
  plog_ = Logger::getInstance("pvss.prov");
  slog_ = Logger::getInstance("pvss.serv");
  olog_ = Logger::getInstance("pvss.oper");
}

InfrastructStorageProcessor::InfrastructStorage* InfrastructStorageProcessor::initStorage(const InfrastructStorageConfig& cfg) {
  const string fn(cfg.dbPath + "/" + cfg.dbName + ".bin");
  std::auto_ptr< DiskDataStorage::storage_type > pf(new PageFile);
  try {
    pf->Open(fn);
  } catch (...) {
    pf->Create(fn, 256, cfg.recordCount);
  }

  std::auto_ptr< DiskDataStorage > data(new DiskDataStorage(pf.release(),  Logger::getInstance("disk"), &glossary_));
  smsc_log_debug(logger_, "%s data storage is created", cfg.dbName.c_str());

  std::auto_ptr< DiskIndexStorage > index(new DiskIndexStorage(cfg.dbName, cfg.dbPath, cfg.recordCount));
  smsc_log_debug(logger_, "%s index storage is created", cfg.dbName.c_str());

  std::auto_ptr< DiskStorage > ds (new DiskStorage(index.release(), data.release()));
  smsc_log_debug(logger_, "%s indexed storage is created", cfg.dbName.c_str());

  std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance("cache"), cfg.cacheSize));
  smsc_log_debug(logger_, "%s memory storage is created", cfg.dbName.c_str());

  return new InfrastructStorage(ms.release(), ds.release());
}

void InfrastructStorageProcessor::shutdownStorages() {
  if (provider_) {
    delete provider_;
    provider_ = 0;
  }
  if (operator_) {
    delete operator_;
    operator_ = 0;
  }
  if (service_) {
    delete service_;
    service_ = 0;
  }
}

InfrastructStorageProcessor::~InfrastructStorageProcessor() {
  shutdownStorages();
  smsc_log_debug(logger_, "infrastruct storage processor deleted");
}

AbonentStorageConfig::AbonentStorageConfig() {
  dbName = DEF_STORAGE_NAME;
  dbPath = DEF_STORAGE_PATH;
  indexGrowth = DEF_INDEX_GROWTH;
  blockSize = DEF_BLOCK_SIZE;
  fileSize = DEF_FILE_SIZE;
  cacheSize = DEF_CACHE_SIZE;
}

AbonentStorageConfig::AbonentStorageConfig(ConfigView& cfg, const char* storageType,
                                            Logger* logger) {
  try {
    dbName = cfg.getString("storageName"); 
  } catch (...) {
    dbName = DEF_STORAGE_NAME;
    smsc_log_warn(logger, "Parameter <PVSS.%s.storageName> missed. Defaul value is '%s'",
                   storageType, DEF_STORAGE_NAME);
  }
  try {
    indexGrowth = cfg.getInt("indexGrowth"); 
  } catch (...) {
    indexGrowth = DEF_INDEX_GROWTH;
    smsc_log_warn(logger, "Parameter <PVSS.%s.indexGrowth> missed. Defaul value is %d",
                   storageType, DEF_INDEX_GROWTH);
  }
  try {
    blockSize = cfg.getInt("dataBlockSize"); 
  } catch (...) {
    blockSize = DEF_BLOCK_SIZE;
    smsc_log_warn(logger, "Parameter <PVSS.%s.blockSize> missed. Defaul value is %d",
                   storageType, DEF_BLOCK_SIZE);
  }
  try {
    fileSize = cfg.getInt("blocksInFile"); 
  } catch (...) {
    fileSize = DEF_FILE_SIZE;
    smsc_log_warn(logger, "Parameter <PVSS.%s.blocksInFile> missed. Defaul value is %d",
                   storageType, DEF_FILE_SIZE);
  }
  try {
    cacheSize = cfg.getInt("cacheSize"); 
  } catch (...) {
    cacheSize = DEF_CACHE_SIZE;
    smsc_log_warn(logger, "Parameter <PVSS.%s.cacheSize> missed. Defaul value is %d",
                   storageType, DEF_CACHE_SIZE);
  }
}

InfrastructStorageConfig::InfrastructStorageConfig() {
  dbPath = DEF_STORAGE_PATH;
  cacheSize = DEF_CACHE_SIZE;
  recordCount = DEF_RECORD_COUNT;
}

InfrastructStorageConfig::InfrastructStorageConfig(ConfigView& cfg, const char* storageType, Logger* logger) {
  try {
    dbPath = cfg.getString("storagePath"); 
  } catch (...) {
    dbPath = "./storage";
    smsc_log_warn(logger, "Parameter <PVSS.%s.storageDir> missed. Defaul value is '/'",
                   storageType);
  }
  try {
    recordCount = cfg.getInt("initRecordCount"); 
  } catch (...) {
    recordCount = DEF_BLOCK_SIZE;
    smsc_log_warn(logger, "Parameter <PVSS.%s.initRecordCount> missed. Defaul value is %d",
                   storageType, DEF_RECORD_COUNT);
  }
  try {
    cacheSize = cfg.getInt("cacheSize"); 
  } catch (...) {
    cacheSize = DEF_CACHE_SIZE;
    smsc_log_warn(logger, "Parameter <PVSS.%s.cacheSize> missed. Defaul value is %d",
                   storageType, DEF_CACHE_SIZE);
  }
}

}//mtpers
}//scag
