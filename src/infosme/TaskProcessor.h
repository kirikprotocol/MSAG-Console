#ifndef SMSC_INFO_SME_TASK_PROCESSOR
#define SMSC_INFO_SME_TASK_PROCESSOR

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <logger/Logger.h>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <core/threads/Thread.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <db/DataSource.h>

#include "DataProvider.h"
#include "TaskScheduler.h"
#include "Task.h"

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::core::threads;
    using namespace smsc::db;
    
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    class TaskRunner : public ThreadedTask
    {
    private:

        Task* task;
        int   method;
        // add method params here !!!

    public:
        
        TaskRunner(Task* task, int method)
            : ThreadedTask(), task(task), method(method) {};
        virtual ~TaskRunner() {};
        
        virtual int Execute() {
            // TODO : implement it ! Call specified method
            return 0;
        };
        virtual const char* taskName() {
            return "InfoSmeTask";
        };
    };

    class TaskManager 
    {
    private:
    
        log4cpp::Category  &logger;
        ThreadPool          pool;
        
    public:
    
        TaskManager() : logger(Logger::getCategory("smsc.infosme.TaskManager")) {};
        virtual ~TaskManager() {
            shutdown();
        };
    
        void shutdown() {
            pool.shutdown();
        }
    
        void init(ConfigView* config) // throw(ConfigException)
        {
            try 
            {
                int maxThreads = config->getInt("max");
                pool.setMaxThreads(maxThreads);
            } 
            catch (ConfigException& exc) {
                logger.warn("Maximum thread pool size wasn't specified !");
            }
            
            try
            {
                int initThreads = config->getInt("init");
                pool.preCreateThreads(initThreads);
            }
            catch (ConfigException& exc) {
                logger.warn("Precreated threads count in pool wasn't specified !");
            }
        };
        
        void invokeTaskMethod(Task* task, int method) {
            pool.startTask(new TaskRunner(task, method));
        };
    };
    
    class TaskProcessor : public Thread
    {
    private:

        log4cpp::Category  &logger;

        TaskManager   manager;  
        DataProvider  provider;
        TaskScheduler scheduler;
        
        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;

        const char* dsInternalName;
        DataSource* dsInternal;
        
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

        bool addTask(const Task* task);
        bool removeTask(std::string taskName);
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

