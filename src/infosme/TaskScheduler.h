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
    
    struct TaskProcessorAdapter
    {
        virtual bool invokeEndProcess(Task* task)  = 0;
        virtual bool invokeBeginProcess(Task* task) = 0;
        virtual bool invokeDropAllMessages(Task* task) = 0;
        
        virtual bool hasTask(std::string taskId) = 0;
        virtual TaskGuard getTask(std::string taskId) = 0;
    
        virtual ~TaskProcessorAdapter() {};

     protected:

         friend class EventRunner;
         virtual void processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                      std::string smscId="", bool internal=false) = 0;
         virtual void processReceipt (std::string smscId, 
                                      bool delivered, bool retry, bool internal=false) = 0;

         TaskProcessorAdapter() {};
    };
    
    class TaskScheduler : public Thread
    {
    private:

        log4cpp::Category  &logger;

        TaskProcessorAdapter*   processor;
        Hash<Schedule*>         schedules;
        Mutex                   schedulesLock; 
        
        Event   awake, exited;
        bool    bStarted, bNeedExit, bChanged;
        Mutex   startLock;


        Schedule* getNextSchedule(time_t& scheduleTime);
        
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
         * Changes task(s) schedule in scheduling plan and reactivates scheduler.
         * If schedule not found returns false, else returns true.
         *
         * @param id            id of old shedule
         * @param schedule      new shedule
         * @return false if schedule not found, else returns true.
         */
        bool changeSchedule(std::string id, Schedule* schedule);
        
        /**
         * Removes task schedule from scheduling plan.
         * If schedule not found returns false, else returns true.
         * 
         * @param id            schedule id
         * @return false if schedule not found, else returns true.
         */
        bool removeSchedule(std::string id);
    };

}}

#endif //SMSC_INFO_SME_TASK_SCHEDULER

