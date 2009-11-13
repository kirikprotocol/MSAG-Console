#ifndef _SMSC_INFOSME2_TASKDISPATCHER_H
#define _SMSC_INFOSME2_TASKDISPATCHER_H

#include <map>
#include <set>
#include <vector>

#include "logger/Logger.h"
#include "Task.h"
#include "ScoredList.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace infosme {

class RegionSender;

class TaskDispatcher
{
    friend class ScoredList< TaskDispatcher >;
    typedef std::set< Task* > TaskSet;
public:
    typedef std::vector< Task* >  TaskList;

public:
    TaskDispatcher( unsigned sleepTime );
    ~TaskDispatcher();

    unsigned sleepTime() const { return sleepTime_; }
    unsigned processRegion( unsigned curTime, RegionSender& s );
    void addRegion( int regionId );
    void delRegion( int regionId );
    void addTask( Task& task );
    void delTask( Task& task );
    void removeInactiveTasks();

private:
    TaskDispatcher( const TaskDispatcher& );
    TaskDispatcher& operator = ( const TaskDispatcher& );

private:
    typedef Task ScoredObjType;
    typedef std::map< int, ScoredList< TaskDispatcher > > TaskMap;

    unsigned scoredObjIsReady( unsigned curTime, ScoredObjType& task );
    int processScoredObj( unsigned curTime, ScoredObjType& task );
    void scoredObjToString( std::string& s, ScoredObjType& task ) {
        s += task.toString();
    }

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    TaskMap                            taskMap_;
    TaskSet                            taskSet_; // the set of all tasks
    unsigned                           sleepTime_;

    // cached values set in processRegion
    time_t                             now_;
    RegionSender*                      currentSender_;
    ScoredList< TaskDispatcher >*      currentList_;
    bool                               hasInactiveTask_;
};

}
}

#endif
