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

    typedef enum { beginGenerationMethod, endGenerationMethod, dropAllMessagesMethod } TaskMethod;
    
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
            case endGenerationMethod:
                task->endGeneration();
                break;
            case beginGenerationMethod:
                task->beginGeneration(statistics);
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
        virtual int  getSequenceNumber() = 0;
        virtual bool send(std::string abonent, std::string message, 
                          TaskInfo info, int seqNum) = 0;
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
        
        bool startThread(ThreadedTask* task) {
            MutexGuard guard(stopLock);
            if (!bStopping && task) {
                startTask(task);
                return true;
            }
            else if (task) delete task;
            return false;
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

    struct ReceiptData
    {
        bool receipted, delivered, retry;

        ReceiptData(bool receipted=false, bool delivered=false, bool retry=false)
         : receipted(receipted), delivered(delivered), retry(retry) {};
        ReceiptData(const ReceiptData& receipt)
          : receipted(receipt.receipted), delivered(receipt.delivered), retry(receipt.retry) {};

        ReceiptData& operator=(const ReceiptData& receipt) {
            receipted = receipt.receipted;
            delivered = receipt.delivered;
            retry = receipt.retry;
            return *this;
        }
    };

    struct ResponceTimer
    {
        time_t      timer;
        int         seqNum;
        
        ResponceTimer(time_t timer=0, int seqNum=0): timer(timer), seqNum(seqNum) {};
        ResponceTimer(const ResponceTimer& rt) : timer(rt.timer), seqNum(rt.seqNum) {};
        ResponceTimer& operator=(const ResponceTimer& rt) {
            timer = rt.timer; seqNum = rt.seqNum;
        }

    };
    struct ReceiptTimer
    {
        time_t      timer;
        std::string smscId;
        
        ReceiptTimer(time_t timer=0, std::string smscId="") : timer(timer), smscId(smscId) {};
        ReceiptTimer(const ReceiptTimer& rt) : timer(rt.timer), smscId(rt.smscId) {};
        ReceiptTimer& operator=(const ReceiptTimer& rt) {
            timer = rt.timer; smscId = rt.smscId;
        }
    };

    class TaskProcessor : public TaskProcessorAdapter, public InfoSmeAdmin, public Thread
    {
    private:

        log4cpp::Category  &logger;

        ThreadManager taskManager;
        ThreadManager eventManager;
        
        TaskScheduler scheduler;    // for scheduled messages generation
        DataProvider  provider;     // to obtain registered data source by key
        
        Hash<Task *>  tasks;
        Mutex         tasksLock;
        
        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        int         switchTimeout;

        const char* taskTablesPrefix;
        DataSource* dsInternal;
        Connection* dsIntConnection;
        Mutex       dsIntConnectionLock;
        
        MessageSender*  messageSender;
        Mutex           messageSenderLock;

        IntHash<TaskMsgId> taskIdsBySeqNum;
        Mutex              taskIdsBySeqNumLock;

        Hash<ReceiptData>  receipts;
        Mutex              receiptsLock;

        Mutex                   responceWaitQueueLock;
        Mutex                   receiptWaitQueueLock;
        Array<ResponceTimer>    responceWaitQueue;
        Array<ReceiptTimer>     receiptWaitQueue;
        int                     responceWaitTime;
        int                     receiptWaitTime;
        
        Connection*         dsStatConnection;
        StatisticsManager*  statistics;

        int     protocolId;
        char*   svcType;
        char*   address;
        
        void processWaitingEvents(time_t time);
        bool processTask(Task* task);
        void resetWaitingTasks();
        
        virtual void processMessage (Task* task, Connection* connection, uint64_t msgId,
                                     bool delivered, bool retry, bool immediate=false);
        friend class EventRunner;
        virtual void processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                     std::string smscId="", bool internal=false);
        virtual void processReceipt (std::string smscId, 
                                     bool delivered, bool retry, bool internal=false);
    
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
        bool isMessageSenderAssigned() {
            MutexGuard guard(messageSenderLock);
            return (messageSender != 0);
        }

        virtual bool putTask(Task* task);
        virtual bool addTask(Task* task);
        virtual bool remTask(std::string taskId);
        virtual bool delTask(std::string taskId);
        virtual bool hasTask(std::string taskId);
        virtual TaskGuard getTask(std::string taskId);
        
        virtual bool invokeEndGeneration(Task* task) {
            return taskManager.startThread(new TaskRunner(task, endGenerationMethod));
        };
        virtual bool invokeBeginGeneration(Task* task) {
            return taskManager.startThread(new TaskRunner(task, beginGenerationMethod, statistics));
        };
        virtual bool invokeDropAllMessages(Task* task) {
            return taskManager.startThread(new TaskRunner(task, dropAllMessagesMethod));
        };

        virtual bool invokeProcessResponce(int seqNum, bool accepted, 
                                           bool retry, bool immediate, std::string smscId="") 
        {
            return eventManager.startThread(new EventRunner(processResponceMethod, *this, 
                                                            seqNum, accepted, retry, immediate, smscId));
        };
        virtual bool invokeProcessReceipt (std::string smscId, bool delivered, bool retry)
        {
            return eventManager.startThread(new EventRunner(processReceiptMethod, *this, 
                                                            smscId, delivered, retry));
        };
        
        bool getStatistics(std::string taskId, TaskStat& stat) {
            return (statistics) ? statistics->getStatistics(taskId, stat):false;
        };

        /* ------------------------ Admin interface ------------------------ */ 

        virtual void startTaskProcessor() {
            this->Start();
        }
        virtual void stopTaskProcessor() {
            this->Stop();
        }
        virtual bool isTaskProcessorRunning() {
            return this->isStarted();
        }

        virtual void startTaskScheduler() {
            scheduler.Start();
        }
        virtual void stopTaskScheduler() {
            scheduler.Stop();
        }
        virtual bool isTaskSchedulerRunning() {
            return scheduler.isStarted();
        }
        virtual void flushStatistics() {
            if (statistics) statistics->flushStatistics();
        }
        
        virtual bool addTask(std::string taskId);
        virtual bool removeTask(std::string taskId);
        virtual bool changeTask(std::string taskId);
        
        virtual bool startTask(std::string taskId);
        virtual bool stopTask(std::string taskId);
        virtual Array<std::string> getGeneratingTasks();
        virtual Array<std::string> getProcessingTasks();

        virtual bool isTaskEnabled(std::string taskId);
        virtual bool setTaskEnabled(std::string taskId, bool enabled);
        
        virtual bool addSchedule(std::string scheduleId);
        virtual bool removeSchedule(std::string scheduleId);
        virtual bool changeSchedule(std::string scheduleId);
        
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

