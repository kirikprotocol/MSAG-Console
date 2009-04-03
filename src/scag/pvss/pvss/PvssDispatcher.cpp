#include "PvssDispatcher.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/pvss/data/ProfileKey.h"
// #include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"
#include "scag/pvss/api/core/server/ServerCore.h"

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


class LogicInitTask : public scag2::util::WatchedThreadedTask
{
public:
    LogicInitTask( PvssLogic* logic ) : logic_(logic) {}

    virtual const char* taskName() { return "pvss.init"; }

    virtual int doExecute() {
        try {
            logic_->init();
        } catch ( std::exception& e ) {
            setFailure( e.what() );
        } catch (...) {
            setFailure( "unknown exception" );
        }
        return 0;
    }

    const std::string& getFailure() const { return failure_; }
    
private:
    void setFailure( const char* what ) {
        failure_ = std::string(what) + " in " + logic_->toString();
    }

private:
    PvssLogic*   logic_;
    std::string  failure_;
};


}


namespace scag2 {
namespace pvss  {

using scag::util::storage::StorageNumbering;

PvssDispatcher::PvssDispatcher(const NodeConfig& nodeCfg): nodeCfg_(nodeCfg), createdLocations_(0), infrastructIndex_(nodeCfg_.storagesCount),
                                                           logger_(Logger::getInstance("pvss.disp")){
  StorageNumbering::setInstance(nodeCfg.nodesCount);
  unsigned addSpeed = nodeCfg_.expectedSpeed / nodeCfg_.disksCount;
  for (int i = 0; i < nodeCfg_.disksCount; ++i) {
    dataFileManagers_.push_back(new scag::util::storage::DataFileManager(1, addSpeed));    
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


void PvssDispatcher::init( core::server::ServerCore* serverCore, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg )
{

  for (unsigned locationNumber = 0; locationNumber < nodeCfg_.locationsCount; ++locationNumber) {
    if (!File::Exists(abntcfg.locations[locationNumber].path.c_str())) {
      smsc_log_debug(logger_, "create storage dir '%s' on disk %d", abntcfg.locations[locationNumber].path.c_str(),
                     abntcfg.locations[locationNumber].disk);
      File::MkDir(abntcfg.locations[locationNumber].path.c_str());
    }
    AbonentLogic* logic = new AbonentLogic( *this,
                                            locationNumber,
                                            abntcfg, *dataFileManagers_[abntcfg.locations[locationNumber].disk] );
    abonentLogics_.Push(logic);
    ++createdLocations_;
  }

    // infrastruct logic
    if (nodeCfg_.nodeNumber == getInfrastructNodeNumber()) {
        if (!infcfg) {
            throw Exception("Error init infrastruct storage, config in NULL");
        }
        infrastructLogic_.reset( new InfrastructLogic(*this,*infcfg) );
    }

    // we have to init all logics in parallel
    // smsc::core::threads::ThreadPool tp;
    smsc::core::buffers::Array< LogicInitTask* > initTasks;
    for ( unsigned i = 0; i < abonentLogics_.Count(); ++i ) {
        LogicInitTask* task = new LogicInitTask(abonentLogics_[i]);
        initTasks.Push(task);
        serverCore->startSubTask(task, false);
    }

    if ( infrastructLogic_.get() ) {
        LogicInitTask* task = new LogicInitTask(infrastructLogic_.get());
        initTasks.Push(task);
        serverCore->startSubTask(task, false);
        // infrastructLogic_->init(*infcfg);
    }

    // make sure all the tasks are started and finished
    for ( unsigned i = 0; i < initTasks.Count(); ++i ) {
        LogicInitTask* task = initTasks[i];
        task->waitUntilStarted();
        task->waitUntilReleased();
    }

    // checking for results, cleanup
    std::string failure;
    for ( unsigned i = 0; i < initTasks.Count(); ++i ) {
        LogicInitTask* task = initTasks[i];
        std::string fail = task->getFailure();
        if ( !fail.empty() ) failure = fail;
        delete task;
    }

    if ( !failure.empty() ) throw smsc::util::Exception(failure.c_str());
    reportStatistics();
}

unsigned PvssDispatcher::getLocationNumber(unsigned elementStorageNumber) const {
  return (elementStorageNumber / StorageNumbering::instance().nodes()) % nodeCfg_.locationsCount;
}

AbonentLogic* PvssDispatcher::getLocation(unsigned elementStorageNumber) {
  return abonentLogics_[getLocationNumber(elementStorageNumber)];
}

void PvssDispatcher::shutdown() {
  uint16_t created = createdLocations_;
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
}


}//pvss
}//scag2
