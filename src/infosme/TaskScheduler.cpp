
#include "TaskScheduler.h"

namespace smsc { namespace infosme 
{

TaskScheduler::TaskScheduler()
    : Thread(), bStarted(false)
{
    // TODO: implement it
}
TaskScheduler::~TaskScheduler()
{
    MutexGuard guard(schedulesLock);
    
    int key = 0; Schedule* schedule = 0;
    IntHash<Schedule*>::Iterator it = schedules.First();
    while (it.Next(key, schedule))
        if (schedule) delete schedule;
    
    // TODO: implement it
}
void TaskScheduler::Start()
{
    // TODO: implement it
}
void TaskScheduler::Stop()
{
    // TODO: implement it
}
int TaskScheduler::Execute()
{
    // TODO: implement it
    return 0;
}

void TaskScheduler::init(TaskProcessor* processor, ConfigView* config)
{
    __require__(processor && config);
    this->processor = processor;
    
    // TODO: load up task scheduling plan from config
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

