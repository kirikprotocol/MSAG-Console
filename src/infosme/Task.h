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
    
    static const uint8_t MESSAGE_NEW_STATE          = 0;  // ����� ��� �������������� ���������
    static const uint8_t MESSAGE_WAIT_STATE         = 10; // ������� submitResponce
    static const uint8_t MESSAGE_ENROUTE_STATE      = 20; // � �������� ��������, ������� deliveryReciept
    static const uint8_t MESSAGE_DELIVERED_STATE    = 30; // �����������
    static const uint8_t MESSAGE_FAILED_STATE       = 40; // ������ ��� ����� � �������� 

    typedef enum {
        NEW         = MESSAGE_NEW_STATE,
        WAIT        = MESSAGE_WAIT_STATE,
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
        Mutex       createTableLock;
        bool        bInProcess, bTableCreated;
        
        char* prepareSqlCall(const char* sql);

        virtual void init(ConfigView* config, std::string taskId, std::string tablePrefix);
        virtual ~Task();

    public:
        
        Task(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt);
        Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
             DataSource* dsOwn, DataSource* dsInt);
        
        static Statement* getStatement(Connection* connection, 
                                       const char* id, const char* sql);
        void createTable();

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
        inline const TaskInfo& getInfo() {
            return info;
        }
        
        bool isInProcess();

        /**
         * ��������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'�
         * ���������� ��� connection'�: ���� �� ������,� ������ ����������� ��������� ������.
         */
        void beginProcess();
        /**
         * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'� ��� TaskProcessor'�.
         * ���������� Event ��� ���������� beginProcess() � ��� ����������.
         */
        void endProcess();
        
        /**
         * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������
         * ����������� endProcess(). ������� ��� ��������������� ���������.
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
        void resetWaiting(Connection* connection);

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
         * ��������� ��������� � ��������� NEW �� ��������� deliveryReport Failed
         * ��� submitResponce Failed � ��������� �������,
         * ���� � ������ ����� retryOnFail � ������������ retryTime 
         * (��������� ������ ���� � ��������� ENROUTE ��� WAIT).
         * ���� ����������� ����� ���������� ������ � ��������� �������
         * ������ ��������� �����, �� ��������� ������������ � ��������� FAILED.
         * 
         * @param connection    �������� connection TaskProcessor'�
         *                      �� ����������� ��������� ������. (�����������)
         * @param msgId         ������������� ��������� � ������� ������.
         * @return true         ���� ��������� ������� � �������� 
         */
        bool doRetry(Connection* connection, uint64_t msgId);

        /**
         * ��������� ��������� � ��������� ENROUTE �� ��������� submitResponce Ok
         * ��������� ������ ���� � ��������� WAIT.
         * ����������� �� TaskProcessor'�
         * 
         * @param connection    �������� connection TaskProcessor'�
         *                      �� ����������� ��������� ������. (�����������)
         * @param msgId         ������������� ��������� � ������� ������.
         * @return true         ���� ��������� ������� � �������� 
         */
        bool doEnroute(Connection* connection, uint64_t msgId);
        
        /**
         * ��������� ��������� � ��������� FAILED �� ��������� submitResponce Failed
         * ��� deliveryReport Failed.
         * ��������� ������ ���� � ��������� WAIT ��� ENROUTE.
         * ����������� �� TaskProcessor'�
         * 
         * @param connection    �������� connection TaskProcessor'�
         *                      �� ����������� ��������� ������. (�����������)
         * @param msgId         ������������� ��������� � ������� ������.
         * @return true         ���� ��������� ������� � �������� 
         */
        bool doFailed(Connection* connection, uint64_t msgId);
        
        /**
         * ��������� ��������� � ��������� DELIVERED �� ��������� submitResponce Ok
         * ��������� ������ ���� � ��������� ENROUTE.
         * ����������� �� TaskProcessor'�
         * 
         * @param connection    �������� connection TaskProcessor'�
         *                      �� ����������� ��������� ������. (�����������)
         * @param msgId         ������������� ��������� � ������� ������.
         * @return true         ���� ��������� ������� � �������� 
         */
        bool doDelivered(Connection* connection, uint64_t msgId);
        
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
        
        virtual ~TaskInvokeAdapter() {};

    protected:

        TaskInvokeAdapter() {};
    };
    struct TaskContainerAdapter
    {
        virtual bool putTask(Task* task) = 0;
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

