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

    typedef enum {
      beginProcessMethod, endProcessMethod, dropAllMessagesMethod
    } TaskMethod;

    class TaskRunner : public TaskGuard, public ThreadedTask // for task method execution 
    {
    private:
        
        TaskMethod    method;
        
    public:
        
        TaskRunner(Task* task, TaskMethod method)
            : TaskGuard(task), ThreadedTask(), method(method) {};

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
        virtual void invokeDropAllMessages(Task* task) {
            pool.startTask(new TaskRunner(task, dropAllMessagesMethod));
        };
    };
    
    class TaskContainer : public TaskContainerAdapter
    {
    private:
        
        Hash<Task *>    tasks;
        Mutex           tasksLock;
        int             prioritySum;

    public:
        
        TaskContainer() : TaskContainerAdapter(), prioritySum(0) {};
        virtual ~TaskContainer();

        virtual bool putTask(Task* task);
        virtual bool addTask(Task* task);
        virtual bool removeTask(std::string taskId);
        virtual bool hasTask(std::string taskId);
        
        virtual TaskGuard getTask(std::string taskId);
        virtual TaskGuard getNextTask();

        void resetWaitingTasks();
    };

    struct MessageSender
    {
        virtual bool send(std::string abonent, std::string message, 
                          TaskInfo info, int& seqNum) = 0;
        virtual ~MessageSender() {};

    protected:
        
        MessageSender() {};
    };

    struct TaskMsgId
    {
        std::string taskId;
        uint64_t    msgId;

        TaskMsgId(std::string taskId="", uint64_t msgId=0) 
            : taskId(taskId), msgId(msgId) {};
        TaskMsgId(const TaskMsgId& tmi) 
            : taskId(tmi.taskId), msgId(tmi.msgId) {};
        
        TaskMsgId& operator=(const TaskMsgId& tmi) {
            taskId = tmi.taskId; msgId = tmi.msgId;
            return *this;
        }
    };

    class TaskProcessor : public TaskProcessorAdapter, public Thread
    {
    private:

        log4cpp::Category  &logger;

        TaskManager   manager;      // for tasks methods execution on thread pool
        DataProvider  provider;     // to obtain registered data source by key
        TaskContainer container;    // contains tasks by id & priority
        TaskScheduler scheduler;    // for scheduled messages generation
        
        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        int         switchTimeout;

        const char* taskTablesPrefix;
        const char* dsInternalName;
        DataSource* dsInternal;
        Connection* dsIntConnection;
        Mutex       dsIntConnectionLock;

        MessageSender*  messageSender;
        Mutex           messageSenderLock;

        IntHash<TaskMsgId> taskIdsBySeqNum;
        Mutex              taskIdsBySeqNumLock;

        int     protocolId;
        char*   svcType;
        char*   address;
        
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

        int getProtocolId()      { return protocolId; };
        const char* getSvcType() { return (svcType) ? svcType:"InfoSme"; };
        const char* getAddress() { return address; };
        
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

        void assignMessageSender(MessageSender* sender) {
            MutexGuard guard(messageSenderLock);
            messageSender = sender;
        }

        void processResponce(int seqNum, bool accepted, bool retry, std::string smscId="");
        void processReceipt (std::string smscId, bool delivered);

    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

