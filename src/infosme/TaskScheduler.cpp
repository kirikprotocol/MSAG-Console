
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

void TaskScheduler::init(ConfigView* config)
{
    // TODO: load up task scheduling plan from config
}

void TaskScheduler::addSchedule(Schedule& schedule)
{
    // TODO: implement it
}
bool TaskScheduler::changeSchedule(int scheduleId, Schedule& schedule)
{
    // TODO: implement it
    return false;
}
bool TaskScheduler::removeSchedule(int scheduleId)
{
    // TODO: implement it
    return false;
}

}}

