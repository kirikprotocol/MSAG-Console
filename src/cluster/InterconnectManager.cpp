
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

    sleep(2);

    {
        MutexGuard mg(dispatcherLock);
        MemAddMemberCommand *cmdAdd = new MemAddMemberCommand(std::string("name"), std::string("address"));
        MemDeleteMemberCommand *cmdDel = new MemDeleteMemberCommand(std::string("name"), std::string("address"));
        dispatcher->addCommand( (Command*)cmdAdd );
        dispatcher->addCommand( (Command*)cmdDel );

        /*MscBlockCommand *cmdBlock = new MscBlockCommand("mscNum");
        dispatcher->addCommand( (Command*)cmdBlock );

        MscClearCommand *cmdClear = new MscClearCommand("mscNum");
        dispatcher->addCommand( (Command*)cmdClear );

        MscRegistrateCommand *cmdReg = new MscRegistrateCommand("mscNum");
        dispatcher->addCommand( (Command*)cmdReg );

        MscUnregisterCommand *cmdUnreg = new MscUnregisterCommand("mscNum");
        dispatcher->addCommand( (Command*)cmdUnreg );

        PrcAddPrincipalCommand *prcAddCmd = new PrcAddPrincipalCommand(10, 5329, "address");
        dispatcher->addCommand( (Command*)prcAddCmd );

        PrcAlterPrincipalCommand *prcAlterCmd = new PrcAlterPrincipalCommand(1027, 5079, "addresses");
        dispatcher->addCommand( (Command*)prcAlterCmd );

        PrcDeletePrincipalCommand *prcDelCmd = new PrcDeletePrincipalCommand("address");
        dispatcher->addCommand( (Command*)prcDelCmd );

        ProfileDeleteCommand *proDelCmd = new ProfileDeleteCommand(1, 50, "address");
        dispatcher->addCommand( (Command*)proDelCmd );


        {
            uint8_t plan = 50;
            uint8_t type = 55;
            char address[] = "address";
            int codePage = smsc::smpp::DataCoding::SMSC7BIT;
            int reportOption = smsc::profiler::ProfileReportOptions::ReportNone;
            int hideOption = smsc::profiler::HideOption::hoDisabled;

            bool hideModifaible = true;
            bool divertModifaible = true;
            bool udhContact = false;
            bool translit = false;

            bool divertActive = true;
            bool divertActiveAbsent = true;
            bool divertActiveBlocked = false;
            bool divertActiveBarred = true;
            bool divertActiveCapacity = false;

            std::string local = "local";
            std::string divert = "divert";

        
            ProfileUpdateCommand *proUpdateCmd = new ProfileUpdateCommand(plan, type, address, codePage, reportOption, hideOption, hideModifaible,
                                                divertModifaible, udhContact, translit, divertActive, divertActiveAbsent,
                                                divertActiveBlocked, divertActiveBarred, divertActiveCapacity,
                                                local, divert);
            dispatcher->addCommand( (Command*)proUpdateCmd );
        }

        {

            smsc::smeman::SmeInfo si;
            si.typeOfNumber = 1;
            si.numberingPlan = 2;
            si.interfaceVersion = 0x34;
            si.rangeOfAddress = ".*";
            si.systemType = "regular";
            si.password = "password";
            si.hostname = "hostname";
            si.port = 1;
            si.systemId = "systemId";
            si.priority = 1;
            si.SME_N = 1;
            si.disabled = true;
            si.wantAlias = true;
            si.forceDC = true;
            si.internal = false;
            si.bindMode = smsc::smeman::smeRX;
            si.receiptSchemeName = "receiptShemeName";
            si.timeout = 1;
            si.proclimit = 2;
            si.schedlimit = 3;
            si.providerId = 4;

            SmeAddCommand *smeAddCmd = new SmeAddCommand(si);
            dispatcher->addCommand( (Command*)smeAddCmd );

        }

        SmeRemoveCommand *smeRemoveCmd = new SmeRemoveCommand("systemId");
        dispatcher->addCommand( (Command*)smeRemoveCmd );

        {
            smsc::smeman::SmeInfo si;
            si.typeOfNumber = 1;
            si.numberingPlan = 2;
            si.interfaceVersion = 0x34;
            si.rangeOfAddress = ".*";
            si.systemType = "regular";
            si.password = "password";
            si.hostname = "hostname";
            si.port = 1;
            si.systemId = "systemId";
            si.priority = 1;
            si.SME_N = 1;
            si.disabled = true;
            si.wantAlias = true;
            si.forceDC = true;
            si.internal = false;
            si.bindMode = smsc::smeman::smeRX;
            si.receiptSchemeName = "receiptShemeName";
            si.timeout = 1;
            si.proclimit = 2;
            si.schedlimit = 3;
            si.providerId = 4;

            SmeUpdateCommand *smeUpdateCmd = new SmeUpdateCommand(si);
            dispatcher->addCommand( (Command*)smeUpdateCmd );
        }*/

        SbmAddSubmiterCommand *sbmAddCmd = new SbmAddSubmiterCommand("name", "address");
        dispatcher->addCommand( (Command*)sbmAddCmd );

        SbmDeleteSubmiterCommand *sbmDelCmd = new SbmDeleteSubmiterCommand("name", "address");
        dispatcher->addCommand( (Command*)sbmDelCmd );
    
    }

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
