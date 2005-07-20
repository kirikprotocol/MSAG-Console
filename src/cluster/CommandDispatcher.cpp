
#include "CommandDispatcher.h"

namespace smsc { namespace cluster 
{

CommandDispatcher::CommandDispatcher() : Thread()
{

}
CommandDispatcher::~CommandDispatcher()
{

}

void CommandDispatcher::addCommand(Command* command)
{
    MutexGuard guard(commandsMonitor);
    // TODO: Put command into commands queue
    //if (isStarted()) commandsMonitor.notify();
}
void CommandDispatcher::addListener(CommandType type, CommandListener* listener)
{
    MutexGuard guard(listenersLock);
    // TODO: Add listener to listeners hash
}

void CommandDispatcher::Start()
{

}
void CommandDispatcher::Stop()
{

}
int CommandDispatcher::Execute()
{
    // TODO: Implement commands dispatching from queue (on monitor) to listeners
}

}}
