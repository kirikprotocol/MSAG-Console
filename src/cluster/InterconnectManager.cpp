
#include "InterconnectManager.h"
#include "util/Exception.hpp"
#include <string>
#include <unistd.h>

#include "smpp/smpp_structures.h"
#include "profiler/profiler-types.hpp"

namespace smsc { namespace cluster {

using smsc::util::Exception;

Interconnect* Interconnect::instance = 0;

InterconnectManager::InterconnectManager(const std::string& inAddr_,
                                         const std::string& attachedInAddr_, int _port, int _attachedPort)
    : Interconnect(), Thread(), logger(smsc::logger::Logger::getInstance("IM")), role(MASTER), inAddr(inAddr_),
      attachedInAddr(attachedInAddr_), port(_port), attachedPort(_attachedPort)
{
    isStopping = true;

    dispatcher = new CommandDispatcher();

    //printf("inAddr: %s, attachedInAddr: %s, port: %d, attachedPort: %d\n", inAddr.c_str(), attachedInAddr.c_str(), port, attachedPort);

    if( socket.InitServer(inAddr.c_str(), port, 0)){
        throw Exception("InterconnectManager: Can't init socket server by host: %s, port: %d", inAddr.c_str(), port);
    }

    if( socket.StartServer() ){
        throw Exception("InterconnectManager: Can't start socket server");
    }

    if( attachedSocket.Init(attachedInAddr.c_str(), attachedPort, 0) ){
        throw Exception("InterconnectManager: Can't init socket by host: %s, port: %d", attachedInAddr.c_str(), attachedPort);
    }

    if( attachedSocket.Connect() )
        smsc_log_info(logger, "InterconnectManager: Can't connect to smsc by host: %s, port: %d", attachedInAddr.c_str(), attachedPort);

    // At a start role is SLAVE allways
    role = SLAVE;

    //printf("role: %d\n", role);

    reader.Init(&role, &socket, dispatcher);
    //printf("reader starting...\n");
    reader.Start();
    //printf("readers started\n");

    /*{
        Command *cmd = new MscReportCommand("mscnum1", true, 5);
        sendCommand(cmd);
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

        Command *cmd1 = new SmeAddCommand(si);
        Command *cmd2 = new SmeUpdateCommand(si);
        Command *cmd3 = new SmeRemoveCommand("systemId");
        sendCommand(cmd1);
        sendCommand(cmd2);
        sendCommand(cmd3);

    }

    {
        const smsc::sms::Address addr(".1.1.address1234567890123");
        smsc::profiler::Profile profile;

        profile.codepage = smsc::smpp::DataCoding::SMSC7BIT;
        profile.reportoptions = 0;
        profile.hide = 0;
        profile.locale = "locale";
        profile.hideModifiable = true;

        profile.divert = "divert";
        profile.divertActive = true;
        profile.divertActiveAbsent = false;
        profile.divertActiveBlocked = false;
        profile.divertActiveBarred = true;
        profile.divertActiveCapacity = true;
        profile.divertModifiable = false;

        profile.udhconcat = true;
        profile.translit = true;

        profile.offset = 10;


        Command *cmd = new ProfileUpdateCommand(addr, profile);
        printf("send command\n");
        sendCommand(cmd);
        printf("command sent\n");
    }

    {
        const char * address_ = "address1234567890123";
        int plan_ = 1;
        int type_ = 2;
        Command *cmd = new ProfileDeleteCommand(plan_, type_, address_);
        sendCommand(cmd);
    }

    {
        const char *mscNum_ = "mscnum1";
        Command *cmd1 = new MscUnregisterCommand(mscNum_);
        Command *cmd2 = new MscBlockCommand(mscNum_);
        Command *cmd3 = new MscClearCommand(mscNum_);
        sendCommand(cmd1);
        sendCommand(cmd2);
        sendCommand(cmd3);
    }

    {
        Command *cmd = new MscRegistrateCommand("mscnum1", 1);
        sendCommand(cmd);
    }


    {
        std::vector<std::string> phones;

        phones.push_back("phone1");
        phones.push_back("phone2");

        Command *cmd1 = new AclRemoveCommand(1);
        Command *cmd2 = new AclCreateCommand("name", "desc", "type", 10, phones);
        Command *cmd3 = new AclUpdateInfoCommand(1, "name", "desc", "type");
        Command *cmd4 = new AclRemoveAddressesCommand(1, phones);
        Command *cmd5 = new AclAddAddressesCommand(1, phones);
        sendCommand(cmd1);
        sendCommand(cmd2);
        sendCommand(cmd3);
        sendCommand(cmd4);
        sendCommand(cmd5);
    }

    {

        Command *cmd1 = new PrcAddPrincipalCommand(1, 2, 3, "address");
        Command *cmd2 = new PrcDeletePrincipalCommand("address");
        Command *cmd3 = new PrcAlterPrincipalCommand(1, 2, "address");

        sendCommand(cmd1);
        sendCommand(cmd2);
        sendCommand(cmd3);
    }

    {
        Command *cmd1 = new MemAddMemberCommand(50235, "dlname", "address");
        Command *cmd2 = new MemDeleteMemberCommand("dlname", "address");

        sendCommand(cmd1);
        sendCommand(cmd2);

    }

    {
        Command *cmd1 = new SbmAddSubmiterCommand(1, "dlname", "address");
        Command *cmd2 = new SbmDeleteSubmiterCommand("dlname", "address");

        sendCommand(cmd1);
        sendCommand(cmd2);
    }

    {
        Command *cmd1 = new DlAddCommand(1, "dlname", "owner", 1, 2);
        Command *cmd2 = new DlDeleteCommand("dlname");
        Command *cmd3 = new DlAlterCommand(1, "dlname");

        sendCommand(cmd1);
        sendCommand(cmd2);
        sendCommand(cmd3);
    }*/




}

InterconnectManager::~InterconnectManager()
{
  socket.Abort();
  attachedSocket.Abort();
  reader.Stop();
  Stop();

  WaitFor();

  if(dispatcher)
  {
    dispatcher->Stop();
    delete dispatcher;
  }
}

void InterconnectManager::init(const std::string& inAddr, const std::string& attachedInAddr, int _port, int _attachedPort)
{
  if (!InterconnectManager::instance)
  {
    InterconnectManager::instance = new InterconnectManager(inAddr, attachedInAddr, _port, _attachedPort);
    ((InterconnectManager*)InterconnectManager::instance)->Start();

  }
}
void InterconnectManager::shutdown()
{

    if (InterconnectManager::instance)
    {
      __trace__("Terminating Interconnect manager");
      delete InterconnectManager::instance;
      InterconnectManager::instance=0;
      __trace__("Interconnect manager terminated");
    }
}

void InterconnectManager::sendCommand(Command* command)
{
    MutexGuard guard(commandsMonitor);
    // TODO: add command to queue

    if(!isMaster())
        return;

    commands.Push(command);
    smsc_log_info(logger, "Command %02X added", command->getType());
    if (!isStoped()){
        commandsMonitor.notify();
    }

}
void InterconnectManager::addListener(CommandType type, CommandListener* listener)
{
    dispatcher->addListener(type, listener);
}
void InterconnectManager::activate()
{
    if(!isSlave())
        return;

    if(dispatcher){
        dispatcher->Start();
    }
}

int InterconnectManager::Execute()
{

    if(isMaster())
    {
        // Flushs commands that was added befor start
        flushCommands();
    }

    /*sleep(30);
    if(isMaster())
    {
        const smsc::sms::Address addr(".1.1.address1234567890123");
        smsc::profiler::Profile profile;

        profile.codepage = smsc::smpp::DataCoding::SMSC7BIT;
        profile.reportoptions = 0;
        profile.hide = 0;
        profile.locale = "locale";
        profile.hideModifiable = true;

        profile.divert = "divert";
        profile.divertActive = true;
        profile.divertActiveAbsent = false;
        profile.divertActiveBlocked = false;
        profile.divertActiveBarred = true;
        profile.divertActiveCapacity = true;
        profile.divertModifiable = false;

        profile.udhconcat = true;
        profile.translit = true;

        profile.offset = 10;


        Command *cmd = new ProfileUpdateCommand(addr, profile);
        printf("send command\n");
        sendCommand(cmd);
        printf("command sent\n");
        flushCommands();
    }*/

    // TODO: Send commands from commands queue (on commandsMonitor)
    while(!isStoped())
    {
      {
        MutexGuard mg(commandsMonitor);
        commandsMonitor.wait(1000);
        if(!isMaster() || commands.Count()==0)
        {
          continue;
        }
      }
      if(isMaster())
      {
        flushCommands();
      }
    }

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
    isStopping = true;
    MutexGuard mg(stopLock);
    MutexGuard guard(commandsMonitor);
    commandsMonitor.notify();
}

bool InterconnectManager::isStoped()
{
    MutexGuard mg(stopLock);
    return isStopping;
}

bool InterconnectManager::isMaster()
{
    bool ismaster = ( role == MASTER );
    return ismaster;
}

bool InterconnectManager::isSlave()
{
    bool ismaster = ( role == SLAVE );
    return ismaster;
}

bool InterconnectManager::isSingle()
{
    bool ismaster = ( role == SINGLE );
    return ismaster;
}

void InterconnectManager::changeRole(Role role_)
{
    if(role == role_)
        return;

    Role oldRole;

    switch(role_){
    case MASTER:
        oldRole = role;
        role = MASTER;
        if(oldRole == SLAVE){
            if(dispatcher){
                dispatcher->Stop();
                dispatcher->WaitFor();
            }
        }

        break;
    case SLAVE:

        reader.Stop();
        reader.WaitFor();

        if( (role == MASTER) || (role == SINGLE) )
        {
            flushCommands();
            role = SLAVE;
        }

        reader.Start();

        if(dispatcher)
            dispatcher->Start();

        break;
    case SINGLE:
        oldRole = role;
        role = SINGLE;

        if(role == SLAVE){
            if(dispatcher){
                dispatcher->Stop();
                dispatcher->WaitFor();
            }
        }

        break;
    };

    {
        MutexGuard mg(handlersLock);
        for(int i=0; i<= handlers.Count() - 1; i++){
            handlers[i].run(role);
        }
    }
}

void InterconnectManager::addChangeRoleHandler(ChangeRoleHandler * fun, void *arg)
{
    MutexGuard mg(handlersLock);
    handlers.Push(ArgHandler(fun, arg));
}

void InterconnectManager::send(Command* command)
{
    uint32_t len = 0;
    std::auto_ptr<uint8_t> buff ( (uint8_t*)command->serialize(len) );
    int size = len + 8;
    std::auto_ptr<uint8_t> buffer( new uint8_t[size] );
    uint32_t type = htonl( command->getType() );
    //printf("send, type: %02X, len: %d\n", command->getType(), len);
    memcpy((void*)buffer.get(), (const void*)&type, 4);
    uint32_t val32 = htonl(len);
    memcpy((void*)( buffer.get() + 4), (const void*)&val32, 4);
    memcpy((void*)( buffer.get() + 8), (const void*)buff.get(), len);

    int toWrite = size; const char* writeBuffer = (const char *)buffer.get();
    int written;
    while (toWrite > 0)
    {
      written = attachedSocket.Write(writeBuffer, toWrite);
      if (written > 0 )
      {
        writeBuffer+=written;
        toWrite-=written;
      }else
      {
        throw Exception("Command send failed. Socket closed. %s", strerror(errno));
      }
    }

}

uint32_t InterconnectManager::readRole()
{
    uint8_t buffer[12];
    int size = 12;

    try {
        int toRead = size; char* readBuffer = (char *)buffer;
        while (toRead > 0) {
            int read = attachedSocket.canRead(10);
            if (read == 0) throw Exception("Role read failed. Timeout expired.");
                else if (read > 0) {
                    read = attachedSocket.Read(readBuffer, toRead);
                    if (read > 0) { readBuffer+=read; toRead-=read; continue; }
                }
                throw Exception("Role read failed. Socket closed. %s", strerror(errno));
        }

        uint32_t type, val, role_, len;
        memcpy((void*)&val,     (const void*)buffer, 4);
        type = ntohl(val);
        if(type != GETROLE_CMD)
            throw Exception("Role read failed. Unxpected command type.");

        memcpy((void*)&val,   (const void*)(buffer + 4), 4);
        len = ntohl(val);
        if(len != 4)
            throw Exception("Role read failed. GETROLE_CMD length isn't 4.");

        memcpy((void*)&val,   (const void*)(buffer + 8), 4);
        role_ = ntohl(val);

        return role_;

    }catch(const Exception& e){
        throw Exception("%s", e.what());
    }catch(...){
        throw Exception("Command read failed. Unexpected error.");
    }
}

void InterconnectManager::flushCommands()
{
  Command* cmd;
  while(!isStopping)
  {
    std::auto_ptr<Command> command;
    {
      MutexGuard mg(commandsMonitor);
      if(commands.Count()>0)
      {
        commands.Shift(cmd);
        command=std::auto_ptr<Command>(cmd);
      }else
      {
        break;
      }
    }
    try
    {
      smsc_log_info(logger, "Command %02X is sending", command->getType());
      send(command.get());
      smsc_log_info(logger, "Command %02X is sent", command->getType());
    }
    catch(std::exception & e)
    {
      {
        MutexGuard mg(commandsMonitor);
        commands.Unshift(command.release());
      }
      smsc_log_info(logger, "Send command failed: '%s'", e.what());
      while( attachedSocket.Connect()!=0 && !isStopping)
      {
        smsc_log_info(logger, "Connect to attahced smsc failed");
      }
    }
  }
}


}}
