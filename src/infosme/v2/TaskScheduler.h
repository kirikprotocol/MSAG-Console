#ifndef SMSC_INFO_SME_TASK_SCHEDULER
#define SMSC_INFO_SME_TASK_SCHEDULER

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "logger/Logger.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "Task.h"
#include "Schedules.h"
#include "InfoSmeAdmin.h"

namespace smsc {
namespace infosme {

using namespace smsc::core::buffers;
    
using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;
    
class TaskScheduler : public Thread
{
public:

    /**
     * Creates TaskScheduler
     */
    TaskScheduler();
    virtual ~TaskScheduler();

    /**
     * Initializes TaskScheduler, loads up all specified tasks schedules 
     *
     * @param config
     * @exception ConfigException throws when configuration is invalid
     */
    void init(TaskProcessorAdapter* processor, ConfigView* config);

    virtual int Execute();
    void Start();
    void Start(int) { Start(); }
    void Stop();

    inline bool isStarted() { 
        MutexGuard guard(startLock);
        return bStarted;
    };
        
    /**
     * Adds schedule into scheduling plan and reactivates scheduler.
     *
     * @param schedule      shedule for task(s)
     * @return false if schedule already defined, else returns true.
     */
    bool addSchedule(Schedule* schedule);

    /**
     * Changes schedule in scheduling plan and reactivates scheduler.
     * If schedule not found returns false, else returns true.
     *
     * @param id            shedule id 
     * @param schedule      new shedule
     * @return false if schedule not found, else returns true.
     */
    bool changeSchedule(std::string id, Schedule* schedule);
        
    /**
     * Removes task(s) schedule from scheduling plan.
     * If schedule not found returns false, else returns true.
     * 
     * @param id            schedule id
     * @return false if schedule not found, else returns true.
     */ 
    bool removeSchedule(std::string id);

    /**
     * Removes task from all registered schedules
     *
     * @param taskId        task id to be removed
     */
    void removeTask(uint32_t taskId);

private:

    Logger  *logger;

    TaskProcessorAdapter*   processor;
    Hash<Schedule*>         schedules;
    Mutex                   schedulesLock; 
        
    Event   awake, exited;
    bool    bStarted, bNeedExit, bChanged;
    Mutex   startLock;

    Schedule* getNextSchedule(time_t& scheduleTime);

};

}
}

#endif //SMSC_INFO_SME_TASK_SCHEDULER

