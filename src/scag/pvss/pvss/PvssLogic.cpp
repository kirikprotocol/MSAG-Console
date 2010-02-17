#include "PvssLogic.h"
#include "PvssDispatcher.h"

#include <exception>
#include "core/buffers/File.hpp"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/ProfileResponse.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"


namespace {

void doRename( const std::string& t,
               const std::string& n,
               const std::string& o,
               const char* ext = "" )
{
    std::string f = n + ext;
    if ( smsc::core::buffers::File::Exists(f.c_str()) ) {
        rename(f.c_str(),(o+ext).c_str());
    }
    std::string g = t + ext;
    if ( smsc::core::buffers::File::Exists(g.c_str()) ) {
        rename(g.c_str(),f.c_str());
    }
}


class CreateProfileVisitor : public scag2::pvss::ProfileCommandVisitor {
public:
    virtual bool visitBatchCommand(scag2::pvss::BatchCommand &cmd) /* throw(scag2::pvss::PvapException) */ {
        const std::vector< scag2::pvss::BatchRequestComponent* >& content = cmd.getBatchContent();
        for ( std::vector< scag2::pvss::BatchRequestComponent* >::const_iterator i = content.begin();
              i != content.end(); ++i ) {
            if ( (*i)->visit(*this) ) return true;
        }
        return false;
    }
    virtual bool visitDelCommand(scag2::pvss::DelCommand &cmd) /* throw(scag2::pvss::PvapException) */  {
      return false;
    }
    virtual bool visitGetCommand(scag2::pvss::GetCommand &cmd) /* throw(scag2::pvss::PvapException) */  {
      return false;
    }
    virtual bool visitIncCommand(scag2::pvss::IncCommand &cmd) /* throw(scag2::pvss::PvapException) */  {
      return true;
    }
    virtual bool visitIncModCommand(scag2::pvss::IncModCommand &cmd) /* throw(scag2::pvss::PvapException) */  {
      return true;
    }
    virtual bool visitSetCommand(scag2::pvss::SetCommand &cmd) /* throw(scag2::pvss::PvapException) */  {
      return true;
    }
  };
  CreateProfileVisitor createProfileVisitor;
}

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

Response* PvssLogic::process(Request& request) /* throw(PvssException) */  {
  try {
      ProfileRequest& preq(static_cast<ProfileRequest&>(request));
      profileBackup_.cleanup();
      commandProcessor_.resetProfile();
      commandProcessor_.setBackup(&profileBackup_);
      Response* resp = processProfileRequest(preq);
      if ( preq.hasTiming() ) {
          const Profile* pf = commandProcessor_.getProfile();
          char buf[100];
          snprintf(buf,sizeof(buf)," [%s props=%u]",
                   preq.getProfileKey().toString().c_str(),
                   unsigned(pf ? pf->GetCount() : 0));
          preq.timingComment(buf);
      }
      return resp;
  } catch (const PvapException& e) {
    smsc_log_warn(logger_, "%p: %p processing error: PvapException", this, &request);
    //TODO: some rollback actions
    throw;
  } catch (const std::runtime_error &e) {
    smsc_log_warn(logger_, "%p: %p processing error: std::runtime_error: %s", this, &request, e.what());
  } catch (const FileException &e) {
      smsc_log_warn(logger_, "%p: %p processing error: FileException: %s", this, &request, e.what());
      throw;
  } catch (const std::exception &e) {
    smsc_log_warn(logger_, "%p: %p processing error: std::exception: %s", this, &request, e.what());
  } catch (...) {
    smsc_log_warn(logger_, "%p: %p processing error: unknown exception", this, &request);      
  }
  return 0;
}


// === abonent logic
AbonentLogic::~AbonentLogic() {
    shutdownStorages();
    smsc_log_debug(logger_, "storage processor %d deleted", locationNumber_);
}

PvssLogic::LogicInitTask* AbonentLogic::startInit( bool checkAtStart )
{
    LogicInitTask* task = new LogicInitTask(this,checkAtStart);
    dataFileManager_.startTask(task,false);
    return task;
}

PvssLogic::LogicRebuildIndexTask* AbonentLogic::startRebuildIndex(unsigned maxSpeed)
{
    LogicRebuildIndexTask* task = new LogicRebuildIndexTask(this,maxSpeed);
    dataFileManager_.startTask(task,false);
    return task;
}

void AbonentLogic::dumpStorage( int i )
{
    smsc_log_debug(logger_,"dump i=%u storages=%u node(i)=%u disp.node=%u disp.loc=%u loc=%u",
                  unsigned(i),
                  unsigned(dispatcher_.getStoragesCount()),
                  unsigned(util::storage::StorageNumbering::instance().node(i)),
                  unsigned(dispatcher_.getNodeNumber()),
                  unsigned(dispatcher_.getLocationNumber(i)),
                  unsigned(locationNumber_));
    if ( i >= 0 &&
         i < dispatcher_.getStoragesCount() &&
         util::storage::StorageNumbering::instance().node(i) == dispatcher_.getNodeNumber() &&
         dispatcher_.getLocationNumber(i) == locationNumber_ ) {
        smsc_log_info(logger_,"INITING STORAGE %u",i);
        initElementStorage( i, false, true );
        smsc_log_info(logger_,"DUMPING STORAGE %u",i);
        AbonentStorage* storage = elementStorages_.Get(i)->storage;
        AbntAddr key;
        Profile profile;
        typedef DiskStorage::value_type value_type;
        value_type value( &profile, new value_type::backup_type );
        for ( DiskStorage::iterator_type iter = storage->dataBegin();
              iter.next( key, value ); ) 
        {
            smsc_log_debug(logger_,"key: %s, val:%p", key.toString().c_str(), value.value );
            // dumping
            if ( value.value ) {
                smsc_log_info(logger_,"%s: %s",key.toString().c_str(),value.value->toString().c_str());
            }
        }
        // value.dealloc();
    }
}


void AbonentLogic::shutdownStorages() {
  if (elementStorages_.Count() <= 0) {
    return;
  }
  IntHash<ElementStorage*>::Iterator it(elementStorages_);
  ElementStorage* elstorage;
  int key = 0;
  while (it.Next(key, elstorage)) {
    smsc_log_debug(logger_, "delete %d element storage index=%d", key, elstorage->index);
    if (elstorage->storage) {
      delete elstorage->storage;
    }
    if (elstorage->glossary) {
      delete elstorage->glossary;
    }
    delete elstorage;
  }
  elementStorages_.Empty();
}


unsigned long AbonentLogic::reportStatistics() const
{
    IntHash<ElementStorage*>::Iterator it(elementStorages_);
    ElementStorage* elstorage;
    int key = 0;
    unsigned long total = 0;
    while (it.Next(key, elstorage)) {
        if (elstorage->storage) {
            MutexGuard mg(elstorage->mutex);
            total += elstorage->storage->filledDataSize();
        }
    }
    return total;
}


void AbonentLogic::init( bool checkAtStart ) /* throw (smsc::util::Exception) */
{
    smsc_log_debug(logger_," init abonent location #%u", locationNumber_ );
    unsigned long total = 0;
    for ( unsigned i = 0; i < dispatcher_.getStoragesCount(); ++i ) {
        if ( util::storage::StorageNumbering::instance().node(i) == dispatcher_.getNodeNumber() ) {
            if ( dispatcher_.getLocationNumber(i) == locationNumber_ ) {
                total += initElementStorage(i,checkAtStart);
            }
        }
    }
    smsc_log_info(logger_,"abonent logic on location #%u inited, total nodes: %lu", locationNumber_, total );
}


void AbonentLogic::rebuildIndex(unsigned maxSpeed)
{
    smsc_log_info(logger_,"rebuilding indices on abonent location #%u", locationNumber_ );
    unsigned long total = 0;
    for ( unsigned i = 0; i < dispatcher_.getStoragesCount(); ++i ) {
        if ( util::storage::StorageNumbering::instance().node(i) == dispatcher_.getNodeNumber() ) {
            if ( dispatcher_.getLocationNumber(i) == locationNumber_ ) {
                total += rebuildElementStorage(i,maxSpeed);
            }
        }
    }
    smsc_log_info(logger_,"abonent logic on location #%u indices rebuilt: %lu", locationNumber_, total );
}


unsigned long AbonentLogic::initElementStorage( unsigned index,
                                                bool checkAtStart,
                                                bool readonly ) /* throw (smsc::util::Exception) */ 
{
  char pathSuffix[4];
  snprintf(pathSuffix, sizeof(pathSuffix), "%03u", index);
  string path = string(config_.locations[locationNumber_].path + "/") + pathSuffix;
  std::auto_ptr<ElementStorage> elStorage(new ElementStorage(index));
  elStorage->glossary = new Glossary();
  initGlossary(path, elStorage->glossary);

    const std::string pathSuffixString(pathSuffix);
  std::auto_ptr< DiskIndexStorage > dis
        (new DiskIndexStorage( config_.dbName, path, config_.indexGrowth, false,
                               smsc::logger::Logger::getInstance(("pvssix."+pathSuffixString).c_str())));
  smsc_log_debug(logger_, "data index storage %d is created", index);
  if ( checkAtStart ) {
      dis->checkTree();
      smsc_log_debug(logger_, "data index storage %d is checked", index);
  }

#ifdef PVSSLOGIC_BHS2
    std::auto_ptr< DiskDataStorage::storage_type > bs
        ( new DiskDataStorage::storage_type
          ( dataFileManager_,
            smsc::logger::Logger::getInstance(("pvssbh."+pathSuffixString).c_str())));
    const std::string fn( config_.dbName + "-data" );
    int ret = -1;
    ret = bs->open(fn,path,readonly);
    if ( ret == DiskDataStorage::storage_type::JOURNAL_FILE_OPEN_FAILED ) {
        if ( readonly ) {
            throw smsc::util::Exception("failed to open journal file %s in readonly mode",path.c_str());
        } else if ( bs->create(fn,path, config_.fileSize, config_.blockSize ) < 0 ) {
            throw smsc::util::Exception("cannot create data disk storage: %s", path.c_str());
        }
        ret = 0;
    }
    if ( ret < 0 ) {
        throw smsc::util::Exception("cannot create data disk storage: %s", path.c_str());
    }
    std::auto_ptr< DiskDataStorage > dds
        ( new DiskDataStorage
          ( bs.release(),
            elStorage->glossary,
            smsc::logger::Logger::getInstance(("pvssdd."+pathSuffixString).c_str())));

#else

  if ( readonly ) {
      throw smsc::util::Exception("readonly mode is not supported for old storage");
  }

  std::auto_ptr< DiskDataStorage::storage_type > bs
        (new DiskDataStorage::storage_type
         (dataFileManager_, elStorage->glossary,
          smsc::logger::Logger::getInstance(("pvssbh."+pathSuffixString).c_str())));
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
                             smsc::logger::Logger::getInstance(("pvssdd."+pathSuffixString).c_str())));
#endif

  smsc_log_debug(logger_, "data disk storage %d is created", index);

  std::auto_ptr< DiskStorage > ds(new DiskStorage(dis.release(), dds.release()));
  smsc_log_debug(logger_, "disk storage is assembled");

  std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance(("pvssmc."+pathSuffixString).c_str()), config_.cacheSize));
  smsc_log_debug(logger_, "memory storage is created");

  elStorage->storage = new AbonentStorage(ms.release(), ds.release());
  elStorage->storage->setProfileLog(abntlog_);
  unsigned long filledNodes = elStorage->storage->filledDataSize();
  elementStorages_.Insert(index, elStorage.release());
  smsc_log_debug(logger_, "abonent storage is assembled");
  smsc_log_info( logger_, "storage #%u is inited, total number of good nodes: %lu", index, filledNodes );
  return filledNodes;
}


/*
#ifdef PVSSLOGIC_BHS2
class AbonentLogic::RBTreeIndexRescuer : public AbonentLogic::DiskDataStorage::storage_type::IndexRescuer
{
public:
    RBTreeIndexRescuer( DiskIndexStorage& istore,
                        DiskDataStorage::storage_type& dstore ) :
    istore_(istore), dstore_(dstore) {}
    virtual void recoverIndex( index_type idx, buffer_type& buffer )
    {
        // unpack buffer
        dstore_.unpackBuffer(buffer,0);
        Deserializer dsr(buffer);
        dsr.setVersion(dstore_.version());
        dsr.setrpos(dstore_.headerSize());
        AbntAddr key;
        dsr >> key;
        istore_.setIndex( key, idx );
    }
private:
    DiskIndexStorage&              istore_;
    DiskDataStorage::storage_type& dstore_;
};
#endif
 */


unsigned long AbonentLogic::rebuildElementStorage( unsigned index, unsigned maxSpeed )
{
    char pathSuffix[4];
    snprintf(pathSuffix, sizeof(pathSuffix), "%03u", index);
    string path = string(config_.locations[locationNumber_].path + "/") + pathSuffix;
    // std::auto_ptr<ElementStorage> elStorage(new ElementStorage(index));
    // elStorage->glossary = new Glossary();
    // initGlossary(path, elStorage->glossary);

    const std::string pathSuffixString(pathSuffix);

#ifdef PVSSLOGIC_BHS2
    std::auto_ptr< DiskDataStorage::storage_type > bs
        (new DiskDataStorage::storage_type
         (dataFileManager_,
          smsc::logger::Logger::getInstance(("pvssbh."+pathSuffixString).c_str())));
#else
    // do we need glossary here ?
    // I (db) think 'no', as we don't unpack datablocks
    std::auto_ptr< DiskDataStorage::storage_type > bs
        (new DiskDataStorage::storage_type
         (dataFileManager_,
          0, // elStorage->glossary,
          smsc::logger::Logger::getInstance(("pvssbh."+pathSuffixString).c_str())));

    const string fn(config_.dbName + "-data");
    int ret = bs->Open(fn, path);
    if (ret < 0 && ret != DiskDataStorage::storage_type::FIRST_FREE_BLOCK_FAILED ) {
        throw Exception("can't open data disk storage: %s", path.c_str());
    }
#endif
    // we have opened data storage

    /// making sure that temporary index file is not here
    std::string n = path + "/" + config_.dbName + "-index";
    std::string t = path + "/" + config_.dbName + "-temp-index";
    std::string o = path + "/" + config_.dbName + "-old-index";
    try {
        File::Unlink(t.c_str());
        File::Unlink((t+".jnl").c_str());
    } catch (...) {}

    /// create a temporary index file
    const bool fullRecovery = true;
    std::auto_ptr< DiskIndexStorage > dis
        (new DiskIndexStorage( config_.dbName + "-temp", path, config_.indexGrowth, false,
                               smsc::logger::Logger::getInstance(("pvssix."+pathSuffixString).c_str()),
                               fullRecovery ));
    dis->setInvalidIndex( bs->invalidIndex() );
    smsc_log_debug(logger_, "temporary data index storage %u is created", index);

    // rebuilding index
#ifdef PVSSLOGIC_BHS2
    DiskDataStorage dds(bs.release(),
                        0, // glossary
                        0 ); // logger
    DiskDataStorage::IndexRescuer< DiskIndexStorage > indexRescuer(*dis.get(),dds);
    const string fn(config_.dbName + "-data");
    int ret = indexRescuer.recover(fn, path);
    if ( ret < 0 ) {
        throw Exception("can't recover data disk storage: %s", path.c_str());
    }
#else
    for ( DiskDataStorage::storage_type::Iterator iter = bs->beginWithRecovery(); iter.next(); ) {
        smsc_log_debug( logger_, "rebuilding: key=%s, idx=%lx",
                        iter.key().toString().c_str(),
                        long(iter.blockIndex()) );
        dis->setIndex(iter.key(),iter.blockIndex());
    }
#endif
    const size_t rebuilt = dis->flush(maxSpeed);
    smsc_log_info( logger_, "storage %s indices rebuilt: %u", path.c_str(), unsigned(rebuilt) );

    doRename(t,n,o);
    doRename(t,n,o,".trx");
    doRename(t,n,o,".jnl");
    return rebuilt;
}


Response* AbonentLogic::processProfileRequest(ProfileRequest& profileRequest) {

  const ProfileKey &profileKey = profileRequest.getProfileKey();
  const std::string& profkey = profileKey.getAddress().toString();

  unsigned elstorageIndex = static_cast<unsigned>(profileKey.getAddress().getNumber() % dispatcher_.getStoragesCount());
  smsc_log_debug( logger_, "%p: %p process profile key='%s' in location: %d, storage: %d",
                  this, &profileRequest, profkey.c_str(), locationNumber_, elstorageIndex);

  ElementStorage **elstoragePtr = elementStorages_.GetPtr(elstorageIndex);
  if (!elstoragePtr) {
    smsc_log_warn(logger_, "%p: %p element storage %d not found in location %d", this, &profileRequest, elstorageIndex, locationNumber_);
    return 0;
  }
  ElementStorage* elstorage = *elstoragePtr;

  bool createProfile = profileRequest.getCommand()->visit(createProfileVisitor);
  Profile *pf = elstorage->storage->get(profileKey.getAddress(), createProfile);
  if (pf) {
      smsc_log_debug(logger_,"FIXME(pre): prof=%s",pf->toString().c_str());
      if ( pf->getKey() != profkey ) {
          smsc_log_warn(logger_,"key mismatch: pf=%p pf.key=%s req.key=%s",
                        pf,pf->getKey().c_str(),profkey.c_str());
          pf->setKey(profkey);
      }
  }
    /*
  if ( pf && pf->getKey() != profileKey.getAddress().toString() ) {
      smsc_log_error(logger_,"key mismatch: pf=%p pf.key=%s req.key=%s", pf, pf->getKey().c_str(), profileKey.getAddress().toString().c_str() );
      abort();
      return 0;
  }
     */
  commandProcessor_.setProfile(pf);
  profileRequest.getCommand()->visit(commandProcessor_);
  if (!pf) {
    if (createProfile) {
        smsc_log_warn(logger_, "%p: %p can't create profile %s", this, &profileRequest, profkey.c_str());
    }
    CommandResponse* r = commandProcessor_.getResponse();
    return r ? new ProfileResponse(profileRequest.getSeqNum(),r) : 0;
  }
  if (commandProcessor_.needRollback()) {
    smsc_log_debug(logger_, "%p: %p rollback profile %s changes", this, &profileRequest, profkey.c_str());
    commandProcessor_.rollback();
    commandProcessor_.resetProfile();
    // NOTE: backup2profile may delete object under pf!
  } else if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile %s", this, &profileRequest, profkey.c_str());
      {
          MutexGuard mg(elstorage->mutex);
          elstorage->storage->flush(profileKey.getAddress());
      }
    commandProcessor_.flushLogs(abntlog_);
  }

    smsc_log_debug(logger_,"FIXME(post): prof=%s",pf->toString().c_str());

  CommandResponse* r = commandProcessor_.getResponse();
  return r ? new ProfileResponse(profileRequest.getSeqNum(),r) : 0;
}

// ==== infrastruct


Response* InfrastructLogic::processProfileRequest(ProfileRequest& profileRequest) {
  Logger *dblog = 0;
  InfrastructStorage* storage = 0;
  uint32_t key = 0;
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
  
  bool createProfile = profileRequest.getCommand()->visit(createProfileVisitor); 
  Profile *pf = storage->get(intKey, createProfile);
  if (pf) {
      smsc_log_debug(logger_,"FIXME(pre): prof=%s",pf->toString().c_str());
      pf->setKey(intKey.toString());
  }

  commandProcessor_.setProfile(pf);
  profileRequest.getCommand()->visit(commandProcessor_);
  if (!pf) {
    if (createProfile) {
        smsc_log_warn(logger_, "%p: %p can't create profile %s", this, &profileRequest, intKey.toString().c_str());
    }
    CommandResponse* r = commandProcessor_.getResponse();
    return r ? new ProfileResponse(profileRequest.getSeqNum(),r) : 0;
  }

  if (commandProcessor_.needRollback()) {
    smsc_log_debug(logger_, "%p: %p rollback profile %s changes", this, &profileRequest, intKey.toString().c_str());
    commandProcessor_.rollback();
  } else if (pf->isChanged()) {
    smsc_log_debug(logger_, "%p: %p flush profile %s", this, &profileRequest, intKey.toString().c_str());
      {
          MutexGuard mg(statMutex_);
          // storage->flush(intKey);
          storage->markDirty(intKey);
      }
    commandProcessor_.flushLogs(dblog);
  } else {
      storage->flushDirty();
  }

    smsc_log_debug(logger_,"FIXME(post): prof=%s",pf->toString().c_str());

  CommandResponse* r = commandProcessor_.getResponse();
  return r ? new ProfileResponse(profileRequest.getSeqNum(),r) : 0;
}


PvssLogic::LogicInitTask* InfrastructLogic::startInit( bool checkAtStart )
{
    // we do in in the main thread
    std::auto_ptr<LogicInitTask> task(new LogicInitTask(this,checkAtStart));
    task->Execute();
    return 0;
}
PvssLogic::LogicRebuildIndexTask* InfrastructLogic::startRebuildIndex(unsigned maxSpeed)
{
    // we do in in the main thread
    std::auto_ptr<LogicRebuildIndexTask> task(new LogicRebuildIndexTask(this,maxSpeed));
    task->Execute();
    return 0;
}
void InfrastructLogic::dumpStorage( int i )
{
    smsc_log_warn(logger_,"dump storage is not impl yet");
}


std::string InfrastructLogic::reportStatistics() const
{
    unsigned long providerStat, operatorStat, serviceStat;
    {
        MutexGuard mg(statMutex_);
        providerStat = provider_->filledDataSize();
        operatorStat = operator_->filledDataSize();
        serviceStat = service_->filledDataSize();
    }
    char buf[100];
    snprintf(buf,sizeof(buf),"providers=%lu services=%lu operators=%lu",
             providerStat, serviceStat, operatorStat);
    return buf;
}


void InfrastructLogic::init( bool checkAtStart ) /* throw (smsc::util::Exception) */ {
  InfrastructStorageConfig locCfg(config_);
  initGlossary(locCfg.dbPath, &glossary_);
  locCfg.dbName = "provider";
  provider_ = initStorage(locCfg,checkAtStart,".pr");
    provider_->setProfileLog(plog_);
  smsc_log_debug(logger_, "provider storage is created");
  locCfg.dbName = "service";
  service_ = initStorage(locCfg,checkAtStart,".sv");
    service_->setProfileLog(slog_);
  smsc_log_debug(logger_, "service storage is created");
  locCfg.dbName = "operator";
  operator_ = initStorage(locCfg,checkAtStart,".op");
    operator_->setProfileLog(olog_);
  smsc_log_debug(logger_, "operator storage is created");
    smsc_log_info(logger_,"infrastructure storages are inited, good nodes: provider=%lu, service=%lu, operator=%lu", 
                  static_cast<unsigned long>(provider_->filledDataSize()),
                  static_cast<unsigned long>(service_->filledDataSize()),
                  static_cast<unsigned long>(operator_->filledDataSize()));
}


void InfrastructLogic::rebuildIndex( unsigned /*maxSpeed*/)
{
    smsc_log_warn(logger_,"infrastructure index rebuilding is not impl yet");
}


InfrastructLogic::InfrastructStorage* InfrastructLogic::initStorage(const InfrastructStorageConfig& cfg,
                                                                    bool  checkAtStart,
                                                                    const std::string& logsfx )
{
  const string fn(cfg.dbPath + "/" + cfg.dbName + ".bin");
  std::auto_ptr< DiskDataStorage::storage_type > pf(new DiskDataStorage::storage_type);
  try {
    pf->Open(fn);
  } catch (...) {
    pf->Create(fn, 256, cfg.recordCount);
  }

  std::auto_ptr< DiskDataStorage > data(new DiskDataStorage(pf.release(),
                                                            Logger::getInstance(("pvssdd"+logsfx).c_str())));
  smsc_log_debug(logger_, "%s data storage is created", cfg.dbName.c_str());

  std::auto_ptr< DiskIndexStorage > index(new DiskIndexStorage(cfg.dbName,
                                                               cfg.dbPath,
                                                               cfg.recordCount,
                                                               Logger::getInstance(("pvssix"+logsfx).c_str())));
  // FIX: mandatory size recalculation
  index->recalcSize();
  smsc_log_debug(logger_, "%s index storage is created", cfg.dbName.c_str());

  std::auto_ptr< DiskStorage > ds (new DiskStorage(index.release(), data.release()));
  smsc_log_debug(logger_, "%s indexed storage is created", cfg.dbName.c_str());

  std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance(("pvssmc"+logsfx).c_str()),
                                                cfg.cacheSize));
  smsc_log_debug(logger_, "%s memory storage is created", cfg.dbName.c_str());

  std::auto_ptr<DataSerializer> ps( new DataSerializer(ds.get(),&glossary_) );
  smsc_log_debug(logger_,"%s serializer is created", cfg.dbName.c_str());

    InfrastructStorage* st = new InfrastructStorage(ms.release(),ds.release(),ps.release(),
                                                    Logger::getInstance(("pvssst"+logsfx).c_str()));
    st->init(cfg.minDirtyTime, cfg.maxDirtyTime, cfg.maxDirtyCount);
    return st;
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
  checkAtStart = false;
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
    /*
     * It is not taken from config
  try {
    checkAtStart = cfg.getBool("checkAtStart");
  } catch (...) {
    checkAtStart = false;
    smsc_log_warn(logger, "Parameter <PVSS.%s.checkAtStart> missed. Default value is %d",
                  storageType, checkAtStart);
  }
     */
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

    try {
        minDirtyTime = cfg.getInt("minDirtyTime");
    } catch (...) {
        minDirtyTime = 10;
        smsc_log_warn(logger,"Parameter <Pvss.%s.minDirtyTime> missed. Default value is %u",
                      storageType,10);
    }
    try {
        maxDirtyTime = cfg.getInt("maxDirtyTime");
    } catch (...) {
        maxDirtyTime = 10000;
        smsc_log_warn(logger,"Parameter <Pvss.%s.maxDirtyTime> missed. Default value is %u",
                      storageType,10000);
    }
    try {
        maxDirtyCount = cfg.getInt("maxDirtyCount");
    } catch (...) {
        maxDirtyCount = 100;
        smsc_log_warn(logger,"Parameter <Pvss.%s.maxDirtyCount> missed. Default value is %u",
                      storageType,100);
    }
}

}//pvss
}//scag2
