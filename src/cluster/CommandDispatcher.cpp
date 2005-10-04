
#include "CommandDispatcher.h"

namespace smsc { namespace cluster 
{

CommandDispatcher::CommandDispatcher() : Thread(),
    logger(smsc::logger::Logger::getInstance("CmdDisp")),
    isStopping(true)
{

}
CommandDispatcher::~CommandDispatcher()
{
    pthread_join(thread, NULL);

    IntHash< Array<CommandListener*> >::Iterator it = listeners.First();
    int type;
    Array<CommandListener*> arr;
    while(it.Next(type, arr)){
        for(int i=0; i<= arr.Count() - 1; i++){
            CommandListener * listener = arr[i];
            if(listener)
                delete listener;
        }
    }
}

void CommandDispatcher::addCommand(Command* command)
{
    
    MutexGuard guard(commandsMonitor);

    // TODO: Put command into commands queue
    commands.Push(command);
    smsc_log_info(logger, "Command %02X added", command->getType());
    if (!isStopping) commandsMonitor.notify();
}
void CommandDispatcher::addListener(CommandType type, CommandListener* listener)
{
    MutexGuard guard(listenersLock);
    // TODO: Add listener to listeners hash

    // Types:
    // higest  16 bits - group of command
    // lowrest 16 bits - type command in group

    int key = type >> 16;

    //printf("Dispatcher::addListener, type: %d, key: %d\n", type, key);

    if( listeners.Exist( key ) ){
        Array<CommandListener*> *arr = listeners.GetPtr(key);

        if(arr){
            arr->Push(listener);
            smsc_log_info(logger, "Command listener with key %02X added, this is for commands %02X, ...", key, type);
        }

    }else{
        Array<CommandListener*> arr;
        arr.Push(listener);
        smsc_log_info(logger, "Command listener with key %02X added, this is for commands %02X, ...", key, type);
        listeners.Insert(key, arr);
    }

    //printf("Dispatcher::addListener, type: %d, key: %d ok\n", type, key);
        
}

void CommandDispatcher::Start()
{
    MutexGuard guard(commandsMonitor);
    if(!isStopping)
        return;
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
    // Flushs commands thats are put befor start
    //printf("CommandDispatcher, Execute 1\n");
    {
            MutexGuard listguard(listenersLock);

            Command *command;
            int count = commands.Count();
            smsc_log_info(logger, "Commands flushing, count: %d", count);
            for(int i=0; i<=count-1; i++){
                
                commands.Shift(command);

                int type = command->getType();
                smsc_log_info(logger, "Flush command %02X", type);
                type >>= 16;

                Array<CommandListener*> *arr = listeners.GetPtr(type);
                if(arr){
                    for(int i=0; i<= arr->Count() - 1; i++){
                        CommandListener *listener = (*arr)[i];
                        if(listener){
                            try {
                                listener->handle(*command);
                            }catch(Exception & e)
                            {
                                smsc_log_info(logger, "Handler exception, key: %02X, %s", type, e.what());
                            }catch(...)
                            {
                                smsc_log_info(logger, "Handler exception, key: %02X, Unexpected error", type);
                            }
                        }
                    }
                }

                if(command)
                    delete command;

            }
            commands.Clean();

            smsc_log_info(logger, "Commands flushed");
    }
    
    while (!isStopping)
    {
        

        ////printf("Dispatcher wait for command...\n");
        if( commandsMonitor.wait(30) ) // Wait for command passes to queue ...

        {
            MutexGuard listguard(listenersLock);

            Command *command;
            int count = commands.Count();
            smsc_log_info(logger, "Commands flushing, count: %d", count);
            for(int i=0; i<=count-1; i++){
                
                commands.Shift(command);

                int type = command->getType();
                smsc_log_info(logger, "Flush command %02X", type);
                type >>= 16;

                Array<CommandListener*> *arr = listeners.GetPtr(type);
                if(arr){
                    for(int i=0; i<= arr->Count() - 1; i++){
                        CommandListener *listener = (*arr)[i];
                        if(listener){
                            try {
                                listener->handle(*command);
                            }catch(Exception & e)
                            {
                                smsc_log_info(logger, "Handler exception, key: %02X, %s", type, e.what());
                            }catch(...)
                            {
                                smsc_log_info(logger, "Handler exception, key: %02X, Unexpected error", type);
                            }
                        }
                    }
                }

                if(command)
                    delete command;


            }
            commands.Clean();
            smsc_log_info(logger, "Commands flushed");
        }
        //printf("Dispatecher: command or timeout.\n");
    }

    //printf("Dispatcher: Execute out...\n");
    
    return 0;
}

bool CommandDispatcher::isStoped()
{
     MutexGuard guard(commandsMonitor);
     return isStopping;
}

}}
