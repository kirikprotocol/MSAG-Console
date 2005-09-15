
#include "TaskScheduler.h"

namespace smsc { namespace infosme 
{

TaskScheduler::TaskScheduler()
    : Thread(), logger(Logger::getInstance("smsc.infosme.TaskScheduler")),
        bStarted(false), bNeedExit(false), bChanged(false)
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
        smsc_log_info(logger, "Starting ...");
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
        smsc_log_info(logger, "Started.");
    }
}
void TaskScheduler::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        smsc_log_info(logger, "Stopping ...");
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
        smsc_log_info(logger, "Stoped.");
    }
}

int TaskScheduler::Execute()
{
    const int SERVICE_SLEEP = 3600; // in seconds
    const int SERVICE_PAUSE = 100;  // in m-seconds

    Schedule*   schedule = 0;
    std::string scheduleId = "";

    while (!bNeedExit)
    {
        int toSleep = -1;
        {
            MutexGuard guard(schedulesLock);
            time_t scheduleTime = -1;
            schedule = getNextSchedule(scheduleTime);
            if (scheduleTime > 0 && schedule) {
                toSleep = scheduleTime-time(NULL);
                scheduleId = schedule->id;
            }
            bChanged = false;
        }
        
        if (toSleep < 0) {
            awake.Wait(SERVICE_PAUSE);
            continue;
        }
        else
        {
            //printf("Sleepping %ld seconds ...\n", toSleep);
            while (toSleep > 0 && !bNeedExit && !bChanged) 
            {
                if (toSleep > SERVICE_SLEEP) {
                    toSleep -= SERVICE_SLEEP;
                    awake.Wait(SERVICE_SLEEP*1000);
                    MutexGuard guard(schedulesLock);
                    if (bChanged) break;
                } 
                else {
                    awake.Wait(toSleep*1000);
                    break;
                }
            }
        }
        if (bNeedExit) break;
        
        {
            MutexGuard guard(schedulesLock);
            if (bChanged) continue;
        }
        
        Hash<bool> tasks;
        {
            MutexGuard guard(schedulesLock);
            if (!schedules.Exists(scheduleId.c_str())) continue;
            schedule = schedules.Get(scheduleId.c_str());
            if (!schedule) continue;
            tasks = schedule->getTasks();
        }
        
        try 
        {
            time_t currentTime = time(NULL);
            char* task_id = 0; bool task_key = false; tasks.First();
            while (tasks.Next(task_id, task_key))
                if (task_id && task_id[0] != '\0')
                {
                    TaskGuard taskGuard = processor->getTask(task_id);
                    Task* task = taskGuard.get();
                    if (!task) { 
                        smsc_log_warn(logger, "Task '%s' not found.", task_id);
                        continue;
                    }
                    if (!task->canGenerateMessages()) continue;
                    
                    if (task->isReady(currentTime, false) && !task->isInGeneration())
                        processor->invokeBeginGeneration(task);
                }
        } 
        catch (std::exception& exc) 
        {
            awake.Wait(0);
            smsc_log_error(logger, "Exception occurred during tasks scheduling : %s", exc.what());
        }
    }
    exited.Signal();
    return 0;
}

void TaskScheduler::init(TaskProcessorAdapter* _processor, ConfigView* config)
{
    __require__(_processor && config);
    
    this->processor = _processor;

    std::auto_ptr< std::set<std::string> > setGuard(config->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* scheduleId = (const char *)i->c_str();
            if (!scheduleId || scheduleId[0] == '\0')
                throw ConfigException("Schedule id empty or wasn't specified.");
            
            smsc_log_info(logger, "Loading schedule '%s' ...", scheduleId);
            
            std::auto_ptr<ConfigView> scheduleConfigGuard(config->getSubConfig(scheduleId));
            ConfigView* scheduleConfig = scheduleConfigGuard.get();
            Schedule* schedule = Schedule::create(scheduleConfig, scheduleId);
            
            if (schedule && !addSchedule(schedule)) {
                delete schedule;
                throw ConfigException("Schedule for id '%s' already defined.");
            }
        }
        catch (ConfigException& exc)
        {
            smsc_log_error(logger, "Load of schedules failed! Config exception: %s", exc.what());
            throw;
        }
    }
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
        
        /*printf("Schedule %s\t Next time: %s", 
               schedule ? schedule->id.c_str():"-", ctime(&time));*/
        
        if (minimalTime < 0 || time < minimalTime) {
            minimalTime = time;
            nextSchedule = schedule;
        }
    }
    scheduleTime = minimalTime;
    return nextSchedule;
}

bool TaskScheduler::addSchedule(Schedule* schedule)
{
    __require__(schedule);
    MutexGuard guard(schedulesLock);
    
    const char* scheduleId = schedule->id.c_str();
    if (!scheduleId || scheduleId[0] == '\0' || 
        schedules.Exists(scheduleId)) return false;
    schedules.Insert(scheduleId, schedule);
    bChanged = true;
    awake.Signal();
    return true;
}
bool TaskScheduler::changeSchedule(std::string id, Schedule* schedule)
{                   
    __require__(schedule);
    MutexGuard guard(schedulesLock);
    
    const char* scheduleId = id.c_str();
    const char* newId = schedule->id.c_str();
    if (!scheduleId || scheduleId[0] == '\0' || 
        !newId || newId[0] == '\0') return false;
    if (schedules.Exists(scheduleId)) {
        Schedule* old = schedules.Get(scheduleId);
        if (old) delete old;
        schedules.Delete(scheduleId);
    }
    schedules.Insert(newId, schedule);
    bChanged = true;
    awake.Signal();
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
    bChanged = true;
    awake.Signal();
    return true;
}

void TaskScheduler::removeTask(std::string taskId)
{
    MutexGuard guard(schedulesLock);

    char* key = 0; Schedule* schedule = 0; schedules.First();
    while (schedules.Next(key, schedule))
        if (schedule) schedule->removeTask(taskId);
}

}}

