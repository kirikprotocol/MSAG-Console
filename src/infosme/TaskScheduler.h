#ifndef SMSC_INFO_SME_TASK_SCHEDULER
#define SMSC_INFO_SME_TASK_SCHEDULER

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <logger/Logger.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/IntHash.hpp>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include "Task.h"
#include "Schedules.h"

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    
    using smsc::core::threads::Thread;
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::Logger;
    
    class TaskProcessor;
    class TaskScheduler : public Thread
    {
    private:

        TaskProcessor* processor;

        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;

        IntHash<Schedule*>  schedules;
        Mutex               schedulesLock; 
        
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
        void init(TaskProcessor* processor, ConfigView* config);

        virtual int Execute();
        void Start();
        void Stop();
        
        /**
         * Adds task schedule into scheduling plan and reactivates scheduler.
         *
         * @param schedule      shedule for task(s)
         */
        void addSchedule(Schedule* schedule);

        /**
         * Changes task schedule in scheduling plan and reactivates scheduler.
         * If task wasn't scheduled returns false, else returns true.
         *
         * @param scheduleId    id of old shedule for task
         * @param schedule      new shedule for task
         * @return false if task wasn't scheduled, else returns true.
         */
        bool changeSchedule(int scheduleId, Schedule* schedule);
        
        /**
         * Removes task schedule from scheduling plan.
         * If task wasn't scheduled returns false, else returns true.
         *
         * @param taskName      task name to be removed
         * @return false if task wasn't scheduled, else returns true.
         */
        bool removeSchedule(int scheduleId);
    };

}}

#endif //SMSC_INFO_SME_TASK_SCHEDULER

