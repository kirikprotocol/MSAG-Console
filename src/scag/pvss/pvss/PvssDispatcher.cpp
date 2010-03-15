#include "PvssDispatcher.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/pvss/data/ProfileKey.h"
// #include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"

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

PvssDispatcher::PvssDispatcher(const NodeConfig& nodeCfg,
                               unsigned creationLimit ):
nodeCfg_(nodeCfg), createdLocations_(0), infrastructIndex_(nodeCfg_.locationsCount),
logger_(Logger::getInstance("pvss.disp"))
{
    StorageNumbering::setInstance(nodeCfg.nodesCount);
    unsigned addSpeed = nodeCfg_.expectedSpeed / nodeCfg_.disksCount;
    for (int i = 0; i < nodeCfg_.disksCount; ++i) {
        dataFileManagers_.push_back(new scag::util::storage::DataFileManager(1, addSpeed,creationLimit));
        char buf[30];
        sprintf(buf,"dflush.%02u",i);
        diskFlushers_.push_back( new scag::util::storage::DiskFlusher(buf) );
    }
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
    snprintf(buf,sizeof(buf)," abonents=%lu locations=%u storages=%u",total,nodeCfg_.locationsCount,nodeCfg_.storagesCount);
    rv += buf;
    return rv;
}


void PvssDispatcher::createLogics( bool makedirs, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg )
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
        AbonentLogic* logic = new AbonentLogic( *this,
                                                locationNumber,
                                                abntcfg,
                                                *dataFileManagers_[diskNumber],
                                                *diskFlushers_[diskNumber]);
        abonentLogics_.Push(logic);
        ++createdLocations_;
    }

    // infrastruct logic
    if (nodeCfg_.nodeNumber == getInfrastructNodeNumber()) {
        if (!infcfg) {
            throw Exception("Error init infrastruct storage, config is NULL");
        }
        infrastructLogic_.reset( new InfrastructLogic(*this,*infcfg) );
    }
}


void PvssDispatcher::init()
{
    // we have to init all logics in parallel
    // smsc::core::threads::ThreadPool tp;
    smsc::core::buffers::Array< LogicTask* > initTasks;
    for ( unsigned i = 0; i < abonentLogics_.Count(); ++i ) {
        LogicTask* task = abonentLogics_[i]->startInit();
        if ( task ) initTasks.Push(task);
        /*
        LogicInitTask* task = new LogicInitTask(abonentLogics_[i]);
        dataFileManagers_[abonentLogics_[i]->getLocationNumber()]
        serverCore->startTask( task, false );
         */
    }

    if ( infrastructLogic_.get() ) {
        LogicTask* task = infrastructLogic_->startInit();
        if ( task ) initTasks.Push(task);
        // serverCore->startSubTask(task, false);
        // infrastructLogic_->init(*infcfg);
    }

    // make sure all the tasks are started and finished
    for ( unsigned i = 0; i < initTasks.Count(); ++i ) {
        LogicTask* task = initTasks[i];
        task->waitUntilStarted();
        task->waitUntilReleased();
    }

    // checking for results, cleanup
    std::string failure;
    for ( unsigned i = 0; i < initTasks.Count(); ++i ) {
        LogicTask* task = initTasks[i];
        std::string fail = task->getFailure();
        if ( !fail.empty() ) {
            if ( ! failure.empty() ) failure.push_back('\n');
            failure.append(fail);
        }
        delete task;
    }

    if ( !failure.empty() ) throw smsc::util::Exception(failure.c_str());

    // starting flushers
    for ( std::vector<scag::util::storage::DiskFlusher*>::const_iterator i = diskFlushers_.begin();
          i != diskFlushers_.end();
          ++i ) {
        (*i)->start(nodeCfg_.maxDirtySpeed);
    }

    smsc_log_info(logger_,"all storages inited, stats: %s", reportStatistics().c_str());
}


void PvssDispatcher::rebuildIndex( unsigned maxSpeed )
{
    smsc::core::buffers::Array< LogicTask* > rebuildTasks;
    for ( unsigned i = 0; i < abonentLogics_.Count(); ++i ) {
        LogicTask* task = abonentLogics_[i]->startRebuildIndex(maxSpeed);
        if (task) rebuildTasks.Push(task);
    }

    if ( infrastructLogic_.get() ) {
        LogicTask* task = infrastructLogic_->startRebuildIndex();
        if (task) rebuildTasks.Push(task);
    }

    // make sure all the tasks are started and finished
    for ( unsigned i = 0; i < rebuildTasks.Count(); ++i ) {
        LogicTask* task = rebuildTasks[i];
        task->waitUntilStarted();
        task->waitUntilReleased();
    }

    // checking for results, cleanup
    std::string failure;
    for ( unsigned i = 0; i < rebuildTasks.Count(); ++i ) {
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
        for ( unsigned i = 0; i < abonentLogics_.Count(); ++i ) {
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

    // stopping all file managers
    for ( std::vector< DataFileManager* >::iterator i = dataFileManagers_.begin();
          i != dataFileManagers_.end();
          ++i ) {
        if ( *i ) {
            (*i)->shutdown();
        }
    }

    // stopping all disk flushers
    for ( std::vector< DiskFlusher* >::const_iterator i = diskFlushers_.begin();
          i != diskFlushers_.end();
          ++i ) {
        (*i)->stop();
        delete *i;
    }
    diskFlushers_.clear();

  for (unsigned i = 0; i < created; ++i) {
    delete abonentLogics_[i];
    --createdLocations_;
  }
  for (unsigned i = 0; i < dataFileManagers_.size(); ++i) {
    if (dataFileManagers_[i]) {
      delete dataFileManagers_[i];
    }
  }
  dataFileManagers_.clear();
}

PvssDispatcher::~PvssDispatcher() {
  shutdown();
    smsc_log_info(logger_,"dtor pvss dispatcher");
}


}//pvss
}//scag2
