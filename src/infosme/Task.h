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

#include <db/DataSource.h>

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
        int id;
        int priority;
        std::string name;

        TaskInfo()
            : id(0), priority(0), name("") {};
        TaskInfo(const TaskInfo& info) 
            : id(info.id), priority(info.priority), name(info.name) {}; 
        virtual ~TaskInfo() {};
        
        TaskInfo& operator=(const TaskInfo& info)
        {
            id = info.id;
            priority = info.priority;
            name = info.name;
            return *this;
        };
    };

    class Task
    {
    private:
    protected:
        
        TaskInfo    info;
        DataSource* ds; // Cсылка на внутренний источник данных shared with TaskProcessor

        Task() {};

    public:
        
        virtual ~Task() {};

        inline int getId() {
            return info.id;
        }
        inline int getPriority() {
            return info.priority;
        }
        inline std::string getName() {
            return info.name;
        }

        /**
         * Initializes Task
         *
         * @param info
         * @param ds
         */
        void init(TaskInfo& info, DataSource* ds) {
            this->info = info;
            this->ds = ds;
        };

        /**
         * Запускает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Выполняестся на ThreadPool'е по команде от Scheduler'а
         *
         * @param connection из DS DataProvider'а 
         */
        virtual void beginProcess(Connection* connection) = 0;
        /**
         * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Выполняестся на ThreadPool'е по команде от Scheduler'а 
         */
        virtual void endProcess() = 0;
        
        /**
         * Меняет состояние отправленного сообщения из спец.таблицы задачи.
         * Состояния: accepted, delivered, expired, ... ???
         * Выполняется из потока SmppTransport на ThreadPool'е по получению
         * submitResponce или deliveryReceipt.
         *
         * @param info
         */
        virtual void doNotifyMessage(StateInfo& info) = 0;

        /**
         * Останавливает процесс генерации сообщений для отправки в спец.таблицу задачи.
         * Удаляет все сгенерированные записи.
         *
         * @param connection основной connection TaskProcessor'а
         */
        virtual void dropAllMessages(Connection* connection) = 0;
        
        /**
         * Возвращает следующее сообщение для отправки из спец.таблицы задачи
         * Выполняется в основном потоке TaskProcessor'а
         *
         * @param connection основной connection TaskProcessor'а
         * @param message
         * @return 
         */
        virtual bool getNextMessage(Connection* connection, Message& message) = 0;
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

