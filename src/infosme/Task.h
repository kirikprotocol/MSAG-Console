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

    struct Message
    {

    };
    struct StateInfo
    {

    };
    struct TaskInfo
    {
        std::string name;
        std::string dsId;
        bool enabled;
        int priority;
        
        TaskInfo()
            : name(""), dsId(""), enabled(true), priority(0) {};
        TaskInfo(const TaskInfo& info) 
            : name(info.name), dsId(info.dsId), 
                enabled(info.enabled), priority(info.priority) {}; 
        virtual ~TaskInfo() {};
        
        TaskInfo& operator=(const TaskInfo& info)
        {
            name = info.name;
            dsId = info.dsId;
            enabled = info.enabled;
            priority = info.priority;
            return *this;
        };
    };

    /* 
        TODO: 1) Задать наконец структуры StateInfo && Message
              2) Имплементить общую логику
    */                                                      
    class Task
    {
    friend class TaskGuard;
    private:
        
        Event           usersCountChanged;
        Mutex           usersCountLock;
        long            usersCount;
        
        Mutex           finalizingLock;
        bool            bFinalizing;

    protected:
        
        TaskInfo        info;
        DataSource*     dsOwn;
        DataSource*     dsInt;
        
        virtual void init(ConfigView* config) {
            __require__(config);
        };
        
        Task() : usersCount(0), bFinalizing(false), dsOwn(0), dsInt(0) {};
        virtual ~Task() {};

    public:
        
        Task(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt) 
            : usersCount(0), bFinalizing(false), dsOwn(dsOwn), dsInt(dsInt) 
        {
            __require__(dsOwn && dsInt);
            this->info = info; this->dsOwn = dsOwn; this->dsInt = dsInt;
        }
        Task(ConfigView* config, DataSource* dsOwn, DataSource* dsInt)
            : usersCount(0), bFinalizing(false), dsOwn(dsOwn), dsInt(dsInt) 
        {
            init(config);
        }
        
        void finalize()
        {
            {
                MutexGuard guard(finalizingLock);
                bFinalizing = true;
            }
            endProcess();
            
            while (true) {
                usersCountChanged.Wait(10);
                MutexGuard guard(usersCountLock);
                if (usersCount <= 0) break;
            }
            delete this;
        }
        bool isFinalizing() {
            MutexGuard guard(finalizingLock);
            return bFinalizing;
        }

        inline int getPriority() {
            return info.priority;
        }
        inline std::string getName() {
            return info.name;
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
         * Меняет состояние отправленного сообщения из спец.таблицы задачи.
         * Состояния: accepted, delivered, expired, ... ???
         * Выполняется из потока SmppTransport на ThreadPool'е по получению
         * submitResponce или deliveryReceipt.
         * Использует connection из внутреннего источника данных.
         *
         * @param info
         */
        void doNotifyMessage(StateInfo& info);

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
            task->usersCountChanged.Signal();
        }
        virtual ~TaskGuard() {
            if (!task) return;
            MutexGuard guard(task->usersCountLock);
            if (task->usersCount > 0) { 
                task->usersCount--;
                task->usersCountChanged.Signal();
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
        virtual void invokeDoNotifyMessage(Task* task, const StateInfo& info) = 0;
        
        virtual ~TaskInvokeAdapter() {};

    protected:

        TaskInvokeAdapter() {};
    };
    struct TaskContainerAdapter
    {
        virtual bool addTask(Task* task) = 0;
        virtual bool removeTask(std::string taskName) = 0;
        
        virtual TaskGuard getTask(std::string taskName) = 0;
        virtual TaskGuard getNextTask() = 0;
        
        virtual ~TaskContainerAdapter() {};

    protected:
        
        TaskContainerAdapter() {};
    };
    
}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

