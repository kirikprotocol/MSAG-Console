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

    class TaskContainer : public TaskContainerAdapter
    {
    private:
        
        Hash<Task *>    tasks;
        Mutex           tasksLock;
        int             prioritySum;

    public:
        
        TaskContainer() : TaskContainerAdapter(), prioritySum(0) {};
        virtual ~TaskContainer();

        virtual bool  addTask(Task* task);
        virtual bool  removeTask(std::string taskName);
        virtual Task* getTask(std::string taskName);

        virtual Task* getNextTask();
    };

    typedef enum {
      beginProcessMethod, endProcessMethod, doNotifyMessageMethod, dropAllMessagesMethod
    } TaskMethod;

    class TaskRunner : public ThreadedTask // for task method execution 
    {
    private:
        
        Task*         task;
        TaskMethod    method;
        Connection*   connection;
        StateInfo     info;
        
    public:
        
        TaskRunner(Task* task, TaskMethod method, Connection* connection=0)
            : ThreadedTask(), task(task), method(method), connection(connection) {};
        TaskRunner(Task* task, TaskMethod method, const StateInfo& info)
            : ThreadedTask(), task(task), method(method), connection(0), info(info) {};

        virtual ~TaskRunner() {};
        
        virtual int Execute()
        {
            __require__(task);

            switch (method)
            {
            case endProcessMethod:
                task->endProcess();
                break;
            case beginProcessMethod:
                task->beginProcess();
                break;
            case dropAllMessagesMethod:
                task->dropAllMessages();
                break;
            case doNotifyMessageMethod:
                task->doNotifyMessage(info);
                break;
            default:
                __trace2__("Invalid method '%d' invoked on task.", method);
                return -1;
            }
            return 0;
        };
        virtual const char* taskName() {
            return "InfoSmeTask";
        };
    };

    class TaskManager : public TaskInvokeAdapter // for tasks execution on thread pool
    {
    private:
    
        log4cpp::Category  &logger;
        ThreadPool          pool;
        
    public:
    
        TaskManager() : TaskInvokeAdapter(), 
            logger(Logger::getCategory("smsc.infosme.TaskManager")) {};
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
        
        virtual void invokeEndProcess(Task* task) {
            pool.startTask(new TaskRunner(task, endProcessMethod));
        };
        virtual void invokeBeginProcess(Task* task) {
            pool.startTask(new TaskRunner(task, beginProcessMethod));
        };
        virtual void invokeDoNotifyMessage(Task* task, const StateInfo& info) {
            pool.startTask(new TaskRunner(task, doNotifyMessageMethod, info));
        };
        virtual void invokeDropAllMessages(Task* task) {
            pool.startTask(new TaskRunner(task, dropAllMessagesMethod));
        };
    };
    
    class TaskProcessor : public TaskProcessorAdapter, public Thread
    {
    private:

        log4cpp::Category  &logger;

        TaskManager   manager;      // for tasks methods execution on thread pool
        DataProvider  provider;     // to obtain registered data source by key
        TaskContainer container;    // contains tasks by name & priority
        TaskScheduler scheduler;    // for scheduled messages generation
        
        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        int         switchTimeout;

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
        
        inline bool isStarted() { 
            MutexGuard guard(startLock);
            return bStarted;
        };
        
        virtual TaskInvokeAdapter& getTaskInvokeAdapter() {
            return manager;
        }
        virtual TaskContainerAdapter& getTaskContainerAdapter() {
            return container;
        }
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

