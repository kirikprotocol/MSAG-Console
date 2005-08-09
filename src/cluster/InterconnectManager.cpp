
#include "InterconnectManager.h"
#include "util/Exception.hpp"
#include <string>
#include <unistd.h>

#include "smpp/smpp_structures.h"
#include "profiler/profiler-types.hpp"

namespace smsc { namespace cluster {

using smsc::util::Exception;

InterconnectManager* InterconnectManager::instance = 0;

InterconnectManager::InterconnectManager(Role _role, const std::string& m_ip, 
                                         const std::string& s_ip, int _port)
    : Interconnect(), Thread(), role(_role), master_ip(m_ip), slave_ip(s_ip), port(_port)
{
    isStopping = true;

    dispatcher = new CommandDispatcher();

    Start();
}

InterconnectManager::~InterconnectManager()
{
    Stop();
    
    pthread_join(thread, NULL);

    if(dispatcher){
        dispatcher->Stop();
        delete dispatcher;
    }
}

void InterconnectManager::init(Role _role, const std::string& m_ip, 
                               const std::string& s_ip, int _port)
{
    if (!InterconnectManager::instance) {
        InterconnectManager::instance = new InterconnectManager(_role, m_ip, s_ip, _port);
    }
}
void InterconnectManager::shutdown()
{
    
    if (InterconnectManager::instance) {
        delete InterconnectManager::instance;
    }
}
    
void InterconnectManager::sendCommand(Command* command)
{
    
    MutexGuard guard(commandsMonitor);
    // TODO: add command to queue
    
    commands.Push(command);
    if (!isStoped()){
        commandsMonitor.notify();
    }
   
}
void InterconnectManager::addListener(CommandType type, CommandListener* listener)
{
    MutexGuard mg(dispatcherLock);
    dispatcher->addListener(type, listener);
}
void InterconnectManager::activate()
{
    MutexGuard mg(dispatcherLock);
    if(dispatcher){
        dispatcher->Start();
    }
}

int InterconnectManager::Execute()
{

    // TODO: Send commands from commands queue (on commandsMonitor)
    while(!isStoped()){

        //printf("Waits for command...\n");
        if( !commandsMonitor.wait(30) )
        {

            printf("\nCommand\n");

            // Sends command to socket.
            Command *command;
            int count = commands.Count();
            for(int i=0; i<=count-1; i++){

                
                commands.Shift(command);

                // Sends command to socket
                send(command);
                printf("\n");

                if(command)
                    delete command;
            }

            
            commands.Clean();
            
        }
        //printf("Signal or timeout\n");


    }

    printf("IntercinnectManager: Execute out...\n");

    return 0;
}

void InterconnectManager::Start()
{
    MutexGuard mg(stopLock);
    if(!isStopping)
        return;
    isStopping = false;

    Thread::Start();
}

void InterconnectManager::Stop()
{
    MutexGuard mg(stopLock);
    MutexGuard guard(commandsMonitor);

    isStopping = true;
    commandsMonitor.notify();
}

bool InterconnectManager::isStoped()
{
    MutexGuard mg(stopLock);
    return isStopping;
}

void InterconnectManager::send(Command* command)
{
    uint32_t len = 0;
    std::auto_ptr<uint8_t> buff ( (uint8_t*)command->serialize(len) );
    int size = len + sizeof( int );
    std::auto_ptr<uint8_t> buffer( new uint8_t[size] );
    int type = command->getType();
    memcpy((void*)buffer.get(), (const void*)&type, 4);
    memcpy((void*)( buffer.get() + sizeof( int ) ), (const void*)buff.get(), len);

    /*int toWrite = size; const char* writeBuffer = (const char *)buff.get();
    while (toWrite > 0) {
        int write = socket.canWrite(10);
        if (write == 0) throw Exception("Command send failed. Timeout expired.");
        else if (write > 0) {
            write = socket.Write(writeBuffer, toWrite);
            if (write > 0) { writeBuffer+=write; toWrite-=write; continue; }
        }
        throw Exception("Command send failed. Socket closed. %s", strerror(errno));
    }*/
}


}}
