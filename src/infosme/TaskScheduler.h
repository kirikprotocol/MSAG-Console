#ifndef SMSC_INFO_SME_TASK_SCHEDULER
#define SMSC_INFO_SME_TASK_SCHEDULER

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <logger/Logger.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include "Task.h"

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    
    using smsc::core::threads::Thread;
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::Logger;
    
    class TaskScheduler : public Thread
    {
    private:

        Hash<Task*> tasks;

        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        
    public:

        /**
         * Creates TaskScheduler
         */
        TaskScheduler();
        virtual ~TaskScheduler();

        virtual int Execute();
        void Start();
        void Stop();
        
        inline bool isStarted() { return bStarted; };

        bool registerTask(Task* task);
        bool unregisterTask(const char* name);
    };

}}

#endif //SMSC_INFO_SME_TASK_SCHEDULER

