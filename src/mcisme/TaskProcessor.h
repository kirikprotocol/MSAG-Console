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

#include <system/smscsignalhandlers.h>

#include <db/DataSource.h>

#include "Tasks.h"
#include "MCIModule.h"
#include "MCISmeAdmin.h"
#include "TemplateManager.h"
#include "StatisticsManager.h"

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
        bool            delivered, retry, immediate, cancel, cancel_failed;
        bool            expired, deleted;
        std::string     smscId;

    public:

        virtual const char* taskName() { return "MCISmeEvent"; };

        EventRunner(EventMethod method, TaskProcessor& processor, ThreadManager& _manager,
                    int seqNum, bool accepted, bool retry, bool immediate, 
                    bool cancel, bool cancel_failed, std::string smscId="")
            : method(method), processor(processor), manager(_manager), seqNum(seqNum),
              delivered(accepted), retry(retry), immediate(immediate), 
              cancel(cancel), cancel_failed(cancel_failed), smscId(smscId),
              expired(false), deleted(false)
        { 
            manager.newThread();
        };
        EventRunner(EventMethod method, TaskProcessor& processor, ThreadManager& _manager,
                    std::string smscId, bool delivered, bool expired, bool deleted)
            : method(method), processor(processor), manager(_manager), seqNum(0),
              delivered(delivered), retry(false), immediate(false), cancel(false), 
              cancel_failed(false), expired(expired), deleted(deleted), smscId(smscId)
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
        bool delivered, expired;

        ReceiptData(bool delivered=false, bool expired=false) : delivered(delivered), expired(expired) {};
        ReceiptData(const ReceiptData& rcpt) : delivered(rcpt.delivered), expired(rcpt.expired) {};
        ReceiptData& operator=(const ReceiptData& rcpt) {
            delivered = rcpt.delivered; expired = rcpt.expired;
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
        Hash<int>                   smscIds;            // unresponded cancel messages seqNums by smsc_ids
        IntHash<Message>            messages;           // messages by sequence number
        Hash<ReceiptData>           receipts;           // receipts by smsc_id
        CyclicQueue<ResponceTimer>  responceWaitQueue;  // used for responce waiting after submit|cancel
        CyclicQueue<ReceiptTimer>   receiptWaitQueue;   // used for responce waiting after receipt come

        int responceWaitTime, receiptWaitTime;

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
        bool popResponceData(const char* smsc_id, Message& message);
        bool popReceiptData (const char* smsc_id, ReceiptData& receipt);
        bool putReceiptData (const char* smsc_id, const ReceiptData& receipt);
    };

    class TaskProcessor : public Thread, public MissedCallListener, public AdminInterface
    {
    private:

        smsc::logger::Logger *logger;

        int     protocolId, daysValid;
        char    *svcType, *address;

        ThreadManager       eventManager;
        ResponcesTracker    responcesTracker;
        TemplateManager*    templateManager;
        MCIModule*          mciModule;

        Mutex               messageSenderLock;
        MessageSender*      messageSender;
        
        DataSource*         ds;
        Connection*         dsStatConnection;
        StatisticsManager*  statistics;
        
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
        bool putToInQueue(const MissedCallEvent& event, bool skip=true);
        bool getFromInQueue(MissedCallEvent& event);
        
        void openOutQueue();
        void closeOutQueue();
        bool putToOutQueue(const Message& event, bool force=false);
        bool getFromOutQueue(Message& event);
        
        void initDataSource(ConfigView* config);

        bool formatMessage(Task* task, Message& message);
        bool processCancel(Task* task, const char* smsc_id, Message& message,
                           bool receipted=false, bool delivered=false, bool expired=false);
        void processReceipt(Task* task, bool delivered, bool expired, const char* smsc_id, uint64_t msg_id=0);
        void processNotificationResponce(Message& message,
                                         bool accepted, bool retry, bool immediate, std::string smscId="");
        friend class EventRunner;
        virtual void processResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                     bool cancel, bool cancel_failed, std::string smscId="");
        virtual void processReceipt (std::string smscId, bool delivered, bool expired, bool deleted);
    
    public:

        TaskProcessor(ConfigView* config);
        virtual ~TaskProcessor();

        int getDaysValid()       { return daysValid;  };
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

        virtual void missed(MissedCallEvent event) {
            putToInQueue(event);
        };
        
        virtual bool invokeProcessResponce(int seqNum, bool accepted, bool retry, bool immediate,
                                           bool cancel, bool cancel_failed, std::string smscId="")
        {
            return eventManager.startThread(new EventRunner(processResponceMethod, *this, eventManager,
                                                            seqNum, accepted, retry, immediate, 
                                                            cancel, cancel_failed, smscId));
        };
        virtual bool invokeProcessReceipt (std::string smscId, bool delivered, bool expired, bool deleted)
        {
            return eventManager.startThread(new EventRunner(processReceiptMethod, *this, eventManager,
                                                            smscId, delivered, expired, deleted));
        };

        /* ------------------------ Admin interface ------------------------ */

        virtual void flushStatistics() {
            if (statistics) statistics->flushStatistics();
        }
        virtual EventsStat getStatistics() {
            return (statistics) ? statistics->getStatistics():EventsStat(0,0,0,0);
        }
        virtual int getActiveTasksCount() {
            MutexGuard guard(tasksMonitor);
            return tasks.GetCount();
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
