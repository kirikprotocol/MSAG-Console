#include "PvssLogic.h"
#include "PvssDispatcher.h"

#include <ctime>
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
#include "util/config/ConfString.h"

#include "scag/util/storage/CachedDelayedThreadedDiskStorage.h"

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

void PvssLogic::initGlossary(const string& path, Glossary& glossary) {
    if (!File::Exists(path.c_str())) {
        smsc_log_debug(logger_, "create storage dir '%s'", path.c_str());
        File::MkDir(path.c_str());
    }
    string fn = path + DEF_GLOSSARY_NAME;
    if ( GlossaryBase::SUCCESS != glossary.Open(fn) ) {
        throw std::runtime_error("StorageProcessor: glossary open error");
    }
}

Response* PvssLogic::process(Request& request) /* throw(PvssException) */  {
  try {
      ProfileRequest& preq(static_cast<ProfileRequest&>(request));
      CommandResponse* r = processProfileRequest(preq);
      return r ? new ProfileResponse(preq.getSeqNum(),r) : 0;
  } catch (const PvapException& e) {
    smsc_log_warn(logger_, "%p: %p processing error: PvapException", this, &request);
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
  initGlossary(path, *elStorage->glossary);

    const std::string pathSuffixString(pathSuffix);
    std::auto_ptr< DiskIndexStorage > dis
        (new DiskIndexStorage( config_.dbName,
                               path,
                               config_.indexGrowth,
                               false,
                               smsc::logger::Logger::getInstance(("pvssix."+pathSuffixString).c_str())));
    smsc_log_debug(logger_, "data index storage %d is created", index);
    if ( checkAtStart ) {
        dis->checkTree();
        smsc_log_debug(logger_, "data index storage %d is checked", index);
    }

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
    smsc_log_debug(logger_, "data disk storage %d is created", index);

    std::auto_ptr< DiskStorage > ds(new DiskStorage(dis.release(), dds.release()));
    smsc_log_debug(logger_, "disk storage is assembled");

    std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance(("pvssmc."+pathSuffixString).c_str()), config_.cacheSize));
    smsc_log_debug(logger_, "memory storage is created");

    // std::auto_ptr<DataSerializer> ps(new DataSerializer(ds.get(),elStorage->glossary));
    elStorage->storage = new AbonentStorage(ms.release(),
                                            ds.release(),
                                            // ps.release(),
                                            smsc::logger::Logger::getInstance
                                            (("pvssst."+pathSuffixString).c_str()));
    // elStorage->storage->init( config_.minDirtyTime,
    // config_.maxDirtyTime,
    // config_.maxDirtyCount );
    elStorage->storage->setProfileBackup(profileBackup_);

  unsigned long filledNodes = elStorage->storage->filledDataSize();
  elementStorages_.Insert(index, elStorage.release());
  smsc_log_debug(logger_, "abonent storage is assembled");
  smsc_log_info( logger_, "storage #%u is inited, total number of good nodes: %lu", index, filledNodes );
  return filledNodes;
}


unsigned long AbonentLogic::rebuildElementStorage( unsigned index, unsigned maxSpeed )
{
    char pathSuffix[4];
    snprintf(pathSuffix, sizeof(pathSuffix), "%03u", index);
    string path = string(config_.locations[locationNumber_].path + "/") + pathSuffix;
    // std::auto_ptr<ElementStorage> elStorage(new ElementStorage(index));
    // elStorage->glossary = new Glossary();
    // initGlossary(path, elStorage->glossary);

    const std::string pathSuffixString(pathSuffix);

    std::auto_ptr< DiskDataStorage::storage_type > bs
        (new DiskDataStorage::storage_type
         (dataFileManager_,
          smsc::logger::Logger::getInstance(("pvssbh."+pathSuffixString).c_str())));
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
    DiskDataStorage dds(bs.release(), 0, 0);
    DiskDataStorage::IndexRescuer< DiskIndexStorage > indexRescuer(*dis.get(),dds);
    const string fn(config_.dbName + "-data");
    int ret = indexRescuer.recover(fn, path);
    if ( ret < 0 ) {
        throw Exception("can't recover data disk storage: %s", path.c_str());
    }

    const size_t rebuilt = dis->flush(maxSpeed);
    smsc_log_info( logger_, "storage %s indices rebuilt: %u", path.c_str(), unsigned(rebuilt) );

    doRename(t,n,o);
    doRename(t,n,o,".trx");
    doRename(t,n,o,".jnl");
    return rebuilt;
}


CommandResponse* AbonentLogic::processProfileRequest(ProfileRequest& profileRequest)
{
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

    const bool createProfile = profileRequest.getCommand()->visit(createProfileVisitor);
    Profile* pf = elstorage->storage->get(profileKey.getAddress(), createProfile);
    /// FIXME: temporary
    if (pf) pf->setKey( profkey );
    if ( commandProcessor_.applyCommonLogic(profkey, profileRequest, pf, createProfile )) {
        if ( pf->isChanged() ) {
            MutexGuard mg(elstorage->mutex);
            elstorage->storage->flush(profileKey.getAddress());
            // elstorage->storage->markDirty(profileKey.getAddress());
        // } else {
            // elstorage->storage->flushDirty();
        }
    }
    return commandProcessor_.getResponse();
}

// ==== infrastruct


class InfrastructLogic::InfraLogic
{
private:

    template < class MemStorage, class DiskStorage > struct ProfileSerializer
    {
    public:
        typedef typename MemStorage::key_type     key_type;
        typedef typename MemStorage::stored_type  stored_type;
        typedef typename DiskStorage::buffer_type buffer_type;

        ProfileSerializer( DiskStorage* disk,
                           GlossaryBase* glossary = 0 ) :
        disk_(disk), glossary_(glossary), newbuf_(0), ownbuf_(0) {
            if (!disk) throw smsc::util::Exception("disk storage should be provided");
        }

        ~ProfileSerializer() {
            if (newbuf_) delete newbuf_;
        }

        /// --- reading

        /// making a new buffer
        buffer_type* getFreeBuffer( bool create = false ) {
            if (!newbuf_ && create) { newbuf_ = new buffer_type; }
            return newbuf_; 
        }

        buffer_type* getOwnedBuffer() {
            return ownbuf_;
        }

        /// deserialization && attaching the buffer
        bool deserialize( const key_type& k, stored_type& val ) {
            assert(newbuf_ && val.value );
            util::io::Deserializer dsr(*newbuf_,glossary_);
// #ifdef ABONENTSTORAGE
//            disk_->unpackBuffer(*newbuf_,&hdrbuf_);
//            dsr.setrpos(disk_->headerSize());
//            dsr >> *val.value;
//            disk_->packBuffer(*newbuf_,&hdrbuf_);
//#else
            dsr >> *val.value;
//#endif
            
            std::swap(val.backup,newbuf_);
            ownbuf_ = val.backup;
            return true;
        }

        /// --- writing
        void serialize( const key_type& k, stored_type& val ) {
            assert( val.value );
            if (!newbuf_) { newbuf_ = new buffer_type; }
            newbuf_->clear();
            util::io::Serializer ser(*newbuf_,glossary_);
//#ifdef ABONENTSTORAGE
//            ser.setwpos(disk_->headerSize());
//            ser << *val.value;
//            disk_->packBuffer(*newbuf_);
//#else
            ser << *val.value;
//#endif
            std::swap(val.backup,newbuf_);
            ownbuf_ = val.backup;
        }

    private:
        DiskStorage*  disk_;     // not owned
        GlossaryBase* glossary_; // not owned
        buffer_type*  newbuf_;   // owned
        buffer_type*  ownbuf_;   // not owned (owned by other stored_type instance)
        // buffer_type   hdrbuf_;
    };

    typedef PageFileDiskStorage2 DiskDataStorage;
    typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
    typedef ArrayedMemoryCache< IntProfileKey, LockableProfile, DataBlockBackupTypeJuggling2 > MemStorage;
    typedef IndexedStorage2< DiskIndexStorage, DiskDataStorage > DiskStorage;
    typedef ProfileSerializer< MemStorage, DiskStorage > DataSerializer;
    typedef CachedDelayedThreadedDiskStorage< MemStorage, DiskStorage, DataSerializer, ProfileHeapAllocator< MemStorage::key_type, LockableProfile > > InfrastructStorage;

public:
    InfraLogic( const char* dblogName ) :
    profileBackup_(smsc::logger::Logger::getInstance(dblogName)),
    commandProcessor_(profileBackup_),
    log_(0),
    storage_(0)
    {}
    
    void init( const std::string& name,
               const std::string& logsfx,
               const InfrastructStorageConfig& cfg,
               Glossary& glossary ) {
        log_ = smsc::logger::Logger::getInstance(("pvssst" + logsfx).c_str());
        smsc_log_debug(log_,"starting initialization of %s infralogic",name.c_str());
        const std::string fn( cfg.dbPath + "/" + name + ".bin");
        std::auto_ptr< DiskDataStorage::storage_type > pf(new DiskDataStorage::storage_type);
        try {
            pf->Open(fn);
        } catch (...) {
            pf->Create(fn, cfg.pageSize, cfg.recordCount);
        }

        // successfully open, checking if pageSize is smaller than requested
        if ( pf->getPageSize() != cfg.pageSize ) {
            resizePageSize(pf,cfg.dbPath+"/"+name,cfg);
        }

        std::auto_ptr< DiskDataStorage > data(new DiskDataStorage(pf.release(),
                                                                  Logger::getInstance(("pvssdd"+logsfx).c_str())));
        smsc_log_debug(log_, "infralogic %s disk data storage is created", name.c_str());

        std::auto_ptr< DiskIndexStorage > index(new DiskIndexStorage(name,
                                                                     cfg.dbPath,
                                                                     cfg.recordCount,
                                                                     Logger::getInstance(("pvssix"+logsfx).c_str())));
        // FIX: mandatory size recalculation
        index->recalcSize();
        smsc_log_debug(log_, "infralogic %s disk index storage is created", name.c_str());

        std::auto_ptr< DiskStorage > ds(new DiskStorage(index.release(), data.release()));
        smsc_log_debug(log_, "infralogic %s indexed storage is created", name.c_str());

        std::auto_ptr< MemStorage > ms(new MemStorage(Logger::getInstance(("pvssmc"+logsfx).c_str()),
                                                      cfg.cacheSize));
        smsc_log_debug(log_, "infralogic %s memory storage is created", name.c_str());

        std::auto_ptr<DataSerializer> ps1( new DataSerializer(ds.get(),&glossary) );
        std::auto_ptr<DataSerializer> ps2( new DataSerializer(ds.get(),&glossary) );
        smsc_log_debug(log_,"infralogic %s serializers are created", name.c_str());

        storage_ = new InfrastructStorage(ms.release(),ds.release(),
                                          ps1.release(),ps2.release(),
                                          Logger::getInstance(("pvssst"+logsfx).c_str()));
        storage_->init(cfg.maxDirtySpeed);
        storage_->setProfileBackup(profileBackup_);
        smsc_log_info(log_,"infralogic %s initialized",name.c_str());
    }

    ~InfraLogic() {
        delete storage_;
    }

    void shutdown() {
        delete storage_; storage_ = 0;
    }
    void dumpStorage( int index );
    void init( bool checkAtStart = false );
    void rebuildIndex( unsigned maxSpeed = 0 );
    /*
    void keepAlive( util::msectime_type now ) {
        // not needed
        // MutexGuard mg(statMutex_);
        // storage_->flushDirty(now);
    }
     */

    CommandResponse* process( const IntProfileKey& intKey,
                              ProfileRequest& request );

    unsigned long filledDataSize() const {
        return static_cast<unsigned long>(storage_->filledDataSize());
    }
    
private:
    void resizePageSize( std::auto_ptr< DiskDataStorage::storage_type >& pf,
                         const std::string& fold,
                         const InfrastructStorageConfig& cfg );

private:
    ProfileBackup           profileBackup_;
    ProfileCommandProcessor commandProcessor_;
    smsc::logger::Logger*   log_;
    // smsc::core::synchronization::Mutex statMutex_;
    InfrastructStorage*                storage_;
    LockableProfile         dummyProfile_;  // is used to be locked when there is no profile
};



void InfrastructLogic::InfraLogic::resizePageSize( std::auto_ptr< DiskDataStorage::storage_type >& pf,
                                                   const std::string& fold,
                                                   const InfrastructStorageConfig& cfg )
{
    smsc_log_info(log_,"storage %s pageSize is to be resized from %u to %u",
                  fold.c_str(), pf->getPageSize(), cfg.pageSize);
    std::auto_ptr< DiskStorage > ostore;
    {
        std::auto_ptr< DiskDataStorage > data( new DiskDataStorage(pf.release(),
                                                                   smsc::logger::Logger::getInstance("olddd")));
        smsc_log_debug(log_,"data storage is opened");
        std::auto_ptr< DiskIndexStorage > index( new DiskIndexStorage(fold+".idx",
                                                                      cfg.recordCount,
                                                                      smsc::logger::Logger::getInstance("oldix")));
        smsc_log_debug(log_,"index storage is opened");
        ostore.reset(new DiskStorage(index.release(),data.release()));
    }
    
    char datebuf[40];
    {
        struct tm stm;
        stm.tm_isdst = 0;
        const time_t curt = time(0);
        localtime_r(&curt,&stm);
        std::strftime(datebuf,sizeof(datebuf),"-%Y%m%d%H%M%S",&stm);
    }

    // creating the new storage
    const std::string fnew = fold + "-tmp" + datebuf;
    std::auto_ptr< DiskStorage > nstore;
    {
        try {
            smsc::core::buffers::File::Unlink((fnew+".bin").c_str());
        } catch (...) {
        }
        std::auto_ptr< DiskDataStorage::storage_type > pf(new DiskDataStorage::storage_type);
        try {
            pf->Create(fnew+".bin", cfg.pageSize, cfg.recordCount);
        } catch (...) {
            smsc_log_error(log_,"file %s.bin cannot be created",fnew.c_str());
            throw smsc::util::Exception("file %s.bin cannot be created",fnew.c_str());
        }

        std::auto_ptr< DiskDataStorage > data( new DiskDataStorage(pf.release(),
                                                                   smsc::logger::Logger::getInstance("newdd")));
        smsc_log_debug(log_,"data storage is created");

        try {
            smsc::core::buffers::File::Unlink((fnew+".idx").c_str());
        } catch (...) {
        }
        std::auto_ptr< DiskIndexStorage > index( new DiskIndexStorage(fnew+".idx",
                                                                      cfg.recordCount,
                                                                      smsc::logger::Logger::getInstance("newix")));
        smsc_log_debug(log_,"index storage is created");
        nstore.reset(new DiskStorage(index.release(),data.release()));
    }
    
    // main resizing loop
    DiskStorage::key_type    k;
    DiskStorage::buffer_type b;
    for ( DiskStorage::iterator_type i(ostore->begin()); i.next(k,b); ) {
        nstore->set(k,b);
        smsc_log_info(log_,"key=%s written",k.toString().c_str());
    }

    // closing everything
    nstore.reset(0);
    ostore.reset(0);

    // renaming files
    smsc_log_info(log_,"renaming files...");
    smsc::core::buffers::File::Rename((fold+".bin").c_str(),(fold+".bin"+datebuf).c_str());
    smsc::core::buffers::File::Rename((fold+".idx").c_str(),(fold+".idx"+datebuf).c_str());
    smsc::core::buffers::File::Rename((fnew+".bin").c_str(),(fold+".bin").c_str());
    smsc::core::buffers::File::Rename((fnew+".idx").c_str(),(fold+".idx").c_str());

    // reopening the file
    pf.reset( new DiskDataStorage::storage_type );
    try {
        pf->Open(fold+".bin");
    } catch (...) {
        smsc_log_error(log_,"cannot open %s.bin after pageSize resizing",fold.c_str());
        throw smsc::util::Exception("cannot open %s.bin",fold.c_str());
    }
}



CommandResponse* InfrastructLogic::InfraLogic::process( const IntProfileKey& intKey,
                                                        ProfileRequest& profileRequest )
{
    const bool createProfile = profileRequest.getCommand()->visit(createProfileVisitor); 
    LockableProfile* pf = storage_->get(intKey, createProfile);
    const std::string profkey = intKey.toString();
    bool ok;
    {
        smsc::core::synchronization::MutexGuardTmpl< LockableProfile > mg(pf?*pf:dummyProfile_);
        ok = commandProcessor_.applyCommonLogic(profkey,profileRequest,pf,createProfile);
    }
    if (ok) {
        if ( pf->isChanged() ) {
            storage_->markDirty(intKey);
        }
    }
    return commandProcessor_.getResponse();
}


CommandResponse* InfrastructLogic::processProfileRequest(ProfileRequest& profileRequest)
{
    uint32_t key = 0;
    InfraLogic* logic = 0;
    const ProfileKey& profileKey = profileRequest.getProfileKey();
    if (profileKey.hasOperatorKey()) {
        logic = operator_;
        key = profileKey.getOperatorKey();
    } else if (profileKey.hasProviderKey()) {
        logic = provider_;
        key = profileKey.getProviderKey();
    } else if (profileKey.hasServiceKey()) {
        logic = service_;
        key = profileKey.getServiceKey();
    } else {
        throw smsc::util::Exception("unknown profile key %s", profileKey.toString().c_str());
    }
    IntProfileKey intKey(key);
    return logic->process(key,profileRequest);
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
    providerStat = provider_->filledDataSize();
    operatorStat = operator_->filledDataSize();
    serviceStat = service_->filledDataSize();
    char buf[100];
    snprintf(buf,sizeof(buf),"providers=%lu services=%lu operators=%lu",
             providerStat, serviceStat, operatorStat);
    return buf;
}


/*
void InfrastructLogic::keepAlive()
{
    util::msectime_type now = util::currentTimeMillis();
    // MutexGuard mg(statMutex_);
    provider_->keepAlive(now);
    service_->keepAlive(now);
    operator_->keepAlive(now);
}
 */


void InfrastructLogic::init( bool /*checkAtStart*/ )
{
    initGlossary(config_.dbPath,glossary_);
    provider_ = new InfraLogic("pvss.prov");
    provider_->init("provider",".pr",config_,glossary_);
    service_ = new InfraLogic("pvss.serv");
    service_->init("service",".sv",config_,glossary_);
    operator_ = new InfraLogic("pvss.oper");
    operator_->init("operator",".op",config_,glossary_);
    smsc_log_info(logger_,"infrastructure storages are inited, good nodes: %s",
                  reportStatistics().c_str());
}


void InfrastructLogic::rebuildIndex( unsigned /*maxSpeed*/)
{
    smsc_log_warn(logger_,"infrastructure index rebuilding is not impl yet");
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
//    minDirtyTime = 10;
//    maxDirtyTime = 10000;
//    maxDirtyCount = 100;
}

AbonentStorageConfig::AbonentStorageConfig(ConfigView& cfg,
                                           const char* storageType,
                                           Logger* logger) {
  try {
    dbName = ConfString(cfg.getString("storageName")).str();
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
    dbPath = ConfString(cfg.getString("storagePath")).str();
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
        maxDirtySpeed = cfg.getInt("maxDirtySpeed");
    } catch (...) {
        maxDirtySpeed = 10000;
        smsc_log_warn(logger,"Parameter <Pvss.%s.maxDirtySpeed> missed. Default value is %u",
                      storageType,maxDirtySpeed);
    }

    try {
        pageSize = cfg.getInt("pageSize");
        if ( pageSize < 256 ) {
            unsigned newval = 256;
            smsc_log_warn(logger,"pageSize=%u is too small, using default value %u",
                          pageSize,newval);
            pageSize = newval;
        }
    } catch (...) {
        pageSize = 256;
        smsc_log_warn(logger,"Parameter <Pvss.%s.pageSize> missed. Default value is %u",
                      storageType,pageSize);
    }
}

}//pvss
}//scag2
