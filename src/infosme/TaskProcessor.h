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

#include "TaskScheduler.h"
#include "StatisticsManager.h"
#include "InfoSmeAdmin.h"

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
        Statistics*   statistics;
        
    public:
        
        TaskRunner(Task* task, TaskMethod method, Statistics* statistics = 0)
            : TaskGuard(task), ThreadedTask(), method(method), statistics(statistics) {};

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
                task->beginProcess(statistics);
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
        
        Mutex               stopLock;
        bool                bStopping;
        
        StatisticsManager*  statistics;
        
    public:
    
        TaskManager() 
            : TaskInvokeAdapter(), logger(Logger::getCategory("smsc.infosme.TaskManager")), 
                statistics(0), bStopping(false) {};
        virtual ~TaskManager() {
            this->Stop();
            pool.shutdown();
        };
    
        void Stop() {
            MutexGuard guard(stopLock);
            bStopping = true;
        }
    
        void setStatisticsManager(StatisticsManager* manager) {
            statistics = manager;
        };
        
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
            MutexGuard guard(stopLock);
            if (!bStopping) pool.startTask(new TaskRunner(task, endProcessMethod));
        };
        virtual void invokeBeginProcess(Task* task) {
            MutexGuard guard(stopLock);
            if (!bStopping) pool.startTask(new TaskRunner(task, beginProcessMethod, statistics));
        };
        virtual void invokeDropAllMessages(Task* task) {
            MutexGuard guard(stopLock);
            if (!bStopping) pool.startTask(new TaskRunner(task, dropAllMessagesMethod));
        };
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

    class TaskProcessor : public TaskProcessorAdapter, public InfoSmeAdmin, public Thread
    {
    private:

        log4cpp::Category  &logger;

        TaskManager   manager;      // for tasks methods execution on thread pool
        DataProvider  provider;     // to obtain registered data source by key
        TaskScheduler scheduler;    // for scheduled messages generation
        
        Hash<Task *>  tasks;
        Mutex         tasksLock;
        
        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        int         switchTimeout;

        const char* taskTablesPrefix;
        const char* dsInternalName;
        DataSource* dsInternal;
        Connection* dsIntConnection;
        Mutex       dsIntConnectionLock;
        int         dsCommitInterval;

        MessageSender*  messageSender;
        Mutex           messageSenderLock;

        IntHash<TaskMsgId> taskIdsBySeqNum;
        Mutex              taskIdsBySeqNumLock;
        
        Connection*         dsStatConnection;
        StatisticsManager*  statistics;

        int     protocolId;
        char*   svcType;
        char*   address;
        
        bool processTask(Task* task);

        void resetWaitingTasks();
        void dsInternalCommit(bool force=false);

    public:

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
        
        void assignMessageSender(MessageSender* sender) {
            MutexGuard guard(messageSenderLock);
            messageSender = sender;
        }
        virtual TaskInvokeAdapter& getTaskInvokeAdapter() {
            return manager;
        }

        virtual bool putTask(Task* task);
        virtual bool addTask(Task* task);
        virtual bool removeTask(std::string taskId);
        virtual bool hasTask(std::string taskId);
        virtual TaskGuard getTask(std::string taskId);
        
        void processResponce(int seqNum, bool accepted, bool retry, std::string smscId="");
        void processReceipt (std::string smscId, bool delivered, bool retry);

        bool getStatistics(std::string taskId, TaskStat& stat) {
            return (statistics) ? statistics->getStatistics(taskId, stat):false;
        };

        /* ------------------------ Admin interface ------------------------ */ 

        // TODO: implement administrative part
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

