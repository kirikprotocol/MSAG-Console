
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

bool TaskScheduler::registerTask(Task* task)
{
    // TODO: implement it
    return false;
}
bool TaskScheduler::unregisterTask(const char* name)
{
    // TODO: implement it
    return false;
}

}}

