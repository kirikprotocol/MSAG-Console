
#include "TaskScheduler.h"

namespace smsc { namespace infosme 
{

TaskScheduler::TaskScheduler()
    : Thread(), logger(Logger::getCategory("smsc.infosme.TaskScheduler")),
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
                if (toSleep > SERVICE_SLEEP*1000) {
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
            char* task_id = 0; bool task_key = false; tasks.First();
            while (tasks.Next(task_id, task_key))
                if (task_id && task_id[0] != '\0')
                {
                    TaskGuard taskGuard = processor->getTask(task_id);
                    Task* task = taskGuard.get();
                    if (!task) { 
                        logger.warn("Task '%s' not found.", task_id);
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

    std::auto_ptr< std::set<std::string> > setGuard(config->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* scheduleId = (const char *)i->c_str();
            if (!scheduleId || scheduleId[0] == '\0')
                throw ConfigException("Schedule id empty or wasn't specified.");
            
            logger.info("Loading schedule '%s' ...", scheduleId);
            
            std::auto_ptr<ConfigView> scheduleConfigGuard(config->getSubConfig(scheduleId));
            ConfigView* scheduleConfig = scheduleConfigGuard.get();
            
            Schedule* schedule = Schedule::create(scheduleConfig, scheduleId);

            const char* tasksStr = scheduleConfig->getString("tasks");
            if (!tasksStr)
                throw ConfigException("Schedule tasks set empty or wasn't specified.");
            
            const char* tasksCur = tasksStr;
            std::string taskId = "";
            
            if (*tasksCur != '\0') do
            {
                if (*tasksCur == ',' || *tasksCur == '\0') {
                    const char* task_id = taskId.c_str();
                    if (!task_id || task_id[0] == '\0') {
                        delete schedule;
                        throw ConfigException("Task id is invalid.");
                    }
                    if (!processor->hasTask(taskId)) {
                        delete schedule;
                        throw ConfigException("Task '%s' wasn't defined.", task_id);
                    }
                    if (!schedule->addTask(taskId)) {
                        delete schedule;
                        throw ConfigException("Task '%s' was already assigned to schedule.",task_id);
                    }
                    taskId = "";
                } 
                else if (!isspace(*tasksCur)) taskId += *tasksCur;
            } 
            while (*tasksCur++);
            
            if (!addSchedule(schedule)) {
                delete schedule;
                throw ConfigException("Schedule for id '%s' already defined.");
            }
                
        }
        catch (ConfigException& exc)
        {
            logger.error("Load of schedules failed! Config exception: %s", exc.what());
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
    if (!scheduleId || scheduleId[0] == '\0' || !newId || newId[0] == '\0' || 
        !schedules.Exists(scheduleId)) return false;
    Schedule* old = schedules.Get(scheduleId);
    if (old) delete old;
    schedules.Delete(scheduleId);
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

}}

