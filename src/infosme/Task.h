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
#include "Statistics.h"

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
    
    static const uint8_t MESSAGE_NEW_STATE          = 0;  // ����� ��� �������������� ���������
    static const uint8_t MESSAGE_WAIT_STATE         = 10; // ������� submitResponce
    static const uint8_t MESSAGE_ENROUTE_STATE      = 20; // � �������� ��������, ������� deliveryReciept

    typedef enum {
        NEW         = MESSAGE_NEW_STATE,
        WAIT        = MESSAGE_WAIT_STATE,
        ENROUTE     = MESSAGE_ENROUTE_STATE,
    } MessageState;

    struct Message
    {
        uint64_t    id;
        std::string abonent;
        std::string message;

        Message(uint64_t id=0, std::string abonent="", std::string message="")
            : id(id), abonent(abonent), message(message) {};
        virtual ~Message() {};
    };

    struct TaskInfo
    {
        std::string id;
        std::string name;
        bool        enabled;
        int         priority;

        bool    retryOnFail;
        bool    replaceIfPresent;
        bool    transactionMode;
        
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

        int     dsTimeout, dsUncommitedInProcess, dsUncommitedInGeneration;
        int     messagesCacheSize, messagesCacheSleep;
        
        TaskInfo()
            : id(""), name(""), enabled(true), priority(0),
              retryOnFail(false), replaceIfPresent(false), transactionMode(false), 
              endDate(-1), retryTime(-1), validityPeriod(-1), validityDate(-1),
              activePeriodStart(-1), activePeriodEnd(-1),
              dsId(""), tablePrefix(""), querySql(""), msgTemplate(""), svcType(""),
              dsTimeout(0), dsUncommitedInProcess(1), dsUncommitedInGeneration(1), 
              messagesCacheSize(100), messagesCacheSleep(0) {};
        TaskInfo(const TaskInfo& info) 
            : id(info.id), name(info.name), enabled(info.enabled), priority(info.priority),
              retryOnFail(info.retryOnFail), replaceIfPresent(info.replaceIfPresent),
              transactionMode(info.transactionMode), 
              endDate(info.endDate), retryTime(info.retryTime), 
              validityPeriod(info.validityPeriod), validityDate(info.validityDate),
              activePeriodStart(info.activePeriodStart), activePeriodEnd(info.activePeriodEnd),
              dsId(info.dsId), tablePrefix(info.tablePrefix), querySql(info.querySql), 
              msgTemplate(info.msgTemplate), svcType(info.svcType), dsTimeout(info.dsTimeout),
              dsUncommitedInProcess(info.dsUncommitedInProcess),
              dsUncommitedInGeneration(info.dsUncommitedInGeneration),
              messagesCacheSize(info.messagesCacheSize), 
              messagesCacheSleep(info.messagesCacheSleep) {};
        
        virtual ~TaskInfo() {};
        
        TaskInfo& operator=(const TaskInfo& info)
        {
            id = info.id; name = info.name; enabled = info.enabled; priority = info.priority;
            retryOnFail = info.retryOnFail; replaceIfPresent = info.replaceIfPresent;
            transactionMode = info.transactionMode; 
            endDate = info.endDate; retryTime = info.retryTime;
            validityPeriod = info.validityPeriod; validityDate = info.validityDate;
            activePeriodStart = info.activePeriodStart; 
            activePeriodEnd = info.activePeriodEnd;
            dsId = info.dsId; tablePrefix = info.tablePrefix; querySql = info.querySql;
            msgTemplate = info.msgTemplate; svcType = info.svcType;
            dsTimeout = info.dsTimeout;
            dsUncommitedInProcess = info.dsUncommitedInProcess;
            dsUncommitedInGeneration = info.dsUncommitedInGeneration;
            messagesCacheSize = info.messagesCacheSize;
            messagesCacheSleep = info.messagesCacheSleep;
            return *this;
        };
    };

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
        
        Mutex       createTableLock, enableLock;
        Event       generationEndEvent;
        Mutex       inGenerationLock, inProcessLock;
        bool        bInProcess, bInGeneration;

        Mutex           messagesCacheLock;
        Array<Message>  messagesCache;
        time_t          lastMessagesCacheEmpty;
        
        char* prepareSqlCall(const char* sql);
        char* prepareDoubleSqlCall(const char* sql);

        virtual void init(ConfigView* config, std::string taskId, std::string tablePrefix);
        virtual ~Task();
        
        void doFinalization()
        {
            {
                MutexGuard guard(finalizingLock);
                bFinalizing = true;
            }
            endGeneration();
            
            while (true) {
                usersCountEvent.Wait(10);
                MutexGuard guard(usersCountLock);
                if (usersCount <= 0) break;
            }
        }

        inline bool setInProcess(bool inProcess) {
            MutexGuard guard(inProcessLock);
            bInProcess = inProcess;
            return bInProcess;
        }
    
    public:
        
        int         currentPriorityFrameCounter;

        Task(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt);
        Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
             DataSource* dsOwn, DataSource* dsInt);
        
        bool createTable();
        bool dropTable();

        bool destroy() {
            doFinalization();
            bool result = dropTable();
            delete this;
            return result;
        }
        void finalize() {
            doFinalization();
            delete this;
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
            MutexGuard guard(inGenerationLock);
            return bInGeneration;
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
            MutexGuard guard(enableLock);
            return info.enabled;
        }
        inline bool setEnabled(bool enabled=true) {
            MutexGuard guard(enableLock);
            return info.enabled = enabled;
        }
        inline const TaskInfo& getInfo() {
            return info;
        }
        
        bool isReady(time_t time);
        

        /**
         * ��������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'�
         * ���������� ��� connection'�: ���� �� ������,� ������ ����������� ��������� ������.
         */
        void beginGeneration(Statistics* statistics);
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
        void dropAllMessages();
        
        /**
         * ������������ ��� �������������� ��������� �� ��������� WAIT � ������,
         * ���� submitResponce �� ������ ��� ������ ���������� � SMSC.
         * ����������� ��� ������ TaskProcessor'�.
         *
         * @param connection    �������� connection TaskProcessor'�
         *                      �� ����������� ��������� ������. (�����������)
         */
        void resetWaiting(Connection* connection=0);

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
        bool getNextMessage(Connection* connection, Message& message);
        
        /**
         * ������� ���������. ����������� �� TaskProcessor'�
         * 
         * @param msgId         ������������� ��������� � ������� ������.
         * @param connection    connection �� TaskProcessor'� 
         *                      �� ����������� ��������� ������. (�����������)
         * @return true         ���� ��������� ������� � �������. 
         */
        bool deleteMessage(uint64_t msgId, Connection* connection=0);
        
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
        bool retryMessage(uint64_t msgId, time_t nextTime, Connection* connection=0);

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
        bool enrouteMessage(uint64_t msgId, Connection* connection=0);
        
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
        virtual ~TaskGuard() {
            changeTaskCounter(false);
        }
    
        inline Task* get() {
            return task;
        }
    };
    
}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

