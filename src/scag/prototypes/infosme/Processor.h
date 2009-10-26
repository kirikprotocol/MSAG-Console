#ifndef _SCAG_PROTOTYPES_INFOSME_PROCESSOR_H
#define _SCAG_PROTOTYPES_INFOSME_PROCESSOR_H

#include "Task.h"
#include "logger/Logger.h"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/util/Time.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Sender;
class TaskDispatcher;

class Processor : public util::WatchedThreadedTask
{
public:
    Processor( Sender& sender, TaskDispatcher& disp );
    virtual ~Processor();

    TaskGuard getTask( unsigned idx );
    void addTask( Task* task );
    void notify();
    void setTaskActive( unsigned idx, bool active );

protected:
    virtual const char* taskName() { return "processor"; }
    virtual int doExecute();
    void dumpStatistics( unsigned liveTime );

    // process inactive tasks
    // NOTE: must be locked
    void processInactiveTasks();

private:
    // typedef std::vector< Task* > ActiveTaskList;
    typedef std::list< Task* >     TaskList;
    typedef std::list< TaskGuard > GuardedTaskList;
    typedef std::map< unsigned, GuardedTaskList::iterator > TaskMap;

private:
    smsc::logger::Logger*     log_;
    Sender*                   sender_; // not owned
    TaskDispatcher*           dispatcher_; // owned

    GuardedTaskList           allTasks_; // all tasks
    TaskMap                   taskMap_;  // mapping from id
    bool                      notified_;
    TaskList                  inactiveTasks_;
};

}
}
}

#endif
