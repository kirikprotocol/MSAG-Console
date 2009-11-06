#ifndef _SMSC_INFOSME2_TASKDISPATCHER_H
#define _SMSC_INFOSME2_TASKDISPATCHER_H

#include <map>

#include "logger/Logger.h"
#include "Task.h"
#include "ScoredList.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace infosme2 {

class RegionSender;

class TaskDispatcher
{
    friend class ScoredList< TaskDispatcher >;

public:
    TaskDispatcher();
    ~TaskDispatcher();

    unsigned processRegion( unsigned curTime, RegionSender& s );
    void addRegion( int regionId );
    void delRegion( int regionId );
    void addTask( Task& task );
    void delTask( Task& task );

private:
    TaskDispatcher( const TaskDispatcher& );
    TaskDispatcher& operator = ( const TaskDispatcher& );

private:
    typedef Task ScoredObjType;
    typedef std::map< int, ScoredList< TaskDispatcher > > TaskMap;

    unsigned scoredObjIsReady( unsigned curTime, ScoredObjType& task );
    int processScoredObj( unsigned curTime, ScoredObjType& task );

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    TaskMap                            taskMap_;

    // cached values set in processRegion
    time_t                             now_;
    RegionSender*                      currentSender_;
    ScoredList< TaskDispatcher >*      currentList_;
    bool                               hasInactiveTask_;
};

}
}

#endif
