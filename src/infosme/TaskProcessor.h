#ifndef SMSC_INFO_SME_TASK_PROCESSOR
#define SMSC_INFO_SME_TASK_PROCESSOR

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <memory>

#include "logger/Logger.h"

#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"

#include "core/buffers/Array.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "util/config/region/Region.hpp"
#include "db/DataSource.h"

#include "TaskScheduler.h"
#include "RetryPolicies.hpp"
#include "StatisticsManager.h"
#include "InfoSmeAdmin.h"

#include "InfoSme_Tasks_Stat_SearchCriterion.hpp"
#include "TrafficControl.hpp"
#include "core/buffers/JStore.hpp"

struct TaskIdMsgId{
  uint64_t msgId;
  uint32_t taskId;
};

inline size_t WriteRecord(File& f,uint64_t key,const TaskIdMsgId& val)
{
  f.WriteNetInt64(key);
  f.WriteNetInt64(val.msgId);
  f.WriteNetInt32(val.taskId);
  return 8+8+4;
}

inline size_t ReadRecord(File& f,uint64_t& key,TaskIdMsgId& val)
{
  key=f.ReadNetInt64();
  val.msgId=f.ReadNetInt64();
  val.taskId=f.ReadNetInt32();
  return 8+8+4;
}

inline size_t WriteKey(File& f,uint64_t key)
{
  f.WriteNetInt64(key);
  return 8;
}

inline size_t ReadKey(File& f,uint64_t& key)
{
  key=f.ReadNetInt64();
  return 8;
}

namespace smsc { namespace infosme
{
    using namespace smsc::core::buffers;
    using namespace smsc::core::threads;
    using namespace smsc::db;

    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;

    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    class FinalStateSaver;

    typedef enum { beginGenerationMethod, endGenerationMethod, dropAllMessagesMethod } TaskMethod;

    class TaskRunner : public TaskGuard, public ThreadedTask // for task method execution
    {
    private:

        TaskMethod            method;
        Statistics*           statistics;
        TaskProcessorAdapter* processor;

    public:

        TaskRunner(Task* task, TaskMethod method, 
                   TaskProcessorAdapter* adapter = 0, Statistics* statistics = 0)
            : TaskGuard(task), ThreadedTask(), method(method), processor(adapter), statistics(statistics) {};
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
                if (task->beginGeneration(statistics)) {
                    if (processor) processor->awakeSignal();
                }
                break;
            case dropAllMessagesMethod:
                task->dropNewMessages(statistics);
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
        ResponseData           rd;
    public:

        EventRunner(EventMethod method, TaskProcessorAdapter& processor,const ResponseData& argRd)
            : method(method), processor(processor),rd(argRd) {};
        /*EventRunner(EventMethod method, TaskProcessorAdapter& processor,
                    std::string smscId, bool delivered, bool retry)
            : method(method), processor(processor), seqNum(0),
              delivered(delivered), retry(retry), immediate(false), smscId(smscId), trafficst(false) {};*/

        virtual ~EventRunner() {};

        virtual int Execute()
        {
            switch (method)
            {
            case processResponceMethod:
                processor.processResponce(rd);
                if (!rd.trafficst) TrafficControl::incIncoming();
                break;
            case processReceiptMethod:
                processor.processReceipt (rd);
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
        virtual uint32_t sendSms(const std::string& src,const std::string& dst,const std::string& txt,bool flash)=0;
        virtual ~MessageSender() {};

    protected:

        MessageSender() {};
    };

    struct TaskMsgId
    {
        uint32_t taskId;
        uint64_t    msgId;

        uint32_t getTaskId()
        {
          return taskId;
        }

        TaskMsgId(uint32_t taskId=0, uint64_t msgId=0)
            : taskId(taskId), msgId(msgId) {};
    };

    class ThreadManager : public ThreadPool
    {
    protected:

        smsc::logger::Logger *logger;
        //ThreadPool           pool;

        Mutex               stopLock;
        bool                bStopping;

    public:

        ThreadManager() : ThreadPool(),
            logger(Logger::getInstance("smsc.infosme.ThreadManager")),
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
                smsc_log_warn(logger, "Maximum thread pool size wasn't specified !");
            }
            try {
                preCreateThreads(config->getInt("init"));
            } catch (ConfigException& exc) {
                smsc_log_warn(logger, "Precreated threads count in pool wasn't specified !");
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

    struct Int64HashFunc{
      static size_t CalcHash(uint64_t key)
      {
        return key;
      }
    };

    class TaskProcessor : public TaskProcessorAdapter, public InfoSmeAdmin, public Thread
    {
    private:

        smsc::logger::Logger *logger;
        std::auto_ptr< FinalStateSaver > finalStateSaver_;

        ThreadManager taskManager;
        ThreadManager eventManager;

        TaskScheduler scheduler;    // for scheduled messages generation
        static RetryPolicies retryPlcs;
        DataProvider  provider;     // to obtain registered data source by key

        IntHash<Task *>  tasks;
        Mutex         tasksLock;

        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;
        int         switchTimeout;

        JStore<uint64_t,TaskIdMsgId,Int64HashFunc> jstore;

        std::string storeLocation;
        

        MessageSender*  messageSender;
        Mutex           messageSenderLock;

        IntHash<TaskMsgId> taskIdsBySeqNum;
      //Mutex              taskIdsBySeqNumLock;
        EventMonitor       taskIdsBySeqNumMonitor;

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

        typedef std::map<std::string, TimeSlotCounter<int>* > timeSlotsHashByRegion_t;
        timeSlotsHashByRegion_t _timeSlotsHashByRegion;

        void processWaitingEvents(time_t time);
        bool processTask(Task* task);
        void resetWaitingTasks();

        virtual void processMessage (Task* task, uint64_t msgId,const ResponseData& rd);
        friend class EventRunner;
        virtual void processResponce(const ResponseData& rd, bool internal=false);
        virtual void processReceipt (const ResponseData& rd, bool internal=false);

        bool doesMessageConformToCriterion(ResultSet* rs,
                                           const InfoSme_Tasks_Stat_SearchCriterion& searchCrit);

        int unrespondedMessagesMax, unrespondedMessagesSleep;

        typedef enum { TRAFFIC_CONTINUED = 1, TRAFFIC_SUSPENDED = -1 } traffic_control_res_t;
        traffic_control_res_t controlTrafficSpeedByRegion(Task* task, Message& message);
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
        };
        bool isMessageSenderAssigned() {
            MutexGuard guard(messageSenderLock);
            return (messageSender != 0);
        };

        virtual bool putTask(Task* task);
        virtual bool addTask(Task* task);
        virtual bool remTask(uint32_t taskId);
        virtual bool delTask(uint32_t taskId);
        virtual bool hasTask(uint32_t taskId);
        virtual TaskGuard getTask(uint32_t taskId);

        virtual TaskInfo getTaskInfo(uint32_t taskId);


        virtual bool invokeEndGeneration(Task* task) {
            return taskManager.startThread(new TaskRunner(task, endGenerationMethod,   this));
        };
        virtual bool invokeBeginGeneration(Task* task) {
            return taskManager.startThread(new TaskRunner(task, beginGenerationMethod, this, statistics));
        };
        virtual bool invokeDropAllMessages(Task* task) {
            return taskManager.startThread(new TaskRunner(task, dropAllMessagesMethod, this, statistics));
        };

        virtual bool invokeProcessResponce(const ResponseData& rd)
        {
            return eventManager.startThread(new EventRunner(processResponceMethod, *this,rd));
        };
        virtual bool invokeProcessReceipt (const ResponseData& rd)
        {
            return eventManager.startThread(new EventRunner(processReceiptMethod, *this,rd));
        };
        virtual void awakeSignal() {
            awake.Signal();
        };

        bool getStatistics(uint32_t taskId, TaskStat& stat) {
            return (statistics) ? statistics->getStatistics(taskId, stat):false;
        };

        static RetryPolicies& getRetryPolicies()
        {
          return retryPlcs;
        }

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

        virtual void addTask(uint32_t taskId);
        virtual void removeTask(uint32_t taskId);
        virtual void changeTask(uint32_t taskId);

        virtual bool startTask(uint32_t taskId);
        virtual bool stopTask(uint32_t taskId);
        virtual Array<std::string> getGeneratingTasks();
        virtual Array<std::string> getProcessingTasks();

        virtual bool isTaskEnabled(uint32_t taskId);
        virtual bool setTaskEnabled(uint32_t taskId, bool enabled);

        virtual void addSchedule(std::string scheduleId);
        virtual void removeSchedule(std::string scheduleId);
        virtual void changeSchedule(std::string scheduleId);

        virtual void addDeliveryMessages(uint32_t taskId,
                                         uint8_t msgState,
                                         const std::string& address,
                                         time_t messageDate,
                                         const std::string& msg,
                                         const std::string& userData );

        virtual void changeDeliveryMessageInfoByRecordId(uint32_t taskId,
                                                         uint8_t messageState,
                                                         time_t unixTime,
                                                         const std::string& recordId);

        virtual void changeDeliveryMessageInfoByCompositCriterion(uint32_t taskId,
                                                                  uint8_t messageState,
                                                                  time_t unixTime,
                                                                  const InfoSme_T_SearchCriterion& searchCrit);
        virtual void deleteDeliveryMessageByRecordId(uint32_t taskId,
                                                     const std::string& recordId);

        virtual void deleteDeliveryMessagesByCompositCriterion(uint32_t taskId,
                                                               const InfoSme_T_SearchCriterion& searchCrit);

        /*virtual void insertRecordIntoTasksStat(uint32_t taskId,
                                               uint32_t period,
                                               uint32_t generated,
                                               uint32_t delivered,
                                               uint32_t retried,
                                               uint32_t failed);*/

        virtual Array<std::string> getTaskMessages(uint32_t taskId,
                                                   const InfoSme_T_SearchCriterion& searchCrit);

        //virtual Array<std::string> getTasksStatistic(const InfoSme_Tasks_Stat_SearchCriterion& searchCrit);

        virtual void endDeliveryMessagesGeneration(uint32_t taskId);

        virtual void changeDeliveryTextMessageByCompositCriterion(uint32_t taskId,
                                                                  const std::string& newTextMsg,
                                                                  const InfoSme_T_SearchCriterion& searchCrit);
      void applyRetryPolicies();
      uint32_t sendSms(const std::string& src,const std::string& dst,const std::string& msg,bool flash);
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR
