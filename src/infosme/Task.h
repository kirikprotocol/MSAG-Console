#ifndef SMSC_INFO_SME_TASK
#define SMSC_INFO_SME_TASK

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <string>
#include <list>
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

class FinalStateSaver;

class Task 
{
    friend class TaskGuard;

public:

    static uint32_t stringToTaskId( const char* taskId );

    Task(ConfigView* config,
         uint32_t taskId,
         std::string location,
         DataSource* dsOwn,
         FinalStateSaver* finalStateSaver );

    void update(ConfigView* config);

    void finalize(); // Wait usages & delete task
    bool shutdown(); // Wait usages, cleanup waiting & delete task
    bool destroy();  // Wait usages, drop entire table & delete task

    inline bool canGenerateMessages() {
        return (dsOwn != 0);
    }
    inline bool isFinalizing() {
        MutexGuard guard(finalizingLock);
        return bFinalizing;
    }
    inline bool isInProcess() {
        MutexGuard guard(inProcessLock);
        return bInProcess;
    }
    inline bool isInGeneration() {
        MutexGuard guard(inGenerationMon);
        return bInGeneration;
    }
    inline bool isGenerationSucceeded() {
        MutexGuard guard(inGenerationMon);
        return (!bInGeneration && bGenerationSuccess);
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
        if (!enabled) setInProcess(false);
        return info.enabled = enabled;
    }
    inline const TaskInfo& getInfo() {
        return info;
    }

    bool isReady(time_t time, bool checkActivePeriod = true);

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

public:
    int         currentPriorityFrameCounter;

protected:

    smsc::logger::Logger *logger;
    OutputFormatter*   formatter;
    FinalStateSaver*   finalStateSaver_;

    Event       usersCountEvent;
    Mutex       usersCountLock;
    long        usersCount;

    Mutex       finalizingLock;
    bool        bFinalizing, bSelectedAll;

    TaskInfo        info;
    DataSource*     dsOwn;

    CsvStore store;

    Mutex       createTableLock, enableLock;
    EventMonitor inGenerationMon;
    Mutex       inProcessLock;
    bool        bInProcess, bInGeneration, bGenerationSuccess;
    bool        infoSme_T_storageWasDestroyed;

    // cache
    class MessageRegionCache;
        /*
        Mutex           messagesCacheLock;
        std::list<Message> messagesCache;

        typedef std::set<std::string>  suspendedRegions_t;
        suspendedRegions_t                                 _suspendedRegions;
        std::list<Message>::iterator                       _messagesCacheIter;
        time_t          lastMessagesCacheEmpty;
         */
    MessageRegionCache* messageCache_;

    void  doFinalization();

    void  trackIntegrity(bool clear=true, bool del=true);

    void init(ConfigView* config, uint32_t taskId);
    ~Task();

    inline bool setInProcess(bool inProcess) {
        MutexGuard guard(inProcessLock);
        bInProcess = inProcess;
        return bInProcess;
    }

    bool fetchMessageFromCache(Message& message);
};


class TaskGuard
{
    private:

        TaskGuard& operator=(const TaskGuard& tg) {
            changeTaskCounter(false);
            task = tg.task;
            changeTaskCounter(true);
            return *this;
        };

    protected:

        Task* task;

        inline void changeTaskCounter(bool increment) {
           if (!task) return;
           MutexGuard guard(task->usersCountLock);
           if (increment) task->usersCount++;
           else task->usersCount--;
           task->usersCountEvent.Signal();
        }

    public:

        TaskGuard(Task* task=0) : task(task) {
            changeTaskCounter(true);
        }
        TaskGuard(const TaskGuard& tg) : task(tg.task) {
            changeTaskCounter(true);
        }
        virtual ~TaskGuard() {
            changeTaskCounter(false);
        }

        inline Task* get() {
            return task;
        }
};

}
}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

