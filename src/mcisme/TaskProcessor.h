#ifndef SMSC_MCI_SME_TASK_PROCESSOR
#define SMSC_MCI_SME_TASK_PROCESSOR

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
#include <core/buffers/IntHash.hpp>
#include <core/buffers/CyclicQueue.hpp>

#include <core/threads/Thread.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <db/DataSource.h>

#include "Tasks.h"
#include "MCIModule.h"
#include "MCISmeAdmin.h"
//TODO: #include "StatisticsManager.h"

namespace smsc { namespace mcisme
{
    using namespace smsc::misscall;

    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    using namespace smsc::core::threads;
    using namespace smsc::db;

    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    typedef enum { processResponceMethod, processReceiptMethod } EventMethod;

    class TaskProcessor;
    class EventRunner : public ThreadedTask
    {
    private:

        EventMethod     method;
        TaskProcessor&  processor;

        int             seqNum;
        bool            delivered, retry, immediate;
        std::string     smscId;

    public:

        virtual const char* taskName() { return "MCISmeEvent"; };

        EventRunner(EventMethod method, TaskProcessor& processor, int seqNum,
                    bool accepted, bool retry, bool immediate, std::string smscId="")
            : method(method), processor(processor), seqNum(seqNum),
                delivered(accepted), retry(retry), immediate(immediate), smscId(smscId) {};
        EventRunner(EventMethod method, TaskProcessor& processor,
                    std::string smscId, bool delivered, bool retry)
            : method(method), processor(processor), seqNum(0),
                delivered(delivered), retry(retry), immediate(false), smscId(smscId) {};

        virtual ~EventRunner() {};
        virtual int Execute();
    };

    struct MessageSender
    {
        virtual int  getSequenceNumber() = 0;
        virtual bool send(const Message& message) = 0;
        virtual ~MessageSender() {};

    protected:

        MessageSender() {};
    };

    struct TaskMsgId
    {
        std::string taskId;
        uint64_t    msgId;

        TaskMsgId(std::string taskId="", uint64_t msgId=0) : taskId(taskId), msgId(msgId) {};
        TaskMsgId(const TaskMsgId& tmi) : taskId(tmi.taskId), msgId(tmi.msgId) {};

        TaskMsgId& operator=(const TaskMsgId& tmi) {
            taskId = tmi.taskId; msgId = tmi.msgId;
            return *this;
        }
    };

    class ThreadManager : public ThreadPool
    {
    protected:

        smsc::logger::Logger *logger;
        ThreadPool           pool;

        Mutex               stopLock;
        bool                bStopping;

    public:

        ThreadManager() : ThreadPool(),
            logger(Logger::getInstance("smsc.mcisme.ThreadManager")), bStopping(false) {};
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
                smsc_log_warn(logger, "Maximum thread pool size wasn't specified !");
            }
            try {
                preCreateThreads(config->getInt("init"));
            } catch (ConfigException& exc) {
                smsc_log_warn(logger, "Precreated threads count in pool wasn't specified !");
            }
        }
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
            return *this;
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
            return *this;
        }
    };

    class TaskProcessor : public Thread, public MissedCallListener, public MCISmeAdmin
    {
    private:

        smsc::logger::Logger *logger;

        int     protocolId;
        char    *svcType, *address;

        ThreadManager   eventManager;

        Mutex           messageSenderLock;
        MessageSender*  messageSender;
        
        DataSource*     ds;
        Connection*     dsStatConnection;
        // StatisticsManager*  statistics;
        
        Mutex           tasksLock;
        Hash<Task *>    tasks; // Hash for tasks by abonent

        int                responceWaitTime, receiptWaitTime;
        Mutex              taskIdsBySeqNumLock, responceWaitQueueLock, receiptWaitQueueLock;
        IntHash<TaskMsgId> taskIdsBySeqNum;
        Mutex              receiptsLock;
        Hash<ReceiptData>  receipts;
        
        Array<ResponceTimer>    responceWaitQueue;
        Array<ReceiptTimer>     receiptWaitQueue; // ???
        
        Mutex   startLock;
        Event   exitedEvent;
        bool    bStarted, bInQueueOpen, bOutQueueOpen;
        int                             maxInQueueSize, maxOutQueueSize;
        EventMonitor                    inQueueMonitor, outQueueMonitor;
        CyclicQueue<MissedCallEvent>    inQueue;
        CyclicQueue<Message>            outQueue;
        
        void openInQueue();
        void closeInQueue();
        bool putToInQueue(const MissedCallEvent& event);
        bool getFromInQueue(MissedCallEvent& event);
        
        void openOutQueue();
        void closeOutQueue();
        bool putToOutQueue(const Message& event);
        bool getFromOutQueue(Message& event);
        
        void initDataSource(ConfigView* config);

        Task* getTask(const char* abonent, bool& newone);

        friend class EventRunner;
        virtual void processResponce(int seqNum, bool accepted, bool retry,
                                     bool immediate, std::string smscId="");
        virtual void processReceipt (std::string smscId, bool delivered, bool retry);

    public:

        TaskProcessor(ConfigView* config);
        virtual ~TaskProcessor();

        int getProtocolId()      { return protocolId; };
        const char* getSvcType() { return (svcType) ? svcType:"MCISme"; };
        const char* getAddress() { return address; };

        void assignMessageSender(MessageSender* sender) {
            MutexGuard guard(messageSenderLock);
            messageSender = sender;
        };
        bool isMessageSenderAssigned() {
            MutexGuard guard(messageSenderLock);
            return (messageSender != 0);
        };

        void Run();             // outQueue processing
        void Stop();
        void Start();           
        virtual int Execute();  // inQueue processing

        void processEvent(const MissedCallEvent& event);
        void processMessage(const Message& message);

        virtual void missed(MissedCallEvent& event) {
            putToInQueue(event);
        }
        
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

        /* ------------------------ Admin interface ------------------------ */

        virtual void flushStatistics() {
            //if (statistics) statistics->flushStatistics();
        }
        virtual int getInQueueSize() {
            MutexGuard guard(inQueueMonitor);
            return inQueue.Count();
        }
        virtual int getOutQueueSize() {
            MutexGuard guard(outQueueMonitor);
            return outQueue.Count();
        }

    };

}}

#endif //SMSC_MCI_SME_TASK_PROCESSOR
