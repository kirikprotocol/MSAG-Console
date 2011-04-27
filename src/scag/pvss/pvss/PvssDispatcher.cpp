#include "PvssDispatcher.h"
#include "ProfileLogRollerHardcoded.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/pvss/data/ProfileKey.h"
// #include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"
#include "scag/counter/Accumulator.h"
#include "scag/counter/Manager.h"
#include "util/PtrDestroy.h"

namespace {

using namespace scag2::pvss;

class ProfileRequestChecker : public RequestVisitor
{
    virtual bool visitPingRequest( PingRequest& ) { return false; }
    virtual bool visitAuthRequest( AuthRequest& ) { return false; }
    virtual bool visitProfileRequest( ProfileRequest& ) { return true; }
};
ProfileRequestChecker prc;
inline bool isProfileRequest( Request& r ) {
    return r.visit(prc);
}

}


namespace scag2 {
namespace pvss  {

using scag::util::storage::StorageNumbering;


class PvssDispatcher::DiskManager
{
public:
    DiskManager( unsigned idx,
                 unsigned addSpeed,
                 const AbonentStorageConfig& abntCfg ) :
    dataFileManager_(0), diskFlusher_(0)
    {
        char buf[15];
        sprintf(buf,"%03u",idx);
        dataFileManager_ = new util::storage::DataFileManager(1,addSpeed,abntCfg.fileSize/2);
        diskFlusher_ = new util::storage::DiskFlusher(buf,abntCfg.flushConfig);
        
        counter::Manager& mgr = counter::Manager::getInstance();
        const std::string name = buf;
        cntpfr_ = mgr.registerAnyCounter( new counter::Accumulator("sys.profiles." + name + ".r") );
        cntpfw_ = mgr.registerAnyCounter( new counter::Accumulator("sys.profiles." + name + ".w") );
        cntkbr_ = mgr.registerAnyCounter( new counter::Accumulator("sys.kbytes." + name + ".r") );
        cntkbw_ = mgr.registerAnyCounter( new counter::Accumulator("sys.kbytes." + name + ".w") );
    }

    ~DiskManager() {
        delete diskFlusher_;
        delete dataFileManager_;
    }

    DataFileManager& getFileManager() { return *dataFileManager_; }
    DiskFlusher& getFlusher() { return *diskFlusher_; }

    void start() {
        diskFlusher_->start();
    }

    void stop() {
        dataFileManager_->shutdown();
        diskFlusher_->stop();
    }

    void flushIOStatistics( unsigned& pfget, unsigned& kbget,
                            unsigned& pfset, unsigned& kbset ) {
        diskFlusher_->flushIOStatistics(pfget,kbget,pfset,kbset);
        cntpfr_->increment(pfget);
        cntpfw_->increment(pfset);
        cntkbr_->increment(kbget);
        cntkbw_->increment(kbset);
    }

    const std::string& getName() const { return name_; }

private:
    std::string            name_;
    DataFileManager*       dataFileManager_; // owned
    DiskFlusher*           diskFlusher_;     // owned
    counter::CounterPtrAny cntpfr_;
    counter::CounterPtrAny cntpfw_;
    counter::CounterPtrAny cntkbr_;
    counter::CounterPtrAny cntkbw_;
};


PvssDispatcher::PvssDispatcher(const NodeConfig& nodeCfg,
                               const AbonentStorageConfig& abntCfg,
                               const InfrastructStorageConfig* infCfg) :
nodeCfg_(nodeCfg), createdLocations_(0), infrastructIndex_(nodeCfg_.locationsCount),
logger_(Logger::getInstance("pvss.disp")),
infraFlusher_(0),
logRoller_( new ProfileLogRollerHardcoded() )
{
    StorageNumbering::setInstance(nodeCfg.nodesCount);
    unsigned addSpeed = nodeCfg_.expectedSpeed / nodeCfg_.disksCount;
    for (unsigned i = 0; i < nodeCfg_.disksCount; ++i) {
        diskManagers_.push_back( new DiskManager( i, addSpeed, abntCfg) );
    }
    if (infCfg) {
        infraFlusher_ = new scag::util::storage::DiskFlusher("inf",infCfg->flushConfig);
    }
    cntAbonents_ = counter::Manager::getInstance().registerAnyCounter
        ( new counter::Accumulator("sys.profiles.abn.total") );
    smsc_log_info(logger_, "nodeNumber:%d, nodesCount:%d, storagesCount:%d, locationsCount:%d, disksCount:%d",
                  nodeCfg_.nodeNumber, nodeCfg_.nodesCount, nodeCfg_.storagesCount, nodeCfg_.locationsCount, nodeCfg_.disksCount);
}

unsigned PvssDispatcher::getIndex(Request& request) const 
{
    if ( ! isProfileRequest(request) ) return getErrorIndex();

  ProfileRequest& profileRequest = static_cast<ProfileRequest&>(request);
  const ProfileKey& profileKey = profileRequest.getProfileKey();

  if (profileKey.hasOperatorKey() || profileKey.hasProviderKey() || profileKey.hasServiceKey()) {
    if (nodeCfg_.nodeNumber != getInfrastructNodeNumber()) {
      smsc_log_warn(logger_, "can't process infrastruct request on node number=%d", nodeCfg_.nodeNumber);
      return getErrorIndex();
    }
    smsc_log_debug(logger_, "give %p packet to infrastruct storage node %d", &request, getInfrastructNodeNumber());
    return infrastructIndex_;
  }

  unsigned storageNumber = static_cast<unsigned>(profileKey.getAddress().getNumber() % nodeCfg_.storagesCount);
  smsc_log_debug(logger_, "give %p packet to storage %d in node %d", &request, storageNumber, nodeCfg_.nodeNumber);
  return getLocationNumber( storageNumber );
}

Server::SyncLogic* PvssDispatcher::getSyncLogic(unsigned idx) {
  if (getErrorIndex() == idx || idx > nodeCfg_.locationsCount) {
    return 0;
  }
  if (idx == infrastructIndex_) {
    return infrastructLogic_.get();
  }
  //return getLocation(idx);
  return abonentLogics_[idx];
}


std::string PvssDispatcher::reportStatistics() const
{
    unsigned long total = 0;
    PvssDispatcher* that = const_cast<PvssDispatcher*>(this);
    std::string rv;
    for ( unsigned idx = 0; ; ++idx ) {
        Server::SyncLogic* logic = that->getSyncLogic(idx);
        if ( ! logic ) break;
        if ( logic == infrastructLogic_.get() ) {
            rv += infrastructLogic_.get()->reportStatistics();
        } else {
            total += static_cast<AbonentLogic*>(logic)->reportStatistics();
        }
    }
    char buf[100];
    cntAbonents_->setValue(total);
    snprintf(buf,sizeof(buf)," abonents=%lu locations=%u storages=%u",total,nodeCfg_.locationsCount,nodeCfg_.storagesCount);
    rv += buf;
    return rv;
}


std::string PvssDispatcher::flushIOStatistics( unsigned scale,
                                               unsigned dt )
{
    std::string rv;
    rv.reserve(200);
    const unsigned d = dt/2;
    unsigned idx = 0;
    for ( std::vector< DiskManager* >::const_iterator i = diskManagers_.begin();
          i != diskManagers_.end();
          ++i ) {
        // unsigned idx = 0; idx < nodeCfg_.disksCount; ++idx ) {
        // DiskFlusher* df = diskFlushers_[idx];
        DiskManager* df = *i;
        if ( !df ) continue;
        unsigned pfget, kbget, pfset, kbset;
        pfget = kbget = pfset = kbset = 0;
        df->flushIOStatistics(pfget,kbget,pfset,kbset);
        char buf[100];
        sprintf(buf," %s:%u(%u)/%u(%u)",df->getName().c_str(),
                unsigned((pfget*scale+d)/dt), unsigned((kbget*scale+d)/dt),
                unsigned((pfset*scale+d)/dt), unsigned((kbset*scale+d)/dt));
        if (idx%4==0 && idx>0) {
            rv.append("\n           ");
        }
        ++idx;
        rv.append(buf);
    }
    if ( infrastructLogic_.get() ) {
        rv.append("\n         infra R/W:");
        infrastructLogic_->flushIOStatistics(rv,scale,dt);
    }
    return rv;
}


void PvssDispatcher::createLogics( bool makedirs, const AbonentStorageConfig& abntcfg,
                                   const InfrastructStorageConfig* infcfg )
{
    for (unsigned locationNumber = 0; locationNumber < nodeCfg_.locationsCount; ++locationNumber) {

        const AbonentStorageConfig::Location& locCfg = abntcfg.locations[locationNumber];
        const unsigned diskNumber = locCfg.disk;

        if (!File::Exists( locCfg.path.c_str() )) {
            if ( makedirs ) {
                smsc_log_debug(logger_, "create storage dir '%s' on disk %d",
                               locCfg.path.c_str(), diskNumber );
                File::MkDir(locCfg.path.c_str());
            } else {
                smsc_log_debug(logger_, "storage dir '%s' creation on disk %d skipped",
                               locCfg.path.c_str(), diskNumber );
                continue;
            }
        }
        DiskManager* dmgr = diskManagers_[diskNumber];
        AbonentLogic* logic = new AbonentLogic( *this,
                                                locationNumber,
                                                abntcfg,
                                                dmgr->getFileManager(),
                                                dmgr->getFlusher() );
        abonentLogics_.Push(logic);
        ++createdLocations_;
    }

    // infrastruct logic
    if (nodeCfg_.nodeNumber == getInfrastructNodeNumber()) {
        if (!infcfg) {
            throw Exception("Error init infrastruct storage, config is NULL");
        }
        infrastructLogic_.reset( new InfrastructLogic(*this,
                                                      *infcfg,
                                                      *infraFlusher_));
    }
}


void PvssDispatcher::init()
{
    // we have to init all logics in parallel
    // smsc::core::threads::ThreadPool tp;
    smsc::core::buffers::Array< LogicTask* > initTasks;
    for ( unsigned i = 0; i < unsigned(abonentLogics_.Count()); ++i ) {
        LogicTask* task = abonentLogics_[i]->startInit();
        if ( task ) initTasks.Push(task);
    }

    if ( infrastructLogic_.get() ) {
        LogicTask* task = infrastructLogic_->startInit();
        if ( task ) initTasks.Push(task);
    }

    // make sure all the tasks are started and finished
    for ( unsigned i = 0; i < unsigned(initTasks.Count()); ++i ) {
        LogicTask* task = initTasks[i];
        task->waitUntilStarted();
        task->waitUntilReleased();
    }

    // checking for results, cleanup
    std::string failure;
    for ( unsigned i = 0; i < unsigned(initTasks.Count()); ++i ) {
        LogicTask* task = initTasks[i];
        std::string fail = task->getFailure();
        if ( !fail.empty() ) {
            if ( ! failure.empty() ) failure.push_back('\n');
            failure.append(fail);
        }
        delete task;
    }

    if ( !failure.empty() ) throw smsc::util::Exception(failure.c_str());

    // starting log roller
    logRoller_->start();

    // starting flushers
    for ( std::vector<DiskManager*>::const_iterator i = diskManagers_.begin();
          i != diskManagers_.end();
          ++i ) {
        (*i)->start();
    }
    if ( infraFlusher_ ) infraFlusher_->start();
    smsc_log_info(logger_,"all storages inited, stats: %s", reportStatistics().c_str());
}


void PvssDispatcher::rebuildIndex( unsigned maxSpeed )
{
    smsc::core::buffers::Array< LogicTask* > rebuildTasks;
    for ( unsigned i = 0; i < unsigned(abonentLogics_.Count()); ++i ) {
        LogicTask* task = abonentLogics_[i]->startRebuildIndex(maxSpeed);
        if (task) rebuildTasks.Push(task);
    }

    if ( infrastructLogic_.get() ) {
        LogicTask* task = infrastructLogic_->startRebuildIndex();
        if (task) rebuildTasks.Push(task);
    }

    // make sure all the tasks are started and finished
    for ( unsigned i = 0; i < unsigned(rebuildTasks.Count()); ++i ) {
        LogicTask* task = rebuildTasks[i];
        task->waitUntilStarted();
        task->waitUntilReleased();
    }

    // checking for results, cleanup
    std::string failure;
    for ( unsigned i = 0; i < unsigned(rebuildTasks.Count()); ++i ) {
        LogicTask* task = rebuildTasks[i];
        std::string fail = task->getFailure();
        if ( !fail.empty() ) {
            if ( ! failure.empty() ) failure.push_back('\n');
            failure.append(fail);
        }
        delete task;
    }
    if ( !failure.empty() ) throw smsc::util::Exception(failure.c_str());
    smsc_log_info(logger_,"index rebuilding is finished");
}


void PvssDispatcher::dumpStorage( int index )
{
    if ( index >= 0 ) {
        for ( unsigned i = 0; i < unsigned(abonentLogics_.Count()); ++i ) {
            abonentLogics_[i]->dumpStorage(index);
        }
    } else if ( infrastructLogic_.get() ) {
        infrastructLogic_->dumpStorage(0);
    }
}


unsigned PvssDispatcher::getLocationNumber(unsigned elementStorageNumber) const {
  return (elementStorageNumber / StorageNumbering::instance().nodes()) % nodeCfg_.locationsCount;
}

AbonentLogic* PvssDispatcher::getLocation(unsigned elementStorageNumber) {
  return abonentLogics_[getLocationNumber(elementStorageNumber)];
}

void PvssDispatcher::shutdown() {
    smsc_log_info(logger_,"shutting down a pvss dispatcher");
    uint16_t created = createdLocations_;

    logRoller_->stop();

    // stopping all disk managers
    for ( std::vector< DiskManager* >::const_iterator i = diskManagers_.begin();
          i != diskManagers_.end();
          ++i ) {
        // dfm.shutdown + df.stop
        (*i)->stop();
    }
    if ( infraFlusher_ ) {
        infraFlusher_->stop();
        delete infraFlusher_;
        infraFlusher_ = 0;
    }

    for (unsigned i = 0; i < created; ++i) {
        delete abonentLogics_[i];
        --createdLocations_;
    }

    for_each(diskManagers_.begin(), diskManagers_.end(), smsc::util::PtrDestroy());
    diskManagers_.clear();
}


PvssDispatcher::~PvssDispatcher() {
    shutdown();
    delete logRoller_; logRoller_ = 0;
    smsc_log_info(logger_,"dtor pvss dispatcher");
}

}//pvss
}//scag2
