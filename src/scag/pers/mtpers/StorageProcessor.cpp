#include "StorageProcessor.h"
#include "core/synchronization/MutexGuard.hpp"
#include "core/buffers/File.hpp"
#include <exception>



namespace scag { namespace mtpers {

using smsc::core::synchronization::MutexGuard;
using smsc::core::buffers::File;
using smsc::core::buffers::FileException;

const char* DEF_STORAGE_NAME    = "abonent";
const char* DEF_LOCAL_PATH      = "infrastruct";
const char* DEF_STORAGE_PATH    = "./storage";
const char* DEF_GLOSSARY_NAME   = "/glossary";
const uint32_t DEF_INDEX_GROWTH = 1000;
const uint32_t DEF_BLOCK_SIZE   = 2048;
const uint32_t DEF_FILE_SIZE    = 50000;
const uint32_t DEF_CACHE_SIZE   = 1000;
const uint32_t DEF_RECORD_COUNT = 1000;

StorageProcessor::StorageProcessor(unsigned maxWaitingCount): maxWaitingCount_(maxWaitingCount)
{
  logger_ = Logger::getInstance("storeproc");
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
    for (vector<PersPacket*>::iterator i = process.begin(); i < process.end(); ++i) {
      smsc_log_debug(logger_, "%p: process context %p", this, *i);
      try {

        this->process(*i);

        smsc_log_debug(logger_, "%p: %p processing complite", this, *i);      

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
    }
    process.clear();
  }
  waitingProcess_.clear();
  return 0;
}

void StorageProcessor::stop() {
  isStopping = true;

  MutexGuard g(processMonitor_);
  processMonitor_.notify();
}

const char * StorageProcessor::taskName() {
  return "StorageProcessor";
}

bool StorageProcessor::addPacket(PersPacket* packet) {
  MutexGuard g(processMonitor_);
  if (waitingProcess_.size() >= maxWaitingCount_) {
    smsc_log_debug(logger_, "%p: %p waiting process queue limit %d", this, packet, maxWaitingCount_);
    return false;
  }
  waitingProcess_.push_back(packet);
  processMonitor_.notify();  
  return true;
}

AbonentStorageProcessor::AbonentStorageProcessor(unsigned maxWaitingCount, unsigned locationNumber, unsigned storagesCount):
                         StorageProcessor(maxWaitingCount), locationNumber_(locationNumber), storagesCount_(storagesCount) {  
  abntlog_ = Logger::getInstance("pvss.abnt");
}

void AbonentStorageProcessor::initElementStorage(const AbonentStorageConfig& cfg, unsigned index) {
  char pathSuffix[4];
  sprintf(pathSuffix, "%03d", index);			
  string path = string(cfg.locationPath[locationNumber_] + "/") + pathSuffix;
  ElementStorage elStorage(index);
  elStorage.glossary = new Glossary();
  initGlossary(path, elStorage.glossary);

  std::auto_ptr< DiskIndexStorage > dis(new DiskIndexStorage(cfg.dbName, path, cfg.indexGrowth));
  smsc_log_debug(logger_, "data index storage %d is created", index);
  std::auto_ptr< DiskDataStorage::storage_type > bs(new DiskDataStorage::storage_type(elStorage.glossary));
  int ret = -1;
  const string fn(cfg.dbName + "-data");
  try {
    ret = bs->Open(fn, path);
  } catch (...) {
    ret = -1;
  }
  if (ret < 0) {
    bs->Create(fn, path, cfg.fileSize, cfg.blockSize);
  }
  std::auto_ptr< DiskDataStorage > dds(new DiskDataStorage(bs.release()));
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

AbonentStorageProcessor::~AbonentStorageProcessor() {
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

InfrastructStorageProcessor::~InfrastructStorageProcessor() {
  if (provider_) {
    delete provider_;
  }
  if (operator_) {
    delete operator_;
  }
  if (service_) {
    delete service_;
  }
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
    smsc_log_warn(logger, "Parameter <MTPers.%s.storageName> missed. Defaul value is '%s'",
                   storageType, DEF_STORAGE_NAME);
  }
  try {
    indexGrowth = cfg.getInt("indexGrowth"); 
  } catch (...) {
    indexGrowth = DEF_INDEX_GROWTH;
    smsc_log_warn(logger, "Parameter <MTPers.%s.indexGrowth> missed. Defaul value is %d",
                   storageType, DEF_INDEX_GROWTH);
  }
  try {
    blockSize = cfg.getInt("dataBlockSize"); 
  } catch (...) {
    blockSize = DEF_BLOCK_SIZE;
    smsc_log_warn(logger, "Parameter <MTPers.%s.blockSize> missed. Defaul value is %d",
                   storageType, DEF_BLOCK_SIZE);
  }
  try {
    fileSize = cfg.getInt("blocksInFile"); 
  } catch (...) {
    fileSize = DEF_FILE_SIZE;
    smsc_log_warn(logger, "Parameter <MTPers.%s.blocksInFile> missed. Defaul value is %d",
                   storageType, DEF_FILE_SIZE);
  }
  try {
    cacheSize = cfg.getInt("cacheSize"); 
  } catch (...) {
    cacheSize = DEF_CACHE_SIZE;
    smsc_log_warn(logger, "Parameter <MTPers.%s.cacheSize> missed. Defaul value is %d",
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
    smsc_log_warn(logger, "Parameter <MTPers.%s.storageDir> missed. Defaul value is '/'",
                   storageType);
  }
  try {
    recordCount = cfg.getInt("initRecordCount"); 
  } catch (...) {
    recordCount = DEF_BLOCK_SIZE;
    smsc_log_warn(logger, "Parameter <MTPers.%s.initRecordCount> missed. Defaul value is %d",
                   storageType, DEF_RECORD_COUNT);
  }
  try {
    cacheSize = cfg.getInt("cacheSize"); 
  } catch (...) {
    cacheSize = DEF_CACHE_SIZE;
    smsc_log_warn(logger, "Parameter <MTPers.%s.cacheSize> missed. Defaul value is %d",
                   storageType, DEF_CACHE_SIZE);
  }
}

}//mtpers
}//scag
