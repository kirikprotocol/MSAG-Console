#ifndef _SCAG_PROTOTYPES_INFOSME_TASKDISPATCHER_H
#define _SCAG_PROTOTYPES_INFOSME_TASKDISPATCHER_H

#include "Task.h"
#include "logger/Logger.h"
#include "ScoredList.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Region;

class TaskDispatcher
{
private:
    friend class ScoredList< TaskDispatcher >;

    typedef std::map< unsigned, ScoredList< TaskDispatcher > > TaskMap;

public:
    typedef std::list< Task* > TaskList;

    TaskDispatcher();
    ~TaskDispatcher();

    void addRegion( Region& c );
    void delRegion( Region& c );

    void addTask( Task& t );
    void delTask( Task& t );

    bool hasInactiveTask() {
        return hasInactiveTask_;
    }

    // should be invoked from processor
    TaskList collectInactiveTasks();

    // >0 sleeping
    // =0 success
    unsigned processRegion( unsigned deltaTime, Region& c );

private:
    // to satisfy scoredlist contract
    typedef Task ScoredObjType;
    unsigned scoredObjIsReady( unsigned deltaTime, Task& t );
    int processScoredObj( unsigned deltaTime, Task& t );
    void scoredObjToString( std::string& s, Task& t );

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    TaskMap                            taskMap_;
    // these are cached in processRegion call
    time_t                             now_;
    Region*                            currentConn_;
    ScoredList< TaskDispatcher >*      currentList_;
    bool                               hasInactiveTask_;
};

}
}
}

#endif
