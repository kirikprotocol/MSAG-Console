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
        TODO: 1) ������ ������� ��������� StateInfo && Message
              2) ������������ ����� ������
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
        
        Task() : usersCount(0), bFinalizing(false), dsOwn(0), dsInt(0) {};
        virtual ~Task() {};

        virtual void init(ConfigView* config) = 0;

    public:
        
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
         * ��������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'�
         * ���������� ��� connection'�: ���� �� ������,� ������ ����������� ��������� ������.
         */
        virtual void beginProcess() = 0;
        /**
         * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������.
         * ������������ �� ThreadPool'� �� ������� �� Scheduler'� ��� TaskProcessor'�.
         * ���������� Event ��� ���������� beginProcess() � ��� ����������.
         */
        virtual void endProcess() = 0;
        
        /**
         * ������ ��������� ������������� ��������� �� ����.������� ������.
         * ���������: accepted, delivered, expired, ... ???
         * ����������� �� ������ SmppTransport �� ThreadPool'� �� ���������
         * submitResponce ��� deliveryReceipt.
         * ���������� connection �� ����������� ��������� ������.
         *
         * @param info
         */
        virtual void doNotifyMessage(StateInfo& info) = 0;

        /**
         * ������������� ������� ��������� ��������� ��� �������� � ����.������� ������
         * ����������� endProcess(). ������� ��� ��������������� ���������.
         * ���������� connection �� ����������� ��������� ������.
         */
        virtual void dropAllMessages() = 0;
        
        /**
         * ���������� ��������� ��������� ��� �������� �� ����.������� ������
         * ����������� � �������� ������ TaskProcessor'�
         *
         * @param connection    �������� connection TaskProcessor'�
         *                      �� ����������� ��������� ������. (�����������)
         * @param message       ��������� ��������� ��� ��������
         * @return              false ���� ��������� ���.
         */
        virtual bool getNextMessage(Connection* connection, Message& message) = 0;
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

