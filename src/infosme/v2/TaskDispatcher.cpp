#include "TaskDispatcher.h"
#include "RegionSender.h"
#include "TaskTypes.hpp"

namespace {

using namespace smsc::infosme;
struct isTaskInactive
{
    bool operator () ( const Task* t ) const {
        return ! t->isActive();
    }
};

}


namespace smsc {
namespace infosme {

TaskDispatcher::TaskDispatcher( unsigned sleepTime ) :
log_(smsc::logger::Logger::getInstance("is2.dispch")),
sleepTime_(sleepTime)
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
        return sleepTime_;
    }
    currentList_ = &i->second;
    if ( currentList_->size() > 0 ) {
        currentSender_ = &c;
        now_ = time(0);
        if ( log_->isDebugEnabled() ) {
            std::string dump;
            currentList_->dump(dump);
            smsc_log_debug(log_,"%s",dump.c_str());
        }
        return currentList_->processOnce(curTime,sleepTime_);
    }
    return sleepTime_;
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


void TaskDispatcher::addTask( Task& task )
{
    MutexGuard mg(lock_);
    TaskSet::iterator iter = taskSet_.lower_bound(&task);
    if ( iter == taskSet_.end() || *iter != &task ) {
        taskSet_.insert( iter, &task );
        for ( TaskMap::iterator i = taskMap_.begin(); i != taskMap_.end(); ++i ) {
            i->second.add( &task );
        }
    }
}


void TaskDispatcher::delTask( Task& task )
{
    MutexGuard mg(lock_);
    TaskSet::iterator iter = taskSet_.find(&task);
    if ( iter == taskSet_.end() ) return;
    ScoredList< TaskDispatcher >::isEqual functor(&task);
    for ( TaskMap::iterator i = taskMap_.begin(); i != taskMap_.end(); ++i ) {
        i->second.remove(functor);
    }
    taskSet_.erase(iter);
}


void TaskDispatcher::removeInactiveTasks()
{
    typedef std::vector< TaskSet::iterator > DelVector;
    DelVector toDel;
    isTaskInactive functor;
    MutexGuard mg(lock_);
    for ( TaskSet::iterator i = taskSet_.begin(); i != taskSet_.end(); ++i ) {
        Task* task = *i;
        if ( !task ) {
            toDel.push_back(i);
        } else if ( functor(task) ) {
            smsc_log_debug(log_,"removing inactive task %u/'%s'",task->getId(),task->getName().c_str());
            for ( TaskMap::iterator iter = taskMap_.begin();
                  iter != taskMap_.end(); ++iter ) {
                iter->second.remove( ScoredList< TaskDispatcher >::isEqual(task) );
            }
            task->closeProcessedFiles();
            toDel.push_back(i);
        }
    }
    for ( DelVector::const_iterator i = toDel.begin(); i != toDel.end(); ++i ) {
        taskSet_.erase(*i);
    }
}


unsigned TaskDispatcher::scoredObjIsReady( unsigned deltaTime, ScoredObjType& task )
{
    if ( ! task.isActive() ) {
        hasInactiveTask_ = true;
        return sleepTime_;
    }

    const unsigned wantToSleep = task.isReady( deltaTime );
    if ( wantToSleep > 0 ) return wantToSleep;

    if ( ! task.prefetchMessage(now_,currentSender_->getId()) ) {
        // this region is not ready in task
        return sleepTime_;
    }
    return 0; // task is ready
}


int TaskDispatcher::processScoredObj( unsigned curTime, ScoredObjType& task )
{
    int nchunks;
    try {

        Message msg;
        task.getPrefetched( msg );
        // MessageGuard mg(task,msg);
        nchunks = currentSender_->send(curTime, task, msg);
        if ( nchunks <= 0 ) {
            return -1000;
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"processObj, exc: %s", e.what());
        return -1000;
    }
    return 1000*nchunks/task.getPriority();
}

}
}
