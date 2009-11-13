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
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"

#include "TaskScheduler.h"
#include "RetryPolicies.hpp"
#include "StatisticsManager.h"
#include "InfoSmeAdmin.h"

#include "InfoSme_Tasks_Stat_SearchCriterion.hpp"
#include "TrafficControl.hpp"
#include "TaskTypes.hpp"
#include "TaskDispatcher.h"

namespace smsc {
namespace infosme {

using namespace smsc::core::buffers;
using namespace smsc::core::threads;

using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;

using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

class FinalStateSaver;

typedef enum { beginGenerationMethod, endGenerationMethod, dropAllMessagesMethod } TaskMethod;

class TaskRunner : public ThreadedTask // for task method execution
{
private:

    TaskGuard             task;
    TaskMethod            method;
    Statistics*           statistics;
    TaskProcessorAdapter* processor;

public:

    TaskRunner(Task* theTask, TaskMethod method, 
               TaskProcessorAdapter* adapter = 0, Statistics* statistics = 0)
        : ThreadedTask(), task(theTask), method(method), processor(adapter), statistics(statistics) {};
    virtual ~TaskRunner() {};

    virtual int Execute()
    {
        __require__(task.get());
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


class ThreadManager : public ThreadPool
{
protected:

    smsc::logger::Logger *logger;
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

struct Int64HashFunc{
    static size_t CalcHash(uint64_t key)
    {
        return key;
    }
};

class MessageSender;

class TaskProcessor : public InfoSmeAdmin, public Thread
{
private:
    typedef IntHash< TaskGuard > TaskHash;

private:

    smsc::logger::Logger *log_;
    std::auto_ptr< FinalStateSaver > finalStateSaver_;

    ThreadManager taskManager;
    ThreadManager eventManager;

    TaskScheduler scheduler;    // for scheduled messages generation
    TaskDispatcher dispatcher_;
    static RetryPolicies retryPlcs;
    DataProvider* provider;     // to obtain registered data source by key

    Mutex              tasksLock;
    TaskHash           tasks;

    // Event       awake, exited;
    bool        bStarted, bNeedExit, notified_;
    EventMonitor startLock;
    int         switchTimeout;

    std::string storeLocation;

    MessageSender*  messageSender;

    int         responseWaitTime;
    int         receiptWaitTime;
    int         mappingRollTime; // FIXME: to be moved to smscconn
    size_t      mappingMaxChanges; // FIXME: to be moved to smscconn

    Connection*         dsStatConnection;
    StatisticsManager*  statistics;

    int     protocolId;
    char*   svcType;
    char*   address;

    void processWaitingEvents(time_t time);
    /// invoked from smsc connector, return true if receipt is needed
    bool processResponse( const TaskMsgId& tmIds,
                          const ResponseData& rd,
                          bool internal,
                          bool receipted );

    // process a lot of messages from the task and return the number of processed messages
    unsigned processTask(Task* task);
    void resetWaitingTasks();

    friend class SmscConnector;
    virtual void processMessage ( const TaskMsgId& tmIds, const ResponseData& rd);

    bool doesMessageConformToCriterion(ResultSet* rs,
                                       const InfoSme_Tasks_Stat_SearchCriterion& searchCrit);

    int unrespondedMessagesMax;
public:

    TaskProcessor();
    void init( ConfigView* config );
    virtual ~TaskProcessor();

    int getProtocolId() const { return protocolId; };
    const char* getSvcType() const { return (svcType) ? svcType:"InfoSme"; };
    const char* getAddress() const { return address; };

    int getResponseWaitTime() const { return responseWaitTime; }
    int getReceiptWaitTime() const { return receiptWaitTime; }
    int getMappingRollTime() const { return mappingRollTime; }
    size_t getMappingMaxChanges() const { return mappingMaxChanges; }
    const std::string& getStoreLocation() const { return storeLocation; }
    int getUnrespondedMessagesMax() const { return unrespondedMessagesMax; }

    virtual int Execute();
    void Start();
    virtual void Start(int) { Start(); }
    void Stop();

    inline bool isStarted() {
        MutexGuard guard(startLock);
        return bStarted;
    };

    /*
    void assignMessageSender(MessageSender* sender) {
        MutexGuard guard(tasksLock);
        messageSender = sender;
    };
    bool isMessageSenderAssigned() {
        MutexGuard guard(tasksLock);
        return (messageSender != 0);
    };
     */

    // virtual bool putTask(Task* task);
    // virtual bool addTask(Task* task);
    // virtual bool remTask(uint32_t taskId);
    // virtual bool delTask(uint32_t taskId);
    // virtual bool hasTask(uint32_t taskId);
    // virtual TaskInfo getTaskInfo(uint32_t taskId);

    /// @param config is a section "Tasks" of main config
    virtual void addTask( uint32_t taskId ) {
        if ( getTask(taskId).get() ) throw Exception("task %u exists already");
        initTask( taskId, 0 );
    }
    virtual void initTask( uint32_t taskId, smsc::util::config::ConfigView* config );
    virtual TaskGuard getTask( uint32_t taskId, bool remove = false );

    virtual bool invokeEndGeneration(Task* task) {
        return taskManager.startThread(new TaskRunner(task, endGenerationMethod,   this));
    };
    virtual bool invokeBeginGeneration(Task* task) {
        return taskManager.startThread(new TaskRunner(task, beginGenerationMethod, this, statistics));
    };
    virtual bool invokeDropAllMessages(Task* task) {
        return taskManager.startThread(new TaskRunner(task, dropAllMessagesMethod, this, statistics));
    };

    // invoked from smscconnector
    virtual bool invokeProcessEvent(ThreadedTask* task) {
        return eventManager.startThread(task);
    }

    virtual void awakeSignal() {
        MutexGuard mg(startLock);
        startLock.notifyAll();
        notified_ = true;
    };

    bool getStatistics(uint32_t taskId, TaskStat& stat) {
        return (statistics) ? statistics->getStatistics(taskId, stat):false;
    };

    static RetryPolicies& getRetryPolicies()
    {
      return retryPlcs;
    }
    
    static void retryMessage( Task* task, uint64_t msgId ) {
        time_t retryTime = getRetryPolicies().getRetryTime( task->getInfo().retryPolicy.c_str(),0);
        if ( retryTime == 0 ) {
            retryTime = 60*60;
        }
        task->retryMessage(msgId,time(0)+retryTime);
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
    
    virtual void reloadSmscAndRegions();

    // virtual void removeTask(uint32_t taskId);
    // virtual void changeTask(uint32_t taskId);

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

    virtual Array<std::string> getTaskMessages(uint32_t taskId,
                                               const InfoSme_T_SearchCriterion& searchCrit);

    virtual void endDeliveryMessagesGeneration(uint32_t taskId);

    virtual void changeDeliveryTextMessageByCompositCriterion(uint32_t taskId,
                                                              const std::string& newTextMsg,
                                                              const InfoSme_T_SearchCriterion& searchCrit);
    virtual void applyRetryPolicies();
    virtual uint32_t sendSms( const std::string& src,
                              const std::string& dst,
                              const std::string& msg,
                              bool flash );

private:
    /// NOTE: already locked
    void checkTaskActivity();
};

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR
