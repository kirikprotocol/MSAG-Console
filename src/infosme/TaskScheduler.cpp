
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

    int key = 0; Schedule* schedule = 0;
    IntHash<Schedule*>::Iterator it = schedules.First();
    while (it.Next(key, schedule))
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
        
        Task* task = 0;
        {
            MutexGuard guard(schedulesLock);
            time_t scheduleTime = -1;
            Schedule* schedule = getNextSchedule(scheduleTime);
            if (!schedule || scheduleTime > time(NULL)) continue;
            task = schedule->task; // TODO: get task name only
        }
        if (!task) { 
            logger.error("Task not found.");
            continue;
        }
        
        try 
        {
            //invoker->invokeBeginProcess(task, ???);
            // TODO locate task & call beginProcess via invoker
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

void TaskScheduler::init(TaskInvokeAdapter* invoker, ConfigView* config)
{
    __require__(invoker && config);
    
    this->invoker = invoker;
    // TODO: load up task scheduling plan from config
}

Schedule* TaskScheduler::getNextSchedule(time_t& scheduleTime)
{
    Schedule*   nextSchedule = 0;
    time_t      minimalTime = -1;

    int key = 0; Schedule* schedule = 0;
    IntHash<Schedule*>::Iterator it = schedules.First();
    while (it.Next(key, schedule)) 
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
    
    if (schedules.Exist(schedule->id)) schedules.Delete(schedule->id);
    schedules.Insert(schedule->id, schedule);
}
bool TaskScheduler::changeSchedule(int scheduleId, Schedule* schedule)
{
    __require__(schedule);
    MutexGuard guard(schedulesLock);
    
    if (!schedules.Exist(scheduleId)) return false;
    Schedule* old = schedules.Get(scheduleId);
    if (old) delete old;
    schedules.Delete(scheduleId);
    schedules.Insert(schedule->id, schedule);
    return true;
}
bool TaskScheduler::removeSchedule(int scheduleId)
{
    MutexGuard guard(schedulesLock);

    if (!schedules.Exist(scheduleId)) return false;
    Schedule* old = schedules.Get(scheduleId);
    if (old) delete old;
    schedules.Delete(scheduleId);
    return true;
}

}}

