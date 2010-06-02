#ifndef SMSC_INFO_SME_TASK
#define SMSC_INFO_SME_TASK

#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <string>
#include <list>
#include <map>
#include <set>

#include <util/debug.h>
#include <logger/Logger.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <util/templates/Formatters.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include "DataProvider.h"
#include "Statistics.h"
#include "DateTime.h"
#include "TaskTypes.hpp"
#include "CsvStore.hpp"
#include "InfoSme_T_SearchCriterion.hpp"
#include "SpeedControl.h"

namespace smsc { 
namespace infosme {

using namespace smsc::core::buffers;
using namespace smsc::db;

using smsc::core::synchronization::Event;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::Mutex;

using smsc::logger::Logger;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

using namespace smsc::util::templates;

class ServicesForTask;

class Task 
{
    friend class TaskGuard;

    Task( uint32_t taskId,
          const std::string& location,
          const TaskInfo&    taskInfo,
          DataSource* dsOwn,
          ServicesForTask* finalStateSaver );

public:
    static uint32_t stringToTaskId( const char* taskId );

    /// to update task info
    void setInfo( const TaskInfo& info );

    std::string toString() const;

    // void update(ConfigView* config);

    void finalize(); // set finalizing and stop generation
    bool shutdown(); // Wait usages, cleanup waiting & delete task
    bool destroy();  // Wait usages, drop entire table & delete task

    inline bool canGenerateMessages() {
        return (dsOwn != 0);
    }
    inline bool isFinalizing() {
        MutexGuard guard(finalizingLock);
        return bFinalizing;
    }
    /*
    inline bool isInProcess() {
        MutexGuard guard(inProcessLock);
        return bInProcess;
    }
     */
    inline bool isInGeneration() {
        MutexGuard guard(inGenerationMon);
        return bInGeneration;
    }
    inline bool isGenerationSucceeded() {
        MutexGuard guard(inGenerationMon);
        return (!bInGeneration && info.bGenerationSuccess);
    }

    inline uint32_t getId()
    {
        return info.uid;
    }
    inline std::string getIdStr()
    {
        char buf[32];
        sprintf(buf,"%u",info.uid);
        return buf;
    }
    inline std::string getName() {
        return info.name;
    }
    inline std::string getAddress() {
        return info.address;
    }
    inline int getPriority() {
        return info.priority;
    }
    inline bool isEnabled() {
        MutexGuard guard(enableLock);
        return info.enabled;
    }
    inline bool setEnabled(bool enabled=true) {
        MutexGuard guard(enableLock);
        // if (!enabled) setInProcess(false);
        return info.enabled = enabled;
    }
    inline const TaskInfo& getInfo() {
        return info;
    }

    // bool isReady(time_t time, bool checkActivePeriod = true);
    unsigned isReady( unsigned curTime ) { return speedControl_.isReady(curTime); }
    bool isActive() const { return active_; }
    bool prefetchMessage( time_t now, int regionId );
    void getPrefetched( Message& msg );
    void checkActivity( time_t now, tm& localTime );

    /// collect tasks statistics, right now only the number of open messages
    void collectStatistics( unsigned& openMsgs );

    /**
     * ��������� ������� ��������� ��������� ��� �������� � ����.������� ������.
     * ������������ �� ThreadPool'� �� ������� �� Scheduler'�
     * ���������� ��� connection'�: ���� �� ������,� ������ ����������� ��������� ������.
     */
    bool beginGeneration(Statistics* statistics);
    /**
     * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������.
     * ������������ �� ThreadPool'� �� ������� �� Scheduler'� ��� TaskProcessor'�.
     * ���������� Event ��� ���������� beginGeneration() � ��� ����������.
     */
    void endGeneration();

    /**
     * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������
     * ����������� endGeneration(). ������� ��� ��������������� ���������.
     * ���������� connection �� ����������� ��������� ������.
     */
    void dropNewMessages(Statistics* statistics);

    /**
     * ������������ ��� �������������� ��������� �� ��������� WAIT � ������,
     * ���� submitResponce �� ������ ��� ������ ���������� � SMSC.
     * ����������� ��� ������ TaskProcessor'�.
     *
     * @param connection    �������� connection TaskProcessor'�
     *                      �� ����������� ��������� ������. (�����������)
     */
    void resetWaiting();

    /**
     * ���������� ��������� ��������� ��� �������� �� ����.������� ������
     * �������� ��������� ���������� ��������� �� WAIT.
     * ����������� � �������� ������ TaskProcessor'�
     *
     * @param connection    �������� connection TaskProcessor'�
     *                      �� ����������� ��������� ������. (�����������)
     * @param message       ��������� ��������� ��� ��������
     * @return              false ���� ��������� ���.
     */
    bool getNextMessage(Message& message);
    void closeProcessedFiles() {
        store.closeProcessedFiles(store.getLocalHour());
    }

    /**
     * ������������/������� ���������. ����������� �� TaskProcessor'�
     * �����������/�������� ������������ ������ info.keepHistory
     *
     * @param msgId         ������������� ��������� � ������� ������.
     * @param state         ��������� ��������� ���������
     * @param connection    connection �� TaskProcessor'�
     *                      �� ����������� ��������� ������. (�����������)
     * @param smppStatus    The smpp status of the response (or the fake response)
     * @return true         ���� ��������� ������� � ���������������/�������.
     */
    bool finalizeMessage( uint64_t msgId, MessageState state, int smppStatus );

    /**
     * ��������� ��������� � ��������� NEW �� ��������� deliveryReport Failed
     * ��� submitResponce Failed � ��������� �������.
     *
     * @param msgId         ������������� ��������� � ������� ������.
     * @param nextTime      ����� ��������� ������� ��������.
     * @param connection    connection �� TaskProcessor'�
     *                      �� ����������� ��������� ������. (�����������)
     * @return true         ���� ��������� ������� � ��������
     */
    bool retryMessage(uint64_t msgId, time_t nextTime);

    /**
     * ��������� ��������� � ��������� ENROUTE �� ��������� submitResponce Ok
     * ��������� ������ ���� � ��������� WAIT.
     * ����������� �� TaskProcessor'�
     *
     * @param msgId         ������������� ��������� � ������� ������.
     * @param connection    connection �� TaskProcessor'�
     *                      �� ����������� ��������� ������. (�����������)
     * @return true         ���� ��������� ������� � ��������
     */
    bool enrouteMessage(uint64_t msgId);

    bool insertDeliveryMessage(uint8_t msgState,
                               const std::string& address,
                               time_t messageDate,
                               const std::string& msg,
                               const std::string& userData );

    bool changeDeliveryMessageInfoByRecordId(uint8_t msgState,
                                             time_t unixTime,
                                             const std::string& recordId,
                                             uint64_t& newMsgId);

    bool changeDeliveryMessageInfoByCompositCriterion(uint8_t msgState,
                                                      time_t unixTime,
                                                      const InfoSme_T_SearchCriterion& searchCrit);

    bool deleteDeliveryMessageByRecordId(const std::string& recordId);

    bool deleteDeliveryMessagesByCompositCriterion(const InfoSme_T_SearchCriterion& searchCrit);

    Array<std::string> selectDeliveryMessagesByCompositCriterion(const InfoSme_T_SearchCriterion& searchCrit);

    void endDeliveryMessagesGeneration();

    bool changeDeliveryTextMessageByCompositCriterion(const std::string& newTextMsg,
                                                      const InfoSme_T_SearchCriterion& searchCrit);

    // messages to work w/ cache
    void putToSuspendedMessagesQueue(const Message& suspendedMessage);
    void resetSuspendedRegions();

protected:
    bool doesMessageConformToCriterion(uint8_t state,const Message&, const InfoSme_T_SearchCriterion& searchCrit);
    void destroy_InfoSme_T_Storage();

private:
    // void  doFinalization();

    void  trackIntegrity(bool clear=true, bool del=true);

    // void init(ConfigView* config, uint32_t taskId);
    ~Task();

    /*
    inline bool setInProcess(bool inProcess) {
        MutexGuard guard(inProcessLock);
        bInProcess = inProcess;
        return bInProcess;
    }
     */

    // bool fetchMessageFromCache(Message& message);

    void doSuspendMessage( const Message& msg );

    bool changeUsage( bool v ) {
        unsigned uc;
        {
            MutexGuard mg(lock_);
            if ( v ) {
                ++usersCount;
            } else {
                if (usersCount > 0) --usersCount;
            }
            uc = usersCount;
        }
        // smsc_log_debug(logger,"task %u count%c1=%u",info.uid,v?'+':'-',usersCount);
        return (usersCount == 0);
    }

protected:
    typedef std::list< Message > MessageList;
    typedef std::map< int, MessageList > RegionMap;

    smsc::logger::Logger *logger;
    OutputFormatter*   formatter;
    ServicesForTask*   finalStateSaver_;

    Event       usersCountEvent;
    Mutex       lock_;
    unsigned    usersCount;

    Mutex       finalizingLock;
    bool        bFinalizing, bSelectedAll;
    bool        active_;

    TaskInfo        info;
    DataSource*     dsOwn;

    bool         prefetched_;
    Message      prefetch_;
    unsigned     messagesInCache_;

    RegionMap    regionMap_;
    CsvStore     store;
    SpeedControl<> speedControl_;

    Mutex       createTableLock, enableLock;
    EventMonitor inGenerationMon;
    // Mutex       inProcessLock;
    // bool        bInProcess, bInGeneration;
    bool        bInGeneration;
    bool        infoSme_T_storageWasDestroyed;

};


class TaskGuard
{
public:
    inline TaskGuard() : task_(0) {}
    inline TaskGuard( Task* task ) : task_(task) {
        if (task_) task_->changeUsage(true);
    }
    inline ~TaskGuard() {
        if (task_ && task_->changeUsage(false) ) delete task_;
    }
    inline TaskGuard( const TaskGuard& tg ) : task_(tg.task_) {
        if (task_) task_->changeUsage(true);
    }
    inline TaskGuard& operator = ( const TaskGuard& tg ) {
        if ( tg.task_ != task_ ) {
            if (task_ && task_->changeUsage(false)) delete task_;
            task_ = tg.task_;
            if (task_) task_->changeUsage(true);
        }
        return *this;
    }
    inline Task* get() const {
        return task_;
    }
    inline Task* release() {
        Task* task = task_;
        task_ = 0;
        if (task) task->changeUsage(false);
        return task;
    }

    inline Task* operator -> () const {
        return task_;
    }

    Task* create( uint32_t              id,
                  const std::string&    location,
                  const TaskInfo&       info,
                  smsc::db::DataSource* dataSource,
                  ServicesForTask*      saver ) {
        if ( task_ && task_->changeUsage(false) ) { delete task_; task_ = 0; }
        task_ = new Task(id,location,info,dataSource,saver);
        task_->changeUsage(true);
        return task_;
    }

private:
    Task* task_;
};

}
}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

