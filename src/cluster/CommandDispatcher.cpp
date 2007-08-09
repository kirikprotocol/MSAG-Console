
#include "CommandDispatcher.h"

namespace smsc { namespace cluster
{

CommandDispatcher::CommandDispatcher() : Thread(),
    logger(smsc::logger::Logger::getInstance("IMCmdDisp")),
    isStopping(true)
{
}

CommandDispatcher::~CommandDispatcher()
{
  Stop();
  WaitFor();

  IntHash< Array<CommandListener*> >::Iterator it = listeners.First();
  int type;
  Array<CommandListener*> arr;
  while(it.Next(type, arr))
  {
    for(int i=0; i<arr.Count(); i++)
    {
      CommandListener * listener = arr[i];
      if(listener)
      {
        delete listener;
      }
    }
  }
}

void CommandDispatcher::addCommand(Command* command)
{
  MutexGuard guard(commandsMonitor);
  commands.Push(command);
  smsc_log_debug(logger, "Command %02X added", command->getType());
  commandsMonitor.notify();
}
void CommandDispatcher::addListener(CommandType type, CommandListener* listener)
{
  MutexGuard guard(listenersLock);

  // Types:
  // higest  16 bits - group of command
  // lowrest 16 bits - type command in group

  int key = type >> 16;

  if( listeners.Exist( key ) )
  {
    Array<CommandListener*> *arr = listeners.GetPtr(key);
    if(arr)
    {
      arr->Push(listener);
      smsc_log_debug(logger, "Command listener with key %02X added, this is for commands %02X, ...", key, type);
    }
  }else
  {
    Array<CommandListener*> arr;
    arr.Push(listener);
    smsc_log_debug(logger, "Command listener with key %02X added, this is for commands %02X, ...", key, type);
    listeners.Insert(key, arr);
  }
}

void CommandDispatcher::Start()
{
  MutexGuard guard(commandsMonitor);
  if(!isStopping)
  {
    return;
  }
  isStopping = false;
  Thread::Start();
}

void CommandDispatcher::Stop()
{
  MutexGuard guard(commandsMonitor);
  isStopping = true;
  commandsMonitor.notify();
}

int CommandDispatcher::Execute()
{
  Command *command;
  while (!isStopping)
  {
    std::auto_ptr<Command> cmd;
    {
      MutexGuard mg(commandsMonitor);
      if(commands.Count()>0)
      {
        commands.Shift(command);
        cmd=std::auto_ptr<Command>(command);
      }else
      {
        commandsMonitor.wait();
        continue;
      }
    }
    {
      MutexGuard listguard(listenersLock);
      int type = command->getType();
      smsc_log_info(logger, "Processing command %X", type);
      int key  = type >> 16;
      Array<CommandListener*> *arr = listeners.GetPtr(key);
      if(arr)
      {
        for(int i=0; i<arr->Count(); i++)
        {
          CommandListener *listener = (*arr)[i];
          if(listener)
          {
            try
            {
              listener->handle(*command);
            }catch(std::exception & e)
            {
              smsc_log_warn(logger, "Handler exception, key: %X, %s", key, e.what());
            }catch(...)
            {
              smsc_log_warn(logger, "Handler exception, key: %X, Unexpected error", key);
            }
          }
        }
      } else {
        smsc_log_warn( logger, "Processor not found for command type %X", type );
      }
    }
  }
  return 0;
}

bool CommandDispatcher::isStoped()
{
  return isStopping;
}

}}
