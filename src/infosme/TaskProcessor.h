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

    typedef enum { beginProcessMethod, endProcessMethod, dropAllMessagesMethod } TaskMethod;
    
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

    typedef enum { processResponceMethod, processReceiptMethod } EventMethod;

    class EventRunner : public ThreadedTask
    {
    private:

        EventMethod            method;
        TaskProcessorAdapter&  processor;

        int             seqNum;
        bool            delivered, retry, immediate;
        std::string     smscId;

    public:

        EventRunner(EventMethod method, TaskProcessorAdapter& processor, int seqNum,
                    bool accepted, bool retry, bool immediate, std::string smscId="")
            : method(method), processor(processor), seqNum(seqNum), 
                delivered(accepted), retry(retry), immediate(immediate), smscId(smscId) {};
        EventRunner(EventMethod method, TaskProcessorAdapter& processor, 
                    std::string smscId, bool delivered, bool retry)
            : method(method), processor(processor), seqNum(0), 
                delivered(delivered), retry(retry), immediate(false), smscId(smscId) {};

        virtual ~EventRunner() {};

        virtual int Execute()
        {
            switch (method)
            {
            case processResponceMethod:
                processor.processResponce(seqNum, delivered, retry, immediate, smscId);
                break;
            case processReceiptMethod:
                processor.processReceipt (smscId, delivered, retry);
                break;
            default:
                __trace2__("Invalid method '%d' invoked by event.", method);
                return -1;
            }
            return 0;
        };
        virtual const char* taskName() {
            return "InfoSmeEvent";
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

    class ThreadManager : public ThreadPool
    {
    protected:

        log4cpp::Category  &logger;
        ThreadPool          pool;
        
        Mutex               stopLock;
        bool                bStopping;

    public:

        ThreadManager() : ThreadPool(), 
            logger(Logger::getCategory("smsc.infosme.ThreadManager")), 
                bStopping(false) {};
        virtual ~ThreadManager() {
            this->Stop();
            shutdown();
        };
        
        void Stop() {
            MutexGuard guard(stopLock);
            bStopping = true;
        }
        
        void startThread(ThreadedTask* task) {
            MutexGuard guard(stopLock);
            if (!bStopping && task) startTask(task);
            else if (task) delete task;
        }
        
        void init(ConfigView* config) // throw(ConfigException)
        {
            try {
                setMaxThreads(config->getInt("max"));
            } catch (ConfigException& exc) {
                logger.warn("Maximum thread pool size wasn't specified !");
            }
            try {
                preCreateThreads(config->getInt("init"));
            } catch (ConfigException& exc) {
                logger.warn("Precreated threads count in pool wasn't specified !");
            }
        };
    };

    class TaskProcessor : public TaskProcessorAdapter, public InfoSmeAdmin, public Thread
    {
    private:

        log4cpp::Category  &logger;

        DataProvider  provider;     // to obtain registered data source by key
        TaskScheduler scheduler;    // for scheduled messages generation

        ThreadManager taskManager;
        ThreadManager eventManager;
        
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
        
        friend class EventRunner;
        virtual void processResponce(int seqNum, bool accepted, 
                                     bool retry, bool immediate, std::string smscId="");
        virtual void processReceipt (std::string smscId, bool delivered, bool retry);
    
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

        virtual bool putTask(Task* task);
        virtual bool addTask(Task* task);
        virtual bool removeTask(std::string taskId);
        virtual bool hasTask(std::string taskId);
        virtual TaskGuard getTask(std::string taskId);
        
        virtual void invokeEndProcess(Task* task) {
            taskManager.startThread(new TaskRunner(task, endProcessMethod));
        };
        virtual void invokeBeginProcess(Task* task) {
            taskManager.startThread(new TaskRunner(task, beginProcessMethod, statistics));
        };
        virtual void invokeDropAllMessages(Task* task) {
            taskManager.startThread(new TaskRunner(task, dropAllMessagesMethod));
        };

        virtual void invokeProcessResponce(int seqNum, bool accepted, 
                                           bool retry, bool immediate, std::string smscId="")
        {
            eventManager.startThread(new EventRunner(processResponceMethod, *this, 
                                                     seqNum, accepted, retry, immediate, smscId));
        };
        virtual void invokeProcessReceipt (std::string smscId, bool delivered, bool retry)
        {
            eventManager.startThread(new EventRunner(processReceiptMethod, *this, 
                                                     smscId, delivered, retry));
        };
        
        bool getStatistics(std::string taskId, TaskStat& stat) {
            return (statistics) ? statistics->getStatistics(taskId, stat):false;
        };

        /* ------------------------ Admin interface ------------------------ */ 

        // TODO: implement administrative part
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

