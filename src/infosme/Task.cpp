
#include "Task.h"

namespace smsc { namespace infosme 
{

bool Task::isInProcess()
{
    return false;
}
void Task::beginProcess()
{
}
void Task::endProcess()
{
}
void Task::doNotifyMessage(StateInfo& info)
{
}
void Task::dropAllMessages()
{
}
bool Task::getNextMessage(Connection* connection, Message& message)
{
    return false;
}

}}
