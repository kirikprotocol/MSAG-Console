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
#include "DateTime.h"

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;
    
    using smsc::logger::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    using namespace smsc::util::templates;

    static const uint8_t MESSAGE_NEW_STATE          =  0; // ����� ��� �������������� ���������
    static const uint8_t MESSAGE_WAIT_STATE         = 10; // ������� submitResponce
    static const uint8_t MESSAGE_ENROUTE_STATE      = 20; // � �������� ��������, ������� deliveryReciept
    static const uint8_t MESSAGE_DELIVERED_STATE    = 30; // ����������
    static const uint8_t MESSAGE_EXPIRED_STATE      = 40; // �� ����������, ����� ����� �������
    static const uint8_t MESSAGE_FAILED_STATE       = 50; // �� ����������, ��������� ������ ��� ��������
    
    typedef enum {
        NEW         = MESSAGE_NEW_STATE, 
        WAIT        = MESSAGE_WAIT_STATE,
        ENROUTE     = MESSAGE_ENROUTE_STATE,
        DELIVERED   = MESSAGE_DELIVERED_STATE,
        EXPIRED     = MESSAGE_EXPIRED_STATE,
        FAILED      = MESSAGE_FAILED_STATE
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

        bool    retryOnFail, replaceIfPresent, trackIntegrity, transactionMode, keepHistory;
        
        time_t  endDate;            // full date/time
        time_t  retryTime;          // only HH:mm:ss in seconds
        time_t  validityPeriod;     // only HH:mm:ss in seconds
        time_t  validityDate;       // full date/time
        time_t  activePeriodStart;  // only HH:mm:ss in seconds
        time_t  activePeriodEnd;    // only HH:mm:ss in seconds
        
        WeekDaysSet activeWeekDays; // Mon, Tue ...

        std::string dsId;
        std::string tablePrefix;
        std::string querySql;
        std::string msgTemplate;
        std::string svcType;        // specified if replaceIfPresent == true
        std::string address;

        int     dsTimeout, dsUncommitedInProcess, dsUncommitedInGeneration;
        int     messagesCacheSize, messagesCacheSleep;
        
        TaskInfo()
            : id(""), name(""), enabled(true), priority(0),
              retryOnFail(false), replaceIfPresent(false), 
              trackIntegrity(false), transactionMode(false), keepHistory(false),
              endDate(-1), retryTime(-1), validityPeriod(-1), validityDate(-1),
              activePeriodStart(-1), activePeriodEnd(-1), activeWeekDays(0),
              dsId(""), tablePrefix(""), querySql(""), msgTemplate(""), svcType(""), address(""),
              dsTimeout(0), dsUncommitedInProcess(1), dsUncommitedInGeneration(1), 
              messagesCacheSize(100), messagesCacheSleep(0) {};
        TaskInfo(const TaskInfo& info) 
            : id(info.id), name(info.name), enabled(info.enabled), priority(info.priority),
              retryOnFail(info.retryOnFail), replaceIfPresent(info.replaceIfPresent),
              trackIntegrity(info.trackIntegrity), transactionMode(info.transactionMode), 
              keepHistory(info.keepHistory), endDate(info.endDate), retryTime(info.retryTime), 
              validityPeriod(info.validityPeriod), validityDate(info.validityDate),
              activePeriodStart(info.activePeriodStart), activePeriodEnd(info.activePeriodEnd),
              activeWeekDays(info.activeWeekDays),
              dsId(info.dsId), tablePrefix(info.tablePrefix), querySql(info.querySql), 
              msgTemplate(info.msgTemplate), svcType(info.svcType), address(info.address),
              dsTimeout(info.dsTimeout),
              dsUncommitedInProcess(info.dsUncommitedInProcess),
              dsUncommitedInGeneration(info.dsUncommitedInGeneration),
              messagesCacheSize(info.messagesCacheSize), 
              messagesCacheSleep(info.messagesCacheSleep) {};
        
        virtual ~TaskInfo() {};
        
        TaskInfo& operator=(const TaskInfo& info)
        {
            id = info.id; name = info.name; enabled = info.enabled; priority = info.priority;
            retryOnFail = info.retryOnFail; replaceIfPresent = info.replaceIfPresent;
            trackIntegrity = info.trackIntegrity; transactionMode = info.transactionMode; 
            keepHistory = info.keepHistory; endDate = info.endDate; retryTime = info.retryTime;
            validityPeriod = info.validityPeriod; validityDate = info.validityDate;
            activePeriodStart = info.activePeriodStart; 
            activePeriodEnd = info.activePeriodEnd;
            activeWeekDays = info.activeWeekDays;
            dsId = info.dsId; tablePrefix = info.tablePrefix; querySql = info.querySql;
            msgTemplate = info.msgTemplate; svcType = info.svcType; address = info.address;
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
        
        smsc::logger::Logger *logger;
        OutputFormatter*   formatter;

    private:
        
        Event       usersCountEvent;
        Mutex       usersCountLock;
        long        usersCount;
        
        Mutex       finalizingLock;
        bool        bFinalizing, bSelectedAll;

    protected:

        TaskInfo        info;
        DataSource*     dsOwn;
        DataSource*     dsInt;
        
        Mutex       createTableLock, enableLock;
        Event       generationEndEvent;
        Mutex       inGenerationLock, inProcessLock;
        bool        bInProcess, bInGeneration, bGenerationSuccess;

        Mutex           messagesCacheLock;
        Array<Message>  messagesCache;
        time_t          lastMessagesCacheEmpty;
        
        void  doFinalization();
        char* prepareSqlCall(const char* sql);
        char* prepareDoubleSqlCall(const char* sql);
        void  trackIntegrity(bool clear=true, bool del=true, Connection* connection=0);

        virtual void init(ConfigView* config, std::string taskId, std::string tablePrefix);
        virtual ~Task();

        inline bool setInProcess(bool inProcess) {
            MutexGuard guard(inProcessLock);
            bInProcess = inProcess;
            return bInProcess;
        }
    
    public:
        
        int         currentPriorityFrameCounter;

        /*Task(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt);*/
        Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
             DataSource* dsOwn, DataSource* dsInt);
        
        void createTable(); // throws Exception
        void dropTable();   // throws Exception

        void finalize(); // Wait usages & delete task
        bool shutdown(); // Wait usages, cleanup waiting & delete task
        bool destroy();  // Wait usages, drop entire table & delete task

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
        inline bool isGenerationSucceeded() {
            MutexGuard guard(inGenerationLock);
            return (!bInGeneration && bGenerationSuccess);
        }

        inline std::string getId() {
            return info.id;
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
         * ������������/������� ���������. ����������� �� TaskProcessor'�
         * �����������/�������� ������������ ������ info.keepHistory
         * 
         * @param msgId         ������������� ��������� � ������� ������.
         * @param state         ��������� ��������� ���������              
         * @param connection    connection �� TaskProcessor'� 
         *                      �� ����������� ��������� ������. (�����������)
         * @return true         ���� ��������� ������� � ���������������/�������. 
         */
        bool finalizeMessage(uint64_t msgId, MessageState state, Connection* connection=0);
        
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
    
}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

