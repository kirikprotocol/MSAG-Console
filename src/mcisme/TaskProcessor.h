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

    class TaskProcessor; class EventRunner;

    struct MessageSender
    {
        virtual int  getSequenceNumber() = 0;
        virtual bool send(int seqNumber, const Message& message) = 0;
        virtual ~MessageSender() {};

    protected:

        MessageSender() {};
    };

    class ThreadManager : public ThreadPool
    {
    protected:

        smsc::logger::Logger *logger;
        ThreadPool           pool;

        Mutex               stopLock;
        bool                bStopping;

        EventMonitor        threadMonitor;
        int                 threadCount;

        friend class EventRunner;

        void newThread() {
            MutexGuard guard(threadMonitor);
            threadCount++; 
            threadMonitor.notifyAll();
        }
        void delThread() {
            MutexGuard guard(threadMonitor);
            if (threadCount) threadCount--;
            threadMonitor.notifyAll();
        }

    public:

        ThreadManager() : ThreadPool(), logger(Logger::getInstance("smsc.mcisme.ThreadManager")),
            bStopping(false), threadCount(0) {};
        virtual ~ThreadManager() {
            this->Stop();
            shutdown();
        };

        void Stop()
        {
            {
                MutexGuard guard(stopLock);
                bStopping = true;
            }
            __trace__("Waiting pooled threads finishing ...");
            MutexGuard guard(threadMonitor);
            while (threadCount > 0) threadMonitor.wait();
            __trace__("All pooled threads finished.");
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

    typedef enum { processResponceMethod, processReceiptMethod } EventMethod;
    class EventRunner : public ThreadedTask
    {
    private:

        EventMethod     method;
        TaskProcessor&  processor;
        ThreadManager&  manager;

        int             seqNum;
        bool            delivered, retry, immediate, replace, replace_failed;
        std::string     smscId;

    public:

        virtual const char* taskName() { return "MCISmeEvent"; };

        EventRunner(EventMethod method, TaskProcessor& processor, ThreadManager& _manager,
                    int seqNum, bool accepted, bool retry, bool immediate, 
                    bool replace, bool replace_failed, std::string smscId="")
            : method(method), processor(processor), manager(_manager), seqNum(seqNum),
              delivered(accepted), retry(retry), immediate(immediate), 
              replace(replace), replace_failed(replace_failed), smscId(smscId) 
        { 
            manager.newThread();
        };
        EventRunner(EventMethod method, TaskProcessor& processor, ThreadManager& _manager,
                    std::string smscId, bool delivered, bool retry)
            : method(method), processor(processor), manager(_manager), seqNum(0),
              delivered(delivered), retry(retry), immediate(false),
              replace(false), replace_failed(false), smscId(smscId)
        {
            manager.newThread();
        };
        virtual ~EventRunner()
        { 
            manager.delThread();
        };
        
        virtual int Execute();
    };

    struct ReceiptData
    {
        bool delivered, retry;

        ReceiptData(bool delivered=false, bool retry=false) : delivered(delivered), retry(retry) {};
        ReceiptData(const ReceiptData& rcpt) : delivered(rcpt.delivered), retry(rcpt.retry) {};
        ReceiptData& operator=(const ReceiptData& rcpt) {
            delivered = rcpt.delivered; retry = rcpt.retry;
            return *this;
        };
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

    class ResponcesTracker : public Thread
    {
    private:
        
        smsc::logger::Logger *logger;

        TaskProcessor*  processor;

        Mutex           startLock;
        bool            bStarted, bNeedExit;
        Event           exitedEvent;

        EventMonitor                responcesMonitor;
        IntHash<Message>            messages;
        Hash<ReceiptData>           receipts;
        CyclicQueue<ResponceTimer>  responceWaitQueue; // used for responce waiting after submit|replace
        CyclicQueue<ReceiptTimer>   receiptWaitQueue;  // used for responce waiting after receipt come

        int                       responceWaitTime, receiptWaitTime;

    public:

        ResponcesTracker() 
            : Thread(), logger(Logger::getInstance("smsc.mcisme.ResponceTracker")),
                processor(0), bStarted(false), bNeedExit(false), responceWaitTime(0), receiptWaitTime(0) {};
        ~ResponcesTracker() { Stop(); };

        void Start(); void Stop();
        virtual int Execute();
        
        void cleanup();
        void init(TaskProcessor* processor, ConfigView* config);
        
        bool putResponceData(int seqNum, const Message& message);
        bool popResponceData(int seqNum, Message& message);
        bool popReceiptData (const char* smsc_id, ReceiptData& receipt);
        bool putReceiptData (const char* smsc_id, const ReceiptData& receipt);
    };

    class TaskProcessor : public Thread, public MissedCallListener, public MCISmeAdmin
    {
    private:

        smsc::logger::Logger *logger;

        int     protocolId;
        char    *svcType, *address;

        ThreadManager       eventManager;
        ResponcesTracker    responcesTracker;
        MCIModule*          mciModule;

        Mutex               messageSenderLock;
        MessageSender*      messageSender;
        
        DataSource*     ds;
        Connection*     dsStatConnection;
        // StatisticsManager*  statistics;
        
        EventMonitor    tasksMonitor;
        Hash<Task *>    tasks, lockedTasks; // Hash for tasks by abonent
        EventMonitor    smscIdMonitor;
        Hash<bool>      lockedSmscIds;
        
        Mutex   startLock;
        Event   exitedEvent;
        bool    bStarted, bInQueueOpen, bOutQueueOpen;
        int                             maxInQueueSize, maxOutQueueSize;
        EventMonitor                    inQueueMonitor, outQueueMonitor;
        CyclicQueue<MissedCallEvent>    inQueue;
        CyclicQueue<Message>            outQueue;
        
        void loadupTasks();
        void unloadTasks();

        friend class ResponcesTracker;
        void openInQueue();
        void closeInQueue();
        bool putToInQueue(const MissedCallEvent& event);
        bool getFromInQueue(MissedCallEvent& event);
        
        void openOutQueue();
        void closeOutQueue();
        bool putToOutQueue(const Message& event, bool force=false);
        bool getFromOutQueue(Message& event);
        
        void initDataSource(ConfigView* config);

        friend class EventRunner;
        virtual void processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                     bool replace, bool replace_failed, std::string smscId="");
        virtual void processReceipt (std::string smscId, bool delivered, bool retry);

        void processReceipt(Task* task, bool delivered, bool retry, 
                            const char* smsc_id, uint64_t msg_id=0);
    
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
        
        void lockSmscId(const char* smsc_id);
        void freeSmscId(const char* smsc_id);

        Task* getTask (const char* abonent);
        Task* getTask (const char* abonent, bool& newone);
        bool  delTask (const char* abonent);
        bool  freeTask(const char* abonent);

        void Run();             // outQueue processing
        virtual int Execute();  // inQueue processing
        void Start(); void Stop();
        
        void processEvent(const MissedCallEvent& event);
        void processMessage(const Message& message);

        virtual void missed(MissedCallEvent& event) {
            putToInQueue(event);
        };
        
        virtual bool invokeProcessResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                           bool replace, bool replace_failed, std::string smscId="")
        {
            return eventManager.startThread(new EventRunner(processResponceMethod, *this, eventManager,
                                                            seqNum, accepted, retry, immediate, 
                                                            replace, replace_failed, smscId));
        };
        virtual bool invokeProcessReceipt (std::string smscId, bool delivered, bool retry)
        {
            return eventManager.startThread(new EventRunner(processReceiptMethod, *this, eventManager,
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
