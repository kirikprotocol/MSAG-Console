#ifndef _SCAG_PROTOTYPES_INFOSME_TASKDISPATCHER_H
#define _SCAG_PROTOTYPES_INFOSME_TASKDISPATCHER_H

#include "Task.h"
#include "logger/Logger.h"
#include "ScoredList.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Connector;

class TaskDispatcher
{
private:
    friend class ScoredList< TaskDispatcher >;

    typedef std::map< unsigned, ScoredList< TaskDispatcher > > TaskMap;

public:
    TaskDispatcher();
    ~TaskDispatcher();

    void addConnector( Connector& c );
    void delConnector( Connector& c );

    void addTask( Task& t );
    void delTask( Task& t );

    // >0 sleeping
    // =0 success
    unsigned processConnector( unsigned deltaTime, Connector& c );

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
    // these are cached in processConnector call
    time_t                             now_;
    Connector*                         currentConn_;
    ScoredList< TaskDispatcher >*      currentList_;
};

}
}
}

#endif
