#include "TaskDispatcher.h"
#include "RegionSender.h"
#include "Message.h"
#include "Types.h"

namespace {

using namespace smsc::infosme2;
struct MessageGuard
{
    MessageGuard(Task& t, Message& m) : task(t), msg(m), res(MessageState::RETRY) {}
    ~MessageGuard() {
        task.setMessageState(msg,res);
    }
    void setState(uint8_t r) {
        res = r;
    }

    Task&    task;
    Message& msg;
    uint8_t  res;
};

}


namespace smsc {
namespace infosme2 {

TaskDispatcher::TaskDispatcher() :
log_(smsc::logger::Logger::getInstance("is2.dispch"))
{
    smsc_log_debug(log_,"ctor");
}


TaskDispatcher::~TaskDispatcher()
{
    smsc_log_debug(log_,"dtor");
}


unsigned TaskDispatcher::processRegion( unsigned curTime, RegionSender& c )
{
    const int cid = c.getId();
    smsc_log_debug(log_,"processRegion(tm=%u,id=%d)",curTime,cid);
    MutexGuard mg(lock_);
    TaskMap::iterator i = taskMap_.find(cid);
    if ( i == taskMap_.end() ) {
        smsc_log_debug(log_,"logic problem: tasklist not found for region '%d'", cid);
        return 1000;
    }
    currentSender_ = &c;
    currentList_ = &i->second;
    now_ = time(0);
    return currentList_->processOnce(curTime,1000);
}


void TaskDispatcher::addRegion( int regionId )
{
    MutexGuard mg(lock_);
    TaskMap::iterator iter = taskMap_.lower_bound(regionId);
    if ( iter == taskMap_.end() || iter->first != regionId ) {
        if ( ! taskMap_.empty() ) {
            iter = taskMap_.insert( iter,
                                    TaskMap::value_type
                                    (regionId, taskMap_.begin()->second.copy()) );
            smsc_log_debug(log_,"registering region %d (with %u tasks)",
                           regionId,iter->second.size());
        } else {
            iter = taskMap_.insert( iter,
                                    TaskMap::value_type
                                    (regionId, TaskMap::mapped_type(*this,5000,log_) ));
            smsc_log_debug(log_,"registering region %d",regionId);
        }
    } else {
        smsc_log_warn(log_,"region %d is already registered",regionId);
    }
}


void TaskDispatcher::delRegion( int regionId )
{
    MutexGuard mg(lock_);
    TaskMap::iterator iter = taskMap_.find(regionId);
    if ( iter != taskMap_.end() ) {
        taskMap_.erase(iter);
        smsc_log_debug(log_,"removing region %d",regionId);
    } else {
        smsc_log_warn(log_,"region %d is not found",regionId);
    }
}


unsigned TaskDispatcher::scoredObjIsReady( unsigned deltaTime, ScoredObjType& task )
{
    if ( ! task.isActive() ) {
        hasInactiveTask_ = true;
        return 1000;
    }

    const unsigned wantToSleep = task.isReady( deltaTime );
    if ( wantToSleep > 0 ) return wantToSleep;

    if ( ! task.prefetchMessage(now_,currentSender_->getId()) ) {
        // this region is not ready in task
        return 1000;
    }
    return 0; // task is ready
}


int TaskDispatcher::processScoredObj( unsigned curTime, ScoredObjType& task )
{
    try {

        Message msg = task.getPrefetched();

        MessageGuard mg(task,msg);
        uint8_t rc = currentSender_->send( curTime, task, msg );
        mg.setState(rc);
        if ( rc != MessageState::ENROUTE ) {
            return -1000;
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"processObj, exc: %s", e.what());
        return -1000;
    }
    return 1000/task.getPriority();
}

}
}
