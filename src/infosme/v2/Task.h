#ifndef SMSC_INFOSME2_TASK_H
#define SMSC_INFOSME2_TASK_H

#include <list>
#include <map>
#include "util/config/ConfigView.h"
#include "SpeedControl.h"
#include "Message.h"
#include "CsvStore.h"
#include "TaskInfo.h"

namespace smsc {
namespace db { class DataSource; }

namespace infosme2 {

class DataProvider;
class FinalStateSaver;
class TaskProcessor;

class TaskGuard;

class Task
{
    friend class TaskGuard;
private:
    typedef std::list< Message > MessageList;
    typedef std::map<int,MessageList> RegionMap;

public:
    static unsigned stringToTaskId( const char* taskId );

    // static Task* createTask( const TaskProcessor&            taskProcessor,
    // const char*                     taskId,
    // smsc::util::config::ConfigView* taskConfig );

    unsigned getId() const { return id_; }
    const std::string& getName() const { return info_.name; }
    unsigned getPriority() const { return info_.priority; }
    bool isEnabled() const { return info_.enabled; }

    bool isActive() const { return active_; }
    bool hasMessages() const;

    /// method checks if the task is ready to send a message.
    /// @return 0 if task is ready, otherwise the number of msec to sleep.
    inline unsigned isReady( unsigned absTime ) {
        return speedControl_.isReady(absTime);
    }

    /// invoked from processor sequentially
    bool prefetchMessage( time_t now, int regionId );
    /// NOTE: that you may only pick prefetched message once
    const Message& getPrefetched();

    /// setting a message state
    void setMessageState( const Message& msg, uint8_t state );

    /// NOTE: the method should be invoked only when all activity is stopped.
    void update( const TaskInfo& info );

private:
    Task();
    ~Task();
    Task( unsigned              id,
          const std::string&    location,
          const TaskInfo&       info,
          smsc::db::DataSource* taskDs,
          FinalStateSaver*      saver );

    void setInfo( const TaskInfo& info );

    // put message back to cache
    void suspendMessage( const Message& msg );

    void changeUsage( bool v ) {
        MutexGuard mg(lock_);
        if ( v ) {
            ++users_;
        } else {
            if (users_ > 0) --users_;
            if (!users_) delete this;
        }
    }

    Task( const Task& );
    Task& operator = ( const Task& );

private:
    static smsc::logger::Logger* log_;

    unsigned id_;
    const std::string     location_;
    smsc::core::synchronization::Mutex lock_;
    SpeedControl          speedControl_;
    TaskInfo              info_;
    unsigned              users_;
    bool                  active_;
    bool                  prefetched_;
    Message               prefetch_;
    RegionMap             regionMap_;
    CsvStore              store_;
};


class TaskGuard
{
public:
    inline TaskGuard() : task_(0) {}
    inline TaskGuard( Task* task ) : task_(task) {
        if (task_) task_->changeUsage(true);
    }
    inline ~TaskGuard() {
        if (task_) task_->changeUsage(false);
    }
    inline TaskGuard( const TaskGuard& tg ) : task_(tg.task_) {
        if (task_) task_->changeUsage(true);
    }
    inline TaskGuard& operator = ( const TaskGuard& tg ) {
        if ( &tg != this ) {
            task_ = tg.task_;
            if (task_) task_->changeUsage(true);
        }
        return *this;
    }
    inline Task* get() const {
        return task_;
    }
    inline Task* operator -> () const {
        return task_;
    }

    Task* create( unsigned              id,
                  const std::string&    location,
                  const TaskInfo&       info,
                  smsc::db::DataSource* dataSource,
                  FinalStateSaver*      finalStateSaver ) {
        if ( task_ ) { task_->changeUsage(false); task_ = 0; }
        task_ = new Task(id,location,info,dataSource,finalStateSaver);
        task_->changeUsage(true);
        return task_;
    }

private:
    Task* task_;
};


}
}

#endif

