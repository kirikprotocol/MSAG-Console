#ifndef SMSC_INFO_SME_TASK
#define SMSC_INFO_SME_TASK

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <string>

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

#include "DataProvider.h"

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    using namespace smsc::util::templates;

    extern time_t parseDateTime(const char* str);
    extern time_t parseDate(const char* str);
    extern int    parseTime(const char* str);
    
    static const uint8_t MESSAGE_NEW_STATE          = 0;
    static const uint8_t MESSAGE_ENROUTE_STATE      = 10;
    static const uint8_t MESSAGE_DELIVERED_STATE    = 20;
    static const uint8_t MESSAGE_FAILED_STATE       = 30;

    typedef enum {
        NEW         = MESSAGE_NEW_STATE,
        ENROUTE     = MESSAGE_ENROUTE_STATE,
        DELIVERED   = MESSAGE_DELIVERED_STATE,
        FAILED      = MESSAGE_FAILED_STATE
    } MessageState;

    struct Message
    {
        uint64_t    id;
        std::string abonent;
        std::string message;
    };

    struct StateInfo
    {

    };
    struct TaskInfo
    {
        std::string id;
        std::string name;
        bool        enabled;
        int         priority;

        bool    retryOnFail;
        bool    replaceIfPresent;
        
        time_t  endDate;            // full date/time
        time_t  retryTime;          // only HH:mm:ss in seconds
        time_t  validityPeriod;     // only HH:mm:ss in seconds
        time_t  validityDate;       // full date/time
        time_t  activePeriodStart;  // only HH:mm:ss in seconds
        time_t  activePeriodEnd;    // only HH:mm:ss in seconds

        std::string dsId;
        std::string tablePrefix;
        std::string querySql;
        std::string msgTemplate;
        std::string svcType;        // specified if replaceIfPresent == true

        int     dsOwnTimeout, dsIntTimeout;
        int     dsUncommited;
        
        TaskInfo()
            : id(""), name(""), enabled(true), priority(0),
              retryOnFail(false), replaceIfPresent(false),
              endDate(-1), retryTime(-1), 
              validityPeriod(-1), validityDate(-1),
              activePeriodStart(-1), activePeriodEnd(-1),
              dsId(""), tablePrefix(""), querySql(""), msgTemplate(""), svcType(""),
              dsOwnTimeout(0), dsIntTimeout(0), dsUncommited(1) {};
        TaskInfo(const TaskInfo& info) 
            : id(info.id), name(info.name), enabled(info.enabled), priority(info.priority),
              retryOnFail(info.retryOnFail), replaceIfPresent(info.replaceIfPresent),
              endDate(info.endDate), retryTime(info.retryTime), 
              validityPeriod(info.validityPeriod), validityDate(info.validityDate),
              activePeriodStart(info.activePeriodStart), activePeriodEnd(info.activePeriodEnd),
              dsId(info.dsId), tablePrefix(info.tablePrefix), querySql(info.querySql), 
              msgTemplate(info.msgTemplate), svcType(info.svcType), 
              dsOwnTimeout(info.dsOwnTimeout), dsIntTimeout(info.dsIntTimeout), 
              dsUncommited(info.dsUncommited) {};
        
        virtual ~TaskInfo() {};
        
        TaskInfo& operator=(const TaskInfo& info)
        {
            id = info.id; name = info.name; enabled = info.enabled; priority = info.priority;
            retryOnFail = info.retryOnFail; replaceIfPresent = info.replaceIfPresent;
            endDate = info.endDate; retryTime = info.retryTime;
            validityPeriod = info.validityPeriod; validityDate = info.validityDate;
            activePeriodStart = info.activePeriodStart; 
            activePeriodEnd = info.activePeriodEnd;
            dsId = info.dsId; tablePrefix = info.tablePrefix; querySql = info.querySql;
            msgTemplate = info.msgTemplate; svcType = info.svcType;
            dsOwnTimeout = info.dsOwnTimeout; dsIntTimeout = info.dsIntTimeout;
            dsUncommited = info.dsUncommited;
            return *this;
        };
    };

    /*  TODO:   
            1) Разобраться с логикой getNextMessage & doNotifyMessage и 
            2) Задать наконец структуры StateInfo && Message
    */                                                      
    class Task
    {
    friend class TaskGuard;
    protected:
        
        log4cpp::Category  &logger;
        OutputFormatter*   formatter;

    private:
        
        Event       usersCountEvent;
        Mutex       usersCountLock;
        long        usersCount;
        
        Mutex       finalizingLock;
        bool        bFinalizing;
    
    protected:

        TaskInfo        info;
        DataSource*     dsOwn;
        DataSource*     dsInt;
        
        Event       processEndEvent;
        Event       inProcessEvent;
        Mutex       inProcessLock;
        Mutex       statementLock;
        Mutex       createTableLock;
        bool        bInProcess, bTableCreated;
        
        void createTable();
        char* prepareSqlCall(const char* sql);
        Statement* getStatement(Connection* connection, const char* id, const char* sql);

        virtual void init(ConfigView* config, std::string taskId, std::string tablePrefix);

        virtual ~Task();

    public:
        
        Task(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt);
        Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
             DataSource* dsOwn, DataSource* dsInt);
        
        void finalize()
        {
            {
                MutexGuard guard(finalizingLock);
                bFinalizing = true;
            }
            endProcess();
            
            while (true) {
                usersCountEvent.Wait(10);
                MutexGuard guard(usersCountLock);
                if (usersCount <= 0) break;
            }
            delete this;
        }
        bool isFinalizing() {
            MutexGuard guard(finalizingLock);
            return bFinalizing;
        }

        inline std::string getId() {
            return info.id;
        }
        inline std::string getName() {
            return info.name;
        }
        inline int getPriority() {
            return info.priority;
        }
        inline bool isEnabled() {
            return info.enabled;
        }
        inline bool setEnabled(bool enabled=true) {
            return info.enabled = enabled;
        }
        
        bool isInProcess();

        /**
         * Запускает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Выполняестся на ThreadPool'е по команде от Scheduler'а
         * Использует два connection'а: один из своего,а другой внутреннего источника данных.
         */
        void beginProcess();
        /**
         * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Выполняестся на ThreadPool'е по команде от Scheduler'а или TaskProcessor'а.
         * Выставляет Event для завершения beginProcess() и ждёт завершения.
         */
        void endProcess();
        
        /**
         * Производит обработку непосредственного ответа на отправленное сообщение.
         * Если сообщение не было принято в обработку серсис центром, то его состояние
         * меняется на FAILED, иначе сообщению присваивается номер пришедший от
         * серсис центра и сообщение ставится на ожидание в состоянии ENROUTE.
         * Использует connection из внутреннего источника данных.
         *
         * @param msgId         идентификатор сообщения в таблице задачи 
         * @param smscId        номер присвоенный smsc
         * @param acepted       признак, принято ли сообщение к доставке.
         */
        void doRespondMessage(uint64_t msgId, bool acepted, std::string smscId="");

        /**
         * Производит обработку отчёта о доставке сообщения. Если сообщение было 
         * доставлено, то меняет его состояние на DELIVERED, иначе, если установлен
         * флаг retryOnFail & retryTime, то переводит сообщение в состояние NEW и
         * выставляет новое значение в поле SEND_DATE, чистит поле MESSAGE_ID.
         * Если же флаг не выставлен, то переводит сообщение в состояние FAILED.
         *
         * @param smscId        номер присвоенный smsc
         * @param delivered     признак, доставленно ли сообщение
         */
        void doReceiptMessage(std::string smscId, bool delivered);

        /**
         * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи
         * посредством endProcess(). Удаляет все сгенерированные сообщения.
         * Использует connection из внутреннего источника данных.
         */
        void dropAllMessages();
        
        /**
         * Возвращает следующее сообщение для отправки из спец.таблицы задачи
         * Выполняется в основном потоке TaskProcessor'а
         *
         * @param connection    основной connection TaskProcessor'а
         *                      из внутреннего источника данных. (оптимизация)
         * @param message       следующее сообщение для отправки
         * @return              false если сообщений нет.
         */
        bool getNextMessage(Connection* connection, Message& message);
    };
    
    class TaskGuard
    {
    protected:
        
        Task* task;

    public:
        
        TaskGuard(Task* task) : task(task) {
            if (!task) return;
            MutexGuard guard(task->usersCountLock);
            task->usersCount++;
            task->usersCountEvent.Signal();
        }
        virtual ~TaskGuard() {
            if (!task) return;
            MutexGuard guard(task->usersCountLock);
            if (task->usersCount > 0) { 
                task->usersCount--;
                task->usersCountEvent.Signal();
            }
        }
        inline Task* get() {
            return task;
        }
    };
    
    struct TaskInvokeAdapter
    {
        virtual void invokeEndProcess(Task* task) = 0;
        virtual void invokeBeginProcess(Task* task) = 0;
        virtual void invokeDropAllMessages(Task* task) = 0;
        virtual void invokeDoRespondMessage(Task* task, uint64_t msgId, 
                                            bool acepted, std::string smscId="") = 0;
        virtual void invokeDoReceiptMessage(Task* task, std::string smscId, bool delivered) = 0;
        
        virtual ~TaskInvokeAdapter() {};

    protected:

        TaskInvokeAdapter() {};
    };
    struct TaskContainerAdapter
    {
        virtual bool addTask(Task* task) = 0;
        virtual bool removeTask(std::string taskId) = 0;
        virtual bool hasTask(std::string taskId) = 0;

        virtual TaskGuard getTask(std::string taskId) = 0;
        virtual TaskGuard getNextTask() = 0;
        
        virtual ~TaskContainerAdapter() {};

    protected:
        
        TaskContainerAdapter() {};
    };
    
}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

