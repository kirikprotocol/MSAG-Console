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
        DataSource* ds; // C����� �� ���������� �������� ������ shared with TaskProcessor

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
         * ��������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'�
         *
         * @param connection �� DS DataProvider'� 
         */
        virtual void beginProcess(Connection* connection) = 0;
        /**
         * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'� 
         */
        virtual void endProcess() = 0;
        
        /**
         * ������ ��������� ������������� ��������� �� ����.������� ������.
         * ���������: accepted, delivered, expired, ... ???
         * ����������� �� ������ SmppTransport �� ThreadPool'� �� ���������
         * submitResponce ��� deliveryReceipt.
         *
         * @param info
         */
        virtual void doNotifyMessage(StateInfo& info) = 0;

        /**
         * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������� ��� ��������������� ������.
         *
         * @param connection �������� connection TaskProcessor'�
         */
        virtual void dropAllMessages(Connection* connection) = 0;
        
        /**
         * ���������� ��������� ��������� ��� �������� �� ����.������� ������
         * ����������� � �������� ������ TaskProcessor'�
         *
         * @param connection �������� connection TaskProcessor'�
         * @param message
         * @return 
         */
        virtual bool getNextMessage(Connection* connection, Message& message) = 0;
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

