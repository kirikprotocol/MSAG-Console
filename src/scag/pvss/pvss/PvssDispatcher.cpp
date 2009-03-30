#include "PvssDispatcher.h"
#include "scag/util/storage/StorageNumbering.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"
#include "scag/pvss/api/core/server/ServerCore.h"

namespace {

class LogicInitTask : public scag2::util::WatchedThreadedTask
{
public:
    LogicInitTask( scag2::pvss::PvssLogic* logic ) : logic_(logic) {}

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
    scag2::pvss::PvssLogic* logic_;
    std::string             failure_;
};


}


namespace scag2 {
namespace pvss  {

using scag::util::storage::StorageNumbering;

PvssDispatcher::PvssDispatcher(const NodeConfig& nodeCfg): nodeNumber_(nodeCfg.nodeNumber), locationsCount_(nodeCfg.locationsCount),
                                                           storagesCount_(nodeCfg.storagesCount), createdLocations_(0), infrastructIndex_(storagesCount_),
                                                           logger_(Logger::getInstance("pvss.disp")), dataFileManager_(4){
  StorageNumbering::setInstance(nodeCfg.nodesCount);
}

unsigned PvssDispatcher::getIndex(Request& request) const 
{
  AbstractProfileRequest& profileRequest = static_cast<AbstractProfileRequest&>(request);
  const ProfileKey& profileKey = profileRequest.getProfileKey();

  if (profileKey.hasOperatorKey() || profileKey.hasProviderKey() || profileKey.hasServiceKey()) {
    if (nodeNumber_ != getInfrastructNodeNumber()) {
      smsc_log_warn(logger_, "can't process infrastruct request on node number=%d", nodeNumber_);
      return getErrorIndex();
    }
    smsc_log_debug(logger_, "give %p packet to infrastruct storage node %d", &request, getInfrastructNodeNumber());
    return infrastructIndex_;
  }

  unsigned storageNumber = static_cast<unsigned>(profileKey.getAddress().getNumber() % storagesCount_);
  smsc_log_debug(logger_, "give %p packet to storage %d in node %d", &request, storageNumber, nodeNumber_);
  return getLocationNumber( storageNumber );
}

Server::SyncLogic* PvssDispatcher::getSyncLogic(unsigned idx) {
  if (getErrorIndex() == idx || idx > locationsCount_) {
    return 0;
  }
  if (idx == infrastructIndex_) {
    return infrastructLogic_.get();
  }
  //return getLocation(idx);
  return abonentLogics_[idx];
}


void PvssDispatcher::reportStatistics() const
{
    unsigned long total = 0;
    Server::SyncLogic* logic = 0;
    PvssDispatcher* that = const_cast<PvssDispatcher*>(this);
    for ( unsigned idx = 0; (logic = that->getSyncLogic(idx)) != 0; ++idx ) {
        total += logic->reportStatistics();
    }
    smsc_log_info(logger_,"abonent logics have %lu profiles", total );
}


void PvssDispatcher::init( core::server::ServerCore* serverCore, const AbonentStorageConfig& abntcfg, const InfrastructStorageConfig* infcfg ) throw (smsc::util::Exception)
{

  for (unsigned locationNumber = 0; locationNumber < locationsCount_; ++locationNumber) {
    if (!File::Exists(abntcfg.locationPath[locationNumber].c_str())) {
      smsc_log_debug(logger_, "create storage dir '%s'", abntcfg.locationPath[locationNumber].c_str());
      File::MkDir(abntcfg.locationPath[locationNumber].c_str());
    }
    AbonentLogic* logic = new AbonentLogic( *this,
                                            locationNumber,
                                            abntcfg, dataFileManager_ );
    abonentLogics_.Push(logic);
    ++createdLocations_;
  }

    // infrastruct logic
    if (nodeNumber_ == getInfrastructNodeNumber()) {
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
}

unsigned PvssDispatcher::getLocationNumber(unsigned elementStorageNumber) const {
  return (elementStorageNumber / StorageNumbering::instance().nodes()) % locationsCount_;
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
}

PvssDispatcher::~PvssDispatcher() {
  shutdown();
}


}//pvss
}//scag2
