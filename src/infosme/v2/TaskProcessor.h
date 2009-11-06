#ifndef SMSC_INFOSME2_TASKPROCESSOR_H
#define SMSC_INFOSME2_TASKPROCESSOR_H

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <util/int.h>
#include <memory>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "util/config/ConfigView.h"
#include "core/synchronization/EventMonitor.hpp"

#include "InfoSmeAdmin.h"
#include "TaskScheduler.h"
#include "MessageSender.h"
#include "TaskDispatcher.h"
#include "Task.h"

// #include "util/config/ConfigException.h"
// #include "core/threads/ThreadPool.hpp"
// #include "core/synchronization/Event.hpp"
// #include "db/DataSource.h"

// #include "TaskScheduler.h"
// #include "RetryPolicies.hpp"
// #include "StatisticsManager.h"

// #include "InfoSme_Tasks_Stat_SearchCriterion.hpp"
// #include "TrafficControl.hpp"
// #include "TaskTypes.hpp"

namespace smsc {
namespace infosme2 {

class FinalStateSaver;
class DataProvider;

/*
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

*/

class TaskProcessor : public InfoSmeAdmin, public TaskProcessorAdapter, protected smsc::core::threads::Thread
{
public:
    TaskProcessor();
    virtual ~TaskProcessor();

    void init( smsc::util::config::ConfigView* config );

    TaskGuard getTask( unsigned taskId );
    void addTask( Task* task );

    /*
    int getProtocolId() const { return protocolId; };
    const char* getSvcType() const { return (svcType) ? svcType: "InfoSme"; };
    const char* getAddress() const { return address; };

    int getResponseWaitTime() const { return responseWaitTime; }
    int getReceiptWaitTime() const { return receiptWaitTime; }
    int getMappingRollTime() const { return mappingRollTime; }
    size_t getMappingMaxChanges() const { return mappingMaxChanges; }
    const std::string& getStoreLocation() const { return storeLocation; }
    int getUnrespondedMessagesMax() const { return unrespondedMessagesMax; }
     */

    FinalStateSaver* getFinalStateSaver() const { return finalStateSaver_; }
    DataProvider*    getDataProvider() const { return dataProvider_; }
    const std::string& storeLocation() const { return storeLocation_; }

    /* ------------------------ Admin interface ------------------------ */

    virtual void startTaskProcessor();
    virtual void stopTaskProcessor();
    virtual bool isTaskProcessorRunning() {
        MutexGuard mg(startMon_);
        return started_;
    }
    virtual void startTaskScheduler() {
        scheduler_.start();
    }
    virtual void stopTaskScheduler() {
        scheduler_.stop();
    }
    virtual bool isTaskSchedulerRunning() {
        return scheduler_.isStarted();
    }

protected:

    virtual int Execute();

    void loadTask( unsigned taskId, smsc::util::config::ConfigView* taskConfig );

    /*
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

    // invoked from smscconnector
    virtual bool invokeProcessEvent(ThreadedTask* task) {
        return eventManager.startThread(task);
    }

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
    
    static void retryMessage( Task* task, uint64_t msgId ) {
        time_t retryTime = getRetryPolicies().getRetryTime( task->getInfo().retryPolicy.c_str(),0);
        if ( retryTime == 0 ) {
            retryTime = 60*60;
        }
        task->retryMessage(msgId,time(0)+retryTime);
    }
     */

    /* ------------------------ Admin interface ------------------------ */

    /*
    virtual void flushStatistics() {
        if (statistics) statistics->flushStatistics();
    }
    virtual void reloadSmscAndRegions();

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
     */

private:

    // std::auto_ptr< FinalStateSaver > finalStateSaver_;

    // ThreadManager taskManager;
    // ThreadManager eventManager;

    // TaskScheduler scheduler;    // for scheduled messages generation
    // static RetryPolicies retryPlcs;
    // DataProvider  provider;     // to obtain registered data source by key

    /*
    IntHash<Task *>  tasks;
    Mutex         tasksLock;

    Event       awake, exited;
    bool        bStarted, bNeedExit;
    Mutex       startLock;
    int         switchTimeout;
     */

    /*
    MessageSender*  messageSender;
    Mutex           messageSenderLock;

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
     */

private:
    smsc::logger::Logger* log_;
    TaskScheduler         scheduler_;
    TaskDispatcher        dispatcher_;
    MessageSender         sender_;
    std::string           storeLocation_;

    smsc::core::synchronization::EventMonitor startMon_;
    bool started_;
    bool stopping_;
    bool notified_;  // when some interesting external events incoming

    DataProvider*         dataProvider_;
    FinalStateSaver*      finalStateSaver_;
    std::string           address_;
};

}
}

#endif //SMSC_INFOSME_TASKPROCESSOR_H
