
#include "TaskScheduler.h"

namespace smsc { namespace infosme 
{

TaskScheduler::TaskScheduler()
    : Thread(), logger(Logger::getCategory("smsc.infosme.TaskScheduler")),
        bStarted(false), bNeedExit(false)
{
}
TaskScheduler::~TaskScheduler()
{
    MutexGuard guard(schedulesLock);
    
    this->Stop();

    char* key = 0; Schedule* schedule = 0; schedules.First();
    while (schedules.Next(key, schedule))
        if (schedule) delete schedule;
}
void TaskScheduler::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        logger.info("Starting ...");
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
        logger.info("Started.");
    }
}
void TaskScheduler::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        logger.info("Stopping ...");
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
        logger.info("Stoped.");
    }
}

int TaskScheduler::Execute()
{
    const int SERVICE_SLEEP = 3600000;
    const int SERVICE_PAUSE = 1000;

    while (!bNeedExit)
    {
        int toSleep = -1;
        {
            MutexGuard guard(schedulesLock);
            time_t scheduleTime = -1;
            getNextSchedule(scheduleTime);
            if (scheduleTime > 0) toSleep = scheduleTime-time(NULL);
        }
        
        if (toSleep < 0) awake.Wait(SERVICE_PAUSE);
        else
        {
            while (toSleep > 0 && !bNeedExit) 
            {
                if (toSleep > SERVICE_SLEEP) {
                    toSleep -= SERVICE_SLEEP;
                    awake.Wait(SERVICE_SLEEP);
                } 
                else {
                    awake.Wait(toSleep);
                    break;
                }
            }
        }
        if (bNeedExit) break;
        
        Hash<bool> tasks;
        {
            MutexGuard guard(schedulesLock);
            time_t scheduleTime = -1;
            Schedule* schedule = getNextSchedule(scheduleTime);
            if (!schedule || scheduleTime > time(NULL)) continue;
            tasks = schedule->getTasks();
        }
        
        try 
        {
            char* task_name = 0; bool task_key = false; tasks.First();
            while (tasks.Next(task_name, task_key))
                if (task_name && task_name[0] != '\0') {
                    Task* task = processor->getTaskContainerAdapter().getTask(task_name); 
                    if (!task) { 
                        logger.error("Task '%s' not found.", task_name);
                        continue;
                    }
                    if (task->isEnabled() && !task->isInProcess())
                        processor->getTaskInvokeAdapter().invokeBeginProcess(task);
                }
        } 
        catch (std::exception& exc) 
        {
            awake.Wait(0);
            logger.error("Exception occurred during tasks scheduling : %s", exc.what());
        }
    }
    exited.Signal();
    return 0;
}

void TaskScheduler::init(TaskProcessorAdapter* processor, ConfigView* config)
{
    __require__(processor && config);
    
    this->processor = processor;

    // TODO: load up task scheduling plan from config
}

Schedule* TaskScheduler::getNextSchedule(time_t& scheduleTime)
{
    Schedule*   nextSchedule = 0;
    time_t      minimalTime = -1;

    char* key = 0; Schedule* schedule = 0; schedules.First();
    while (schedules.Next(key, schedule))
    {
        if (!schedule) continue;
        time_t time = schedule->calulateNextTime();
        if (time < 0) continue;
        if (minimalTime < 0) minimalTime = time;
        if (time < minimalTime) {
            minimalTime = time;
            nextSchedule = schedule;
        }
    }
    scheduleTime = minimalTime;
    return nextSchedule;
}

void TaskScheduler::addSchedule(Schedule* schedule)
{
    __require__(schedule);
    MutexGuard guard(schedulesLock);
    
    const char* scheduleId = schedule->id.c_str();
    if (scheduleId && scheduleId[0] != '\0') {
        if (schedules.Exists(scheduleId)) schedules.Delete(scheduleId);
        schedules.Insert(scheduleId, schedule);
    }
}
bool TaskScheduler::changeSchedule(std::string id, Schedule* schedule)
{                   
    __require__(schedule);
    MutexGuard guard(schedulesLock);
    
    const char* scheduleId = id.c_str();
    const char* newId = schedule->id.c_str();
    if (!scheduleId || scheduleId[0] == '\0' || !newId || newId[0] == '\0' || 
        !schedules.Exists(scheduleId)) return false;
    Schedule* old = schedules.Get(scheduleId);
    if (old) delete old;
    schedules.Delete(scheduleId);
    schedules.Insert(newId, schedule);
    return true;
}
bool TaskScheduler::removeSchedule(std::string id)
{
    MutexGuard guard(schedulesLock);

    const char* scheduleId = id.c_str();
    if (!scheduleId || scheduleId[0] == '\0' || 
        !schedules.Exists(scheduleId)) return false;
    Schedule* old = schedules.Get(scheduleId);
    if (old) delete old;
    schedules.Delete(scheduleId);
    return true;
}

}}

