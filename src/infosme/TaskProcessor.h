#ifndef SMSC_INFO_SME_TASK_PROCESSOR
#define SMSC_INFO_SME_TASK_PROCESSOR

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <logger/Logger.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <db/DataSource.h>

#include "DataProvider.h"
#include "TaskScheduler.h"
#include "Task.h"

//#include "InfoSmeExceptions.h"

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::core::threads::Thread;
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    
    class TaskProcessor : public Thread
    {
    private:

        TaskScheduler scheduler;
        DataProvider  provider;

        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        
        void MainLoop();

    public:

        /**
         * Creates TaskProcessor
         *
         * @param config
         * @exception ConfigException throws when configuration is invalid
         */
        TaskProcessor(ConfigView* config);
        virtual ~TaskProcessor();

        virtual int Execute();
        void Start();
        void Stop();
        
        inline bool isStarted() { return bStarted; };
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

