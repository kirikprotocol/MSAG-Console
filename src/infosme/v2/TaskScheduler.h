#ifndef SMSC_INFOSME2_TASKSCHEDULER
#define SMSC_INFOSME2_TASKSCHEDULER

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/ConfigView.h"

// #include "core/buffers/Array.hpp"
// #include "core/buffers/IntHash.hpp"
// #include "core/synchronization/Mutex.hpp"
// #include "Task.h"
// #include "Schedules.h"

namespace smsc {
namespace infosme2 {

struct TaskProcessorAdapter
{
    /*
    virtual bool invokeEndGeneration(Task* task)  = 0;
    virtual bool invokeBeginGeneration(Task* task) = 0;
    virtual bool invokeDropAllMessages(Task* task) = 0;
        
    virtual bool hasTask(uint32_t taskId) = 0;
    virtual TaskGuard getTask(uint32_t taskId) = 0;
    virtual void awakeSignal() = 0;
     */
protected:
    virtual ~TaskProcessorAdapter() {}
    TaskProcessorAdapter() {}
    TaskProcessorAdapter( const TaskProcessorAdapter& );
    TaskProcessorAdapter& operator = ( const TaskProcessorAdapter& );
};
    

class TaskScheduler : protected smsc::core::threads::Thread
{
public:

    /**
     * Creates TaskScheduler
     */
    TaskScheduler( TaskProcessorAdapter& adapter );
    virtual ~TaskScheduler();

    /**
     * Initializes TaskScheduler, loads up all specified tasks schedules 
     *
     * @param config
     * @exception ConfigException throws when configuration is invalid
     */
    void init( smsc::util::config::ConfigView* config );

    void start();
    // send stop signal and wait until stopped
    void stop();

    inline bool isStarted() { 
        MutexGuard guard(startMon_);
        return started_;
    };
        
    /**
     * Adds schedule into scheduling plan and reactivates scheduler.
     *
     * @param schedule      shedule for task(s)
     * @return false if schedule already defined, else returns true.
     */
    // bool addSchedule(Schedule* schedule);

    /**
     * Changes schedule in scheduling plan and reactivates scheduler.
     * If schedule not found returns false, else returns true.
     *
     * @param id            shedule id 
     * @param schedule      new shedule
     * @return false if schedule not found, else returns true.
     */
    // bool changeSchedule(std::string id, Schedule* schedule);
        
    /**
     * Removes task(s) schedule from scheduling plan.
     * If schedule not found returns false, else returns true.
     * 
     * @param id            schedule id
     * @return false if schedule not found, else returns true.
     */ 
    // bool removeSchedule(std::string id);

    /**
     * Removes task from all registered schedules
     *
     * @param taskId        task id to be removed
     */
    // void removeTask(uint32_t taskId);

protected:
    virtual int Execute();

private:
    smsc::logger::Logger*                log_;
    TaskProcessorAdapter*                processor_;
    // smsc::core::buffers::Hash<Schedule*> schedules_;
    // smsc::core::synchronization::Mutex   schedulesLock_;
        
    // Event   awake, exited;
    // bool    bStarted, bNeedExit, bChanged;
    smsc::core::synchronization::EventMonitor startMon_;
    bool                                      started_;
    bool                                      stopping_;

    // Schedule* getNextSchedule(time_t& scheduleTime);
};

}
}

#endif // SMSC_INFOSME2_TASKSCHEDULER
