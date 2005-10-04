
#include "CommandDispatcher.h"

namespace smsc { namespace cluster 
{

CommandDispatcher::CommandDispatcher() : Thread(),
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
        }

    }else{
        Array<CommandListener*> arr;
        arr.Push(listener);
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
            for(int i=0; i<=count-1; i++){
                
                commands.Shift(command);

                int type = command->getType();
                //printf("\nDispatcher, type: %02X\n", type);
                type >>= 16;
                //printf("Dispatcher, key: %d\n", type);

                Array<CommandListener*> *arr = listeners.GetPtr(type);
                if(arr){
                    for(int i=0; i<= arr->Count() - 1; i++){
                        CommandListener *listener = (*arr)[i];
                        if(listener){
                            listener->handle(*command);
                        }
                    }
                }

                if(command)
                    delete command;


            }
            commands.Clean();
    }
    
    while (!isStopping)
    {
        

        ////printf("Dispatcher wait for command...\n");
        if( commandsMonitor.wait(30) ) // Wait for command passes to queue ...

        {
            MutexGuard listguard(listenersLock);

            Command *command;
            int count = commands.Count();
            for(int i=0; i<=count-1; i++){
                
                commands.Shift(command);

                int type = command->getType();
                //printf("\nDispatcher, type: %d\n", type);
                type >>= 16;
                //printf("Dispatcher, key: %d\n", type);

                Array<CommandListener*> *arr = listeners.GetPtr(type);
                if(arr){
                    for(int i=0; i<= arr->Count() - 1; i++){
                        CommandListener *listener = (*arr)[i];
                        if(listener){
                            listener->handle(*command);
                        }
                    }
                }

                if(command)
                    delete command;


            }
            commands.Clean();
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
