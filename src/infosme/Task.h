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
        int         priority;

        TaskInfo()
            : name(""), dsId(""), priority(0) {};
        TaskInfo(const TaskInfo& info) 
            : name(info.name), dsId(info.dsId), priority(info.priority) {}; 
        virtual ~TaskInfo() {};
        
        TaskInfo& operator=(const TaskInfo& info)
        {
            name = info.name;
            dsId = info.dsId;
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
    private:
    protected:
        
        TaskInfo        info;
        DataSource*     dsOwn;
        DataSource*     dsInt;
        
        Task() : dsOwn(0), dsInt(0) {};

        virtual void init(ConfigView* config) = 0;

    public:
        
        virtual ~Task() {};

        inline int getPriority() {
            return info.priority;
        }
        inline std::string getName() {
            return info.name;
        }
        
        void init(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt)
        {
            __require__(dsOwn && dsInt);
            this->info = info; this->dsOwn = dsOwn; this->dsInt = dsInt;
        };
        
        void init(ConfigView* config, DataSource* dsOwn, DataSource* dsInt)
        {
            __require__(config);

            TaskInfo taskInfo;
            // TODO: fill taskInfo from config here !!
            
            init(taskInfo, dsOwn, dsInt);
            init(config);
        };
        
        virtual bool isInProcess() = 0;

        /**
         * Запускает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Выполняестся на ThreadPool'е по команде от Scheduler'а
         * Использует два connection'а: один из своего,а другой внутреннего источника данных.
         */
        virtual void beginProcess() = 0;
        /**
         * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Выполняестся на ThreadPool'е по команде от Scheduler'а или TaskProcessor'а.
         * Выставляет Event для завершения beginProcess() и ждёт завершения.
         */
        virtual void endProcess() = 0;
        
        /**
         * Меняет состояние отправленного сообщения из спец.таблицы задачи.
         * Состояния: accepted, delivered, expired, ... ???
         * Выполняется из потока SmppTransport на ThreadPool'е по получению
         * submitResponce или deliveryReceipt.
         * Использует connection из внутреннего источника данных.
         *
         * @param info
         */
        virtual void doNotifyMessage(StateInfo& info) = 0;

        /**
         * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи
         * посредством endProcess(). Удаляет все сгенерированные сообщения.
         * Использует connection из внутреннего источника данных.
         */
        virtual void dropAllMessages() = 0;
        
        /**
         * Возвращает следующее сообщение для отправки из спец.таблицы задачи
         * Выполняется в основном потоке TaskProcessor'а
         *
         * @param connection    основной connection TaskProcessor'а
         *                      из внутреннего источника данных. (оптимизация)
         * @param message       следующее сообщение для отправки
         * @return              false если сообщений нет.
         */
        virtual bool getNextMessage(Connection* connection, Message& message) = 0;
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
        virtual bool  addTask(Task* task) = 0;
        virtual bool  removeTask(std::string taskName) = 0;
        virtual Task* getTask(std::string taskName) = 0;
        
        virtual ~TaskContainerAdapter() {};

    protected:
        
        TaskContainerAdapter() {};
    };
    
    class TaskFactory
    {
    protected:

        static Hash<TaskFactory *>*   registry;
        
        TaskFactory() {};
        virtual ~TaskFactory() {};
        
        virtual Task* createTask() = 0;
        
    public:

        static void registerFactory(TaskFactory* tf, const char* type)
        {
            if (!registry) registry = new Hash<TaskFactory *>();
            registry->Insert(type, tf);
        };
        
        static Task* createTask(const char* type)
        {
            TaskFactory* tf = (registry) ? 
                ((registry->Exists(type)) ? registry->Get(type):0):0;
            return ((tf) ? tf->createTask():0);
        };
    };

}}

#endif //SMSC_INFO_SME_TASK_PROCESSOR

