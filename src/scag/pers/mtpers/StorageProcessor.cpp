#include "StorageProcessor.h"
#include "core/synchronization/MutexGuard.hpp"
#include <exception>



namespace scag { namespace mtpers {

using smsc::core::synchronization::MutexGuard;

const char* DEF_STORAGE_NAME    = "abonent";
const char* DEF_LOCAL_PATH      = "infrastruct";
const char* DEF_STORAGE_PATH    = "./storage";
const char* DEF_GLOSSARY_NAME   = "/glossary";
const uint32_t DEF_INDEX_GROWTH = 1000;
const uint32_t DEF_BLOCK_SIZE   = 2048;
const uint32_t DEF_FILE_SIZE    = 50000;
const uint32_t DEF_CACHE_SIZE   = 1000;
const uint32_t DEF_RECORD_COUNT = 1000;

StorageProcessor::StorageProcessor(uint16_t maxWaitingCount):
                                   maxWaitingCount_(maxWaitingCount)
{
  logger_ = Logger::getInstance("storeproc");
}

void StorageProcessor::initGlossary(const string& dbPath) {
  string fn = dbPath + DEF_GLOSSARY_NAME;
  if ( GlossaryBase::SUCCESS != glossary_.Open(fn) ) {
    throw std::runtime_error("StorageProcessor: glossary open error");
  }
}

StorageProcessor::~StorageProcessor() {
  glossary_.Close();
}


int StorageProcessor::Execute() {
  vector<ConnectionContext*> process;
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
    for (vector<ConnectionContext*>::iterator i = process.begin(); i < process.end(); ++i) {
      smsc_log_debug(logger_, "%p: process context %p", this, *i);
      try {
        this->process(*i);
        smsc_log_debug(logger_, "%p: %p processing complite", this, *i);      
      } catch (const SerialBufferOutOfBounds &e) {
        smsc_log_warn(logger_, "%p: %p processing error: SerialBufferOutOfBounds", this, *i);
        (*i)->createFakeResponse(scag::pers::RESPONSE_ERROR);
      } catch (const std::runtime_error &e) {
        smsc_log_warn(logger_, "%p: %p processing error: std::runtime_error: %s", this, *i, e.what());
        (*i)->createFakeResponse(scag::pers::RESPONSE_ERROR);
      } catch (const FileException &e) {
        smsc_log_warn(logger_, "%p: %p processing error: FileException: %s", this, *i, e.what());
        (*i)->createFakeResponse(scag::pers::RESPONSE_ERROR);
      } catch (const std::exception &e) {
        smsc_log_warn(logger_, "%p: %p processing error: std::exception: %s", this, *i, e.what());
        (*i)->createFakeResponse(scag::pers::RESPONSE_ERROR);
      } catch (...) {
        smsc_log_warn(logger_, "%p: %p processing error: unknown exception", this, *i);      
        (*i)->createFakeResponse(scag::pers::RESPONSE_ERROR);
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

bool StorageProcessor::addContext(ConnectionContext* cx) {
  MutexGuard g(processMonitor_);
  if (waitingProcess_.size() >= maxWaitingCount_) {
    return false;
  }
  waitingProcess_.push_back(cx);
  processMonitor_.notify();  
  return true;
}

AbonentStorageProcessor::AbonentStorageProcessor(uint16_t maxWaitingCount, uint16_t storageIndex):
                         StorageProcessor(maxWaitingCount), storageIndex_(storageIndex) {   
}

void AbonentStorageProcessor::init(const AbonentStorageConfig& cfg) {
  char pathSuffix[4];
  //sprintf(pathSuffix, "/%02d", storageIndex_ + 1);			
  string path = cfg.dbPath + "/" + cfg.localPath[storageIndex_];
  initGlossary(path);
  std::auto_ptr< DiskIndexStorage > dis(new DiskIndexStorage(cfg.dbName, path, cfg.indexGrowth));
  smsc_log_debug(logger_, "data index %d storage is created", storageIndex_);

  std::auto_ptr< DiskDataStorage::storage_type > bs(new DiskDataStorage::storage_type(&glossary_));
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
  smsc_log_debug(logger_, "data disk storage %d is created", storageIndex_);


  std::auto_ptr< DiskStorage > ds(new DiskStorage(dis.release(), dds.release()));
  smsc_log_debug(logger_, "disk storage is assembled");

  std::auto_ptr< MemStorage > ms(new MemStorage(cfg.cacheSize));
  smsc_log_debug(logger_, "memory storage is created");

  storage_.reset( new AbonentStorage(ms.release(), ds.release()));
  smsc_log_debug(logger_, "abonent storage is assembled");
  abntlog_ = Logger::getInstance("pvss.abnt");
}

void AbonentStorageProcessor::process(ConnectionContext* cx) {
  smsc_log_debug(logger_, "%p: %p createProfile=%d", this, cx, cx->packet->createProfile);
  Profile *pf = storage_->get(cx->packet->address, cx->packet->createProfile);
  if (!pf) {
    cx->createFakeResponse(scag::pers::RESPONSE_PROPERTY_NOT_FOUND);
    return;
  }
  cx->packet->execCommand(pf, cx->outbuf, abntlog_, cx->packet->address.toString());
  if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile", this, cx);
    storage_->flush(cx->packet->address);
  }
}

void InfrastructStorageProcessor::process(ConnectionContext* cx) {
  Logger *dblog = 0;
  InfrastructStorage* storage = 0;
  switch (cx->packet->profileType) {
  case scag::pers::PT_OPERATOR: dblog = olog_; storage = operator_.get(); break;
  case scag::pers::PT_PROVIDER: dblog = plog_; storage = provider_.get(); break;
  case scag::pers::PT_SERVICE:  dblog = slog_; storage = service_.get(); break;
  default: 
    smsc_log_error(logger_, "cx %p unknown profile type %d", cx, cx->packet->profileType);
    cx->createFakeResponse(scag::pers::RESPONSE_BAD_REQUEST);
    return;
  }
  IntProfileKey key(cx->packet->intKey);
  Profile *pf = storage->get(key, cx->packet->createProfile);
  if (!pf) {
    cx->createFakeResponse(scag::pers::RESPONSE_PROPERTY_NOT_FOUND);
    return;
  }
  cx->packet->execCommand(pf, cx->outbuf, dblog, key.toString());
  if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile", this, cx);
    storage->flush(key);
  }
}

void InfrastructStorageProcessor::init(const InfrastructStorageConfig& cfg) {
  InfrastructStorageConfig locCfg(cfg);
  locCfg.dbPath += "/" + cfg.localPath;
  initGlossary(locCfg.dbPath);
  locCfg.dbName = "provider";
  initStorage(locCfg, provider_);
  locCfg.dbName = "service";
  initStorage(locCfg, service_);
  locCfg.dbName = "operator";
  initStorage(locCfg, operator_);
  plog_ = Logger::getInstance("pvss.prov");
  slog_ = Logger::getInstance("pvss.serv");
  olog_ = Logger::getInstance("pvss.oper");
}

void InfrastructStorageProcessor::initStorage(const InfrastructStorageConfig& cfg,
                                              std::auto_ptr<InfrastructStorage>& storage) {
  const string fn(cfg.dbPath + "/" + cfg.dbName + ".bin");
  std::auto_ptr< DiskDataStorage::storage_type > pf(new PageFile);
  try {
    pf->Open(fn);
  } catch (...) {
    pf->Create(fn, 256, cfg.recordCount);
  }

  std::auto_ptr< DiskDataStorage > data(new DiskDataStorage(pf.release(), &glossary_));
  smsc_log_debug(logger_, "%s data storage is created", cfg.dbName.c_str());

  std::auto_ptr< DiskIndexStorage > index(new DiskIndexStorage(cfg.dbName, cfg.dbPath, cfg.recordCount));
  smsc_log_debug(logger_, "%s index storage is created", cfg.dbName.c_str());

  std::auto_ptr< DiskStorage > ds (new DiskStorage(index.release(), data.release()));
  smsc_log_debug(logger_, "%s indexed storage is created", cfg.dbName.c_str());

  std::auto_ptr< MemStorage > ms(new MemStorage(cfg.cacheSize));
  smsc_log_debug(logger_, "%s memory storage is created", cfg.dbName.c_str());

  storage.reset(new InfrastructStorage(ms.release(), ds.release()));
  smsc_log_debug(logger_, "%s storage is created", cfg.dbName.c_str());
}

AbonentStorageConfig::AbonentStorageConfig(uint32_t snumber) {
  dbName = DEF_STORAGE_NAME;
  dbPath = DEF_STORAGE_PATH;
  indexGrowth = DEF_INDEX_GROWTH;
  blockSize = DEF_BLOCK_SIZE;
  fileSize = DEF_FILE_SIZE;
  cacheSize = DEF_CACHE_SIZE;
  storageNumber = snumber;
}

AbonentStorageConfig::AbonentStorageConfig(uint32_t snumber, ConfigView& cfg, const char* storageType,
                                            Logger* logger) {
  storageNumber = snumber;
  try {
    dbName = cfg.getString("storageName"); 
  } catch (...) {
    dbName = DEF_STORAGE_NAME;
    smsc_log_warn(logger, "Parameter <MTPers.%s.storageName> missed. Defaul value is '%s'",
                   storageType, DEF_STORAGE_NAME);
  }
  /*
  try {
    dbPath = cfg.getString("storagePath"); 
  } catch (...) {
    dbPath = DEF_STORAGE_PATH;
    smsc_log_warn(logger, "Parameter <MTPers.%s.storagePath> missed. Defaul value is '%s'",
                   storageType, DEF_STORAGE_PATH);
  }*/
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
  string storageDirPrefix = "storageDir_";
  char dirName[30];
  string name;
  for (int i = 0; i < storageNumber; ++i) {
    try {
      sprintf(dirName, "%s%02d", storageDirPrefix.c_str(), i + 1);
      name = cfg.getString(dirName);
    } catch (...) {
      sprintf(dirName, "%02d", i + 1);
      name = dirName;
      if (storageNumber == 1) {
        name = dbName;
      }
      smsc_log_warn(logger, "Parameter <MTPers.%s.%s> missed. Defaul value is '%s'",
                     storageType, dirName, name.c_str());
    }
    localPath.push_back(name);
  }
}

InfrastructStorageConfig::InfrastructStorageConfig() {
  dbPath = DEF_STORAGE_PATH;
  cacheSize = DEF_CACHE_SIZE;
  recordCount = DEF_RECORD_COUNT;
}

InfrastructStorageConfig::InfrastructStorageConfig(ConfigView& cfg, const char* storageType, Logger* logger) {
  try {
    localPath = cfg.getString("storageDir"); 
  } catch (...) {
    localPath = "";
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
