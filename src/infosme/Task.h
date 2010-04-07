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

    /// collect tasks statistics, right now only the number of open messages
    void collectStatistics( unsigned& openMsgs );

    /**
     * Запускает процесс генерации сообщений для отправки в спец.таблицу задачи.
     * Выполняестся на ThreadPool'е по команде от Scheduler'а
     * Использует два connection'а: один из своего,а другой внутреннего источника данных.
     */
    bool beginGeneration(Statistics* statistics);
    /**
     * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи.
     * Выполняестся на ThreadPool'е по команде от Scheduler'а или TaskProcessor'а.
     * Выставляет Event для завершения beginGeneration() и ждёт завершения.
     */
    void endGeneration();

    /**
     * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи
     * посредством endGeneration(). Удаляет все сгенерированные сообщения.
     * Использует connection из внутреннего источника данных.
     */
    void dropNewMessages(Statistics* statistics);

    /**
     * Используется для восстановления сообщения из состояния WAIT в случае,
     * если submitResponce не пришёл при обрыве соединения с SMSC.
     * Выполняется при старте TaskProcessor'а.
     *
     * @param connection    основной connection TaskProcessor'а
     *                      из внутреннего источника данных. (оптимизация)
     */
    void resetWaiting();

    /**
     * Возвращает следующее сообщение для отправки из спец.таблицы задачи
     * Изменяет состояние выбранного сообщения на WAIT.
     * Выполняется в основном потоке TaskProcessor'а
     *
     * @param connection    основной connection TaskProcessor'а
     *                      из внутреннего источника данных. (оптимизация)
     * @param message       следующее сообщение для отправки
     * @return              false если сообщений нет.
     */
    bool getNextMessage(Message& message);

    /**
     * Финализирует/удаляет сообщение. Выполняется из TaskProcessor'а
     * Финализация/удаление регулируется флагом info.keepHistory
     *
     * @param msgId         идентификатор сообщения в таблице задачи.
     * @param state         финальное состояние сообщения
     * @param connection    connection от TaskProcessor'а
     *                      из внутреннего источника данных. (оптимизация)
     * @param smppStatus    The smpp status of the response (or the fake response)
     * @return true         если сообщение найдено и финализированно/удалено.
     */
    bool finalizeMessage( uint64_t msgId, MessageState state, int smppStatus );

    /**
     * Переводит сообщение в состояние NEW по получению deliveryReport Failed
     * или submitResponce Failed с временной ошибкой.
     *
     * @param msgId         идентификатор сообщения в таблице задачи.
     * @param nextTime      время следующей попытки доставки.
     * @param connection    connection от TaskProcessor'а
     *                      из внутреннего источника данных. (оптимизация)
     * @return true         если сообщение найдено и изменено
     */
    bool retryMessage(uint64_t msgId, time_t nextTime);

    /**
     * Переводит сообщение в состояние ENROUTE по получению submitResponce Ok
     * Сообщение должно быть в состоянии WAIT.
     * Выполняется из TaskProcessor'а
     *
     * @param msgId         идентификатор сообщения в таблице задачи.
     * @param connection    connection от TaskProcessor'а
     *                      из внутреннего источника данных. (оптимизация)
     * @return true         если сообщение найдено и изменено
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

