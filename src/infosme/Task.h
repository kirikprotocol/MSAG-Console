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

    class Task;

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

    struct TaskInvokeAdapter
    {
        virtual void invokeEndProcess(Task* task) = 0;
        virtual void invokeBeginProcess(Task* task, Connection* connection) = 0;
        virtual void invokeDoNotifyMessage(Task* task, const StateInfo& info) = 0;
        virtual void invokeDropAllMessages(Task* task, Connection* connection) = 0;
    
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
    struct TaskProcessorAdapter
    {
        virtual TaskInvokeAdapter& getTaskInvokeAdapter() = 0;
        virtual TaskContainerAdapter& getTaskContainerAdapter() = 0;
        virtual DataProvider& getDataProvider() = 0;
        virtual DataSource* getInternalDataSource() = 0;
    
        virtual ~TaskProcessorAdapter() {};

     protected:

        TaskProcessorAdapter() {};
    };
    
    /* 
        TODO: 1) Разобраться на что ссылается таска, на DataProvider со своим DS
              и/или на DS, шареный с TaskProcessor'ом ??? 
              2) Вообще, нужно-ли ссылаться на них а не получать готовые извне ???
              3) Пересмотреть сигнатуру методов и соответственно поменять TaskRunner
              4) Предусмотреть загрузку таски из конфига (+ позволить это делать виртуально) 
              5) Задать наконец структуры StateInfo && Message
              5) Нужен ли таске интерфейс TaskProcessor'а ???
    */                                                      
    class Task
    {
    private:
    protected:
        
        TaskInfo                info;
        TaskProcessorAdapter*   processor;

        Task() : processor(0) {};

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
        inline std::string getDSId() {
            return info.dsId;
        }

        /**
         * Initializes Task
         * 
         * @param info
         * @param processor
         */
        virtual void init(TaskInfo& info, TaskProcessorAdapter* processor) {
            __require__(processor);
            this->info = info;
            this->processor = processor;
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

