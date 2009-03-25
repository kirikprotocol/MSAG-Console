#include "PvssLogic.h"

#include <exception>
#include "core/buffers/File.hpp"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"
#include "scag/pvss/api/packets/Response.h"

namespace scag2 {
namespace pvss  {

const char* DEF_GLOSSARY_NAME   = "/glossary";

using smsc::core::buffers::File;
using smsc::core::buffers::FileException;

void PvssLogic::initGlossary(const string& path, Glossary* glossary) {
  if (!File::Exists(path.c_str())) {
    smsc_log_debug(logger_, "create storage dir '%s'", path.c_str());
    File::MkDir(path.c_str());
  }
  string fn = path + DEF_GLOSSARY_NAME;
  if ( GlossaryBase::SUCCESS != glossary->Open(fn) ) {
    throw std::runtime_error("StorageProcessor: glossary open error");
  }
}

Response* PvssLogic::process(Request& request) throw(PvssException) {
  try {
    return processProfileRequest(static_cast<AbstractProfileRequest&>(request));
  } catch (const PvapException& e) {
    smsc_log_warn(logger_, "%p: %p processing error: PvapException", this, &request);
    //TODO: some rollback actions
    throw;
  } catch (const std::runtime_error &e) {
    smsc_log_warn(logger_, "%p: %p processing error: std::runtime_error: %s", this, &request, e.what());
  } catch (const FileException &e) {
    smsc_log_warn(logger_, "%p: %p processing error: FileException: %s", this, &request, e.what());
  } catch (const std::exception &e) {
    smsc_log_warn(logger_, "%p: %p processing error: std::exception: %s", this, &request, e.what());
  } catch (...) {
    smsc_log_warn(logger_, "%p: %p processing error: unknown exception", this, &request);      
  }
  return 0;
}

Response* AbonentLogic::processProfileRequest(AbstractProfileRequest& request) {

  AbstractProfileRequest &profileRequest = static_cast< AbstractProfileRequest& >(request);
  const ProfileKey &profileKey = profileRequest.getProfileKey();
  unsigned elstorageIndex = static_cast<unsigned>(profileKey.getAddress().getNumber() % storagesCount_);
  smsc_log_debug(logger_, "%p: %p process profile key='%s' in location: %d, storage: %d",
                  this, &request, profileKey.getAddress().toString().c_str(), locationNumber_, elstorageIndex);

  ElementStorage *elstorage = elementStorages_.GetPtr(elstorageIndex);
  if (!elstorage) {
    smsc_log_warn(logger_, "%p: %p element storage %d not found in location %d", this, &request, elstorageIndex, locationNumber_);
    return 0;
  }
  bool createProfile = true; //TODO: set false for GET and DEL requests
  Profile *pf = elstorage->storage->get(profileKey.getAddress(), createProfile);
  pf->setChanged(false);
  commandProcessor_.setProfile(pf);
  profileRequest.getCommand()->visit(commandProcessor_);

  if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile %s", this, &request, pf->getKey().c_str());
    elstorage->storage->flush(profileKey.getAddress());
    commandProcessor_.flushLogs(abntlog_);
  } else if (commandProcessor_.rollback()) {
    smsc_log_debug(logger_, "%p: %p rollback profile %s changes", this, &request, pf->getKey().c_str());
    elstorage->storage->backup2Profile(profileKey.getAddress(), elstorage->glossary);
  }
  return commandProcessor_.getResponse();
}

Response* InfrastructLogic::processProfileRequest(AbstractProfileRequest& request) {
  Logger *dblog = 0;
  InfrastructStorage* storage = 0;
  uint32_t key = 0;
  AbstractProfileRequest &profileRequest = static_cast<AbstractProfileRequest &>(request);
  const ProfileKey& profileKey = profileRequest.getProfileKey();
  if (profileKey.hasOperatorKey()) {
    dblog = olog_;
    storage = operator_;
    key = profileKey.getOperatorKey();
  }
  if (profileKey.hasProviderKey()) {
    dblog = plog_;
    storage = provider_;
    key = profileKey.getProviderKey();
  }
  if (profileKey.hasServiceKey()) {
    dblog = slog_;
    storage = service_;
    key = profileKey.getServiceKey();
  }

  IntProfileKey intKey(key);
  bool createProfile = true; //TODO set false for GET and DEL requests
  Profile *pf = storage->get(intKey, createProfile);
  pf->setChanged(false);
  commandProcessor_.setProfile(pf);
  profileRequest.getCommand()->visit(commandProcessor_);

  if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile %s", this, &request, pf->getKey().c_str());
    storage->flush(intKey);
    commandProcessor_.flushLogs(dblog);
  } else if (commandProcessor_.rollback()){
    smsc_log_debug(logger_, "%p: %p rollback profile %s changes", this, &request, pf->getKey().c_str());
    storage->backup2Profile(intKey, &glossary_);
  }
  return commandProcessor_.getResponse();
}

void AbonentLogic::initElementStorage(unsigned index) {
  char pathSuffix[4];
  sprintf(pathSuffix, "%03d", index);			
  string path = string(config_.locationPath[locationNumber_] + "/") + pathSuffix;
  ElementStorage elStorage(index);
  elStorage.glossary = new Glossary();
  initGlossary(path, elStorage.glossary);

  std::auto_ptr< DiskIndexStorage > dis(new DiskIndexStorage( config_.dbName, path, config_.indexGrowth,
                                                              false, smsc::logger::Logger::getInstance("pvss.idx")));
  smsc_log_debug(logger_, "data index storage %d is created", index);
  std::auto_ptr< DiskDataStorage::storage_type > bs
        (new DiskDataStorage::storage_type(elStorage.glossary,
                                           smsc::logger::Logger::getInstance("pvss.bhdisk")));
  int ret = -1;
  const string fn(config_.dbName + "-data");

  ret = bs->Open(fn, path);

  if (ret == BlocksHSStorage< AbntAddr, Profile >::DESCR_FILE_OPEN_FAILED) {
    if (bs->Create(fn, path, config_.fileSize, config_.blockSize) < 0) {
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

  std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance("cache"), config_.cacheSize));
  smsc_log_debug(logger_, "memory storage is created");

  elStorage.storage = new AbonentStorage(ms.release(), ds.release());

  elementStorages_.Insert(elStorage.index, elStorage);
  smsc_log_debug(logger_, "abonent storage is assembled");
}

void AbonentLogic::shutdownStorages() {
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

AbonentLogic::~AbonentLogic() {
  shutdownStorages();
  smsc_log_debug(logger_, "storage processor %d deleted", locationNumber_);
}


void AbonentLogic::init() throw (smsc::util::Exception)
{
    smsc_log_debug(logger_," init abonent location #%u", locationNumber_ );
    for ( unsigned i = 0; i < storagesCount_; ++i ) {
        if ( StorageNumbering::instance().node(i) == nodeNumber_ ) {
            initElementStorage(i);
        }
    }
}


void InfrastructLogic::init() throw (smsc::util::Exception) {
  InfrastructStorageConfig locCfg(config_);
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
}

InfrastructLogic::InfrastructStorage* InfrastructLogic::initStorage(const InfrastructStorageConfig& cfg) {
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

void InfrastructLogic::shutdownStorages() {
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

InfrastructLogic::~InfrastructLogic() {
  shutdownStorages();
  smsc_log_debug(logger_, "infrastruct storage processor deleted");
}

const char* DEF_STORAGE_NAME    = "abonent";
const char* DEF_LOCAL_PATH      = "infrastruct";
const char* DEF_STORAGE_PATH    = "./storage";
const uint32_t DEF_INDEX_GROWTH = 1000;
const uint32_t DEF_BLOCK_SIZE   = 2048;
const uint32_t DEF_FILE_SIZE    = 50000;
const uint32_t DEF_CACHE_SIZE   = 1000;
const uint32_t DEF_RECORD_COUNT = 1000;

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

}//pvss
}//scag2
