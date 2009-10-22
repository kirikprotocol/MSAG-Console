#include "ProtoException.h"
#include "TaskDispatcher.h"
#include "Connector.h"

namespace {

using namespace scag2::prototypes::infosme;

struct MessageGuard {

    MessageGuard(Task& t, Message& m) : task(t), msg(m), res(MessageState::LIMITED) {}
    ~MessageGuard() {
        switch(res) {
        case MessageState::OK : task.finalizeMessage(msg,res); break;
        case MessageState::FAIL : task.finalizeMessage(msg,res); break;
        default: task.suspendMessage(msg); break;
        }
    }

    inline void setRes( int rc ) { res = rc; }

    Task&    task;
    Message& msg;
    int      res;
};

}


namespace scag2 {
namespace prototypes {
namespace infosme {

TaskDispatcher::TaskDispatcher() :
log_(smsc::logger::Logger::getInstance("dispatch")),
currentConn_(0),
currentList_(0)
{}


TaskDispatcher::~TaskDispatcher() 
{}


void TaskDispatcher::addConnector( Connector& c )
{
    MutexGuard mg(lock_);
    currentList_ = ( taskMap_.empty() ? 0 : &taskMap_.begin()->second );
    TaskMap::iterator iter = 
        taskMap_.insert(std::make_pair(c.getId(),
                                       TaskMap::mapped_type(*this))).first;
    if ( currentList_ ) {
        TaskMap::mapped_type& list = iter->second;
        for ( size_t i = 0; i < currentList_->size(); ++i ) {
            list.add( (*currentList_)[i] );
        }
    }
}


void TaskDispatcher::addTask( Task& t )
{
    MutexGuard mg(lock_);
    for ( TaskMap::iterator i = taskMap_.begin(); i != taskMap_.end(); ++i ) {
        i->second.add( &t );
    }
}


unsigned TaskDispatcher::processConnector( unsigned deltaTime, Connector& c )
{
    const unsigned cid = c.getId();
    smsc_log_debug(log_,"processConnector(%u,c.id=%u)",deltaTime,cid);
    MutexGuard mg(lock_);
    TaskMap::iterator i = taskMap_.find(cid);
    if ( i == taskMap_.end() ) {
        smsc_log_debug(log_,"something wrong: task list not found");
        return 1000;
    }
    currentConn_ = &c;
    currentList_ = &i->second;
    now_ = time(0);
    if ( log_->isDebugEnabled() ) {
        std::string s;
        s.reserve(400);
        currentList_->dump(s);
        smsc_log_debug(log_,"cid=%u tasks (%u) are:%s",
                       cid,
                       unsigned(currentList_->size()),s.c_str());
    }
    return currentList_->processOnce(deltaTime,500);
}


unsigned TaskDispatcher::scoredObjIsReady( unsigned deltaTime, Task& task )
{
    if ( ! task.isActive() ||
         ! task.hasMessages() ||
         ! task.prefetchMessage(now_,currentConn_->getId()) ) {
        // task is not ready
        return 1000;
    }
    return task.wantToSleep( deltaTime );
}


int TaskDispatcher::processScoredObj( unsigned deltaTime, Task& task )
{
    Message msg;
    try {

        if ( ! task.getMessage(now_,currentConn_->getId(),msg) ) {
            // task has no messages for this region, drop it to the bottom of the list
            return -1000;
        }

        MessageGuard mg(task,msg);
        int rc = currentConn_->send( deltaTime, msg );
        mg.setRes(rc);
        if ( rc != MessageState::OK ) {
            return -1000;
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"processObj, exc: %s", e.what());
        return -1000;
    }
    return 1000/task.getPriority();
}


void TaskDispatcher::scoredObjToString( std::string& s, Task& task )
{
    s.append(task.toString());
}

}
}
}
