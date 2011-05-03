
#include "InterconnectManager.h"
#include "util/Exception.hpp"
#include <string>
#include <unistd.h>

#include "smpp/smpp_structures.h"
#include "profiler/profiler-types.hpp"
#include "util/sleep.h"

namespace smsc { namespace cluster {

using smsc::util::Exception;

Interconnect* Interconnect::instance = 0;

InterconnectManager::InterconnectManager(const std::string& inAddr_,
                                         const std::string& attachedInAddr_, int _port, int _attachedPort)
    : Interconnect(), Thread(), logger(smsc::logger::Logger::getInstance("IM")), role(SLAVE), inAddr(inAddr_),
      attachedInAddr(attachedInAddr_), port(_port), attachedPort(_attachedPort)
{
    isStopping = true;

    dispatcher = new CommandDispatcher();

    //printf("inAddr: %s, attachedInAddr: %s, port: %d, attachedPort: %d\n", inAddr.c_str(), attachedInAddr.c_str(), port, attachedPort);

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

void InterconnectManager::internalInit()
{
  if( socket.InitServer(inAddr.c_str(), port, 0))
  {
    throw Exception("InterconnectManager: Can't init server socket at %s:%d", inAddr.c_str(), port);
  }

  if( socket.StartServer() )
  {
    throw Exception("InterconnectManager: Can't start socket server");
  }

  linger l;
  l.l_onoff=1;
  l.l_linger=0;
  setsockopt(socket.getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,(int)sizeof(l));


  if( attachedSocket.Init(attachedInAddr.c_str(), attachedPort, 0) )
  {
    throw Exception("InterconnectManager: Failed to init socket for %s:%d", attachedInAddr.c_str(), attachedPort);
  }

  if( attachedSocket.Connect() )
  {
    smsc_log_warn(logger, "InterconnectManager: Connect to %s:%d failed", attachedInAddr.c_str(), attachedPort);
  }

  reader.Init(&role, &socket, dispatcher);
  reader.Start();
}

void InterconnectManager::init(const std::string& inAddr, const std::string& attachedInAddr, int _port, int _attachedPort)
{
  if (!InterconnectManager::instance)
  {
    InterconnectManager::instance = new InterconnectManager(inAddr, attachedInAddr, _port, _attachedPort);
    ((InterconnectManager*)InterconnectManager::instance)->internalInit();
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
    {
      return;
    }

    commands.Push(command);
    smsc_log_info(logger, "Command %02X added", command->getType());
    if (!isStoped())
    {
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
    {
      return;
    }

    if(dispatcher)
    {
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
        //profile.locale = "locale";
        profile.locale = "";
        profile.hideModifiable = true;

        //profile.divert = "divert";
        profile.divert = "";
        profile.divertActive = true;
        profile.divertActiveAbsent = false;
        profile.divertActiveBlocked = false;
        profile.divertActiveBarred = true;
        profile.divertActiveCapacity = true;
        profile.divertModifiable = false;

        profile.udhconcat = true;
        profile.translit = true;

        profile.offset = 10;


        Command *cmd1 = new ProfileUpdateCommand(addr, profile);

        const char * address_ = "address1234567890123";
        int plan_ = 1;
        int type_ = 2;
        Command *cmd2 = new ProfileDeleteCommand(plan_, type_, address_);


        smsc::smeman::SmeInfo si;
        si.typeOfNumber = 1;
        si.numberingPlan = 2;
        si.interfaceVersion = 0x34;
        //si.rangeOfAddress = ".*";
        si.rangeOfAddress = "";
        //si.systemType = "regular";
        si.systemType = "";
        //si.password = "password";
        si.password = "";
        //si.hostname = "hostname";
        si.hostname = "";
        si.port = 1;
        //si.systemId = "systemId";
        si.systemId = "";
        si.priority = 1;
        si.SME_N = 1;
        si.disabled = true;
        si.wantAlias = true;
        si.forceDC = true;
        si.internal = false;
        si.bindMode = smsc::smeman::smeRX;
        //si.receiptSchemeName = "receiptShemeName";
        si.receiptSchemeName = "";
        si.timeout = 1;
        si.proclimit = 2;
        si.schedlimit = 3;
        si.providerId = 4;

        Command *cmd3 = new SmeAddCommand(si);
        Command *cmd4 = new SmeUpdateCommand(si);
        Command *cmd5 = new SmeRemoveCommand("systemId");

        const char *mscNum_ = "mscnum1";
        Command *cmd6 = new MscUnregisterCommand(mscNum_);
        Command *cmd7 = new MscBlockCommand(mscNum_);
        Command *cmd8 = new MscClearCommand(mscNum_);
        Command *cmd9 = new MscReportCommand("mscnum1", true, 5);
        Command *cmd10 = new MscRegistrateCommand("mscnum1", 1);

        Command *cmd11 = new PrcAddPrincipalCommand(1, 2, 3, "address");
        Command *cmd12 = new PrcDeletePrincipalCommand("address");
        Command *cmd13 = new PrcAlterPrincipalCommand(1, 2, "address");

        Command *cmd14 = new MemAddMemberCommand(50235, "dlname", "address");
        Command *cmd15 = new MemDeleteMemberCommand("dlname", "address");

        Command *cmd16 = new SbmAddSubmiterCommand(1, "dlname", "address");
        Command *cmd17 = new SbmDeleteSubmiterCommand("dlname", "address");

        Command *cmd18 = new DlAddCommand(1, "dlname", "owner", 1, 2);
        Command *cmd19 = new DlDeleteCommand("dlname");
        Command *cmd20 = new DlAlterCommand(1, "dlname");

        Command *cmd21 = new ApplyRoutesCommand();
        Command *cmd22 = new ApplyAliasesCommand();
        Command *cmd23 = new ApplyRescheduleCommand();
        Command *cmd24 = new ApplyLocaleResourceCommand();


        printf("======= Profile commands =========\n");
        printf("send command: %02X\n", cmd1->getType());
        sendCommand(cmd1);
        printf("send command: %02X\n", cmd2->getType());
        sendCommand(cmd2);

        printf("======= Sme commands     =========\n");
        printf("send command: %02X\n", cmd3->getType());
        sendCommand(cmd3);
        printf("send command: %02X\n", cmd4->getType());
        sendCommand(cmd4);
        printf("send command: %02X\n", cmd5->getType());
        sendCommand(cmd5);

        printf("======= Msc commands     =========\n");
        printf("send command: %02X\n", cmd6->getType());
        sendCommand(cmd6);
        printf("send command: %02X\n", cmd7->getType());
        sendCommand(cmd7);
        printf("send command: %02X\n", cmd8->getType());
        sendCommand(cmd8);
        printf("send command: %02X\n", cmd9->getType());
        sendCommand(cmd9);
        printf("send command: %02X\n", cmd10->getType());
        sendCommand(cmd10);

        printf("======= Prc commands     =========\n");
        printf("send command: %02X\n", cmd11->getType());
        sendCommand(cmd11);
        printf("send command: %02X\n", cmd12->getType());
        sendCommand(cmd12);
        printf("send command: %02X\n", cmd13->getType());
        sendCommand(cmd13);

        printf("======= Mem commands     =========\n");
        printf("send command: %02X\n", cmd14->getType());
        sendCommand(cmd14);
        printf("send command: %02X\n", cmd15->getType());
        sendCommand(cmd15);

        printf("======= Sbm commands     =========\n");
        printf("send command: %02X\n", cmd16->getType());
        sendCommand(cmd16);
        printf("send command: %02X\n", cmd17->getType());
        sendCommand(cmd17);

        printf("======= Dl commands      =========\n");
        printf("send command: %02X\n", cmd18->getType());
        sendCommand(cmd18);
        printf("send command: %02X\n", cmd19->getType());
        sendCommand(cmd19);
        printf("send command: %02X\n", cmd20->getType());
        sendCommand(cmd20);

        printf("======= Apply commands      =========\n");
        printf("send command: %02X\n", cmd21->getType());
        sendCommand(cmd21);
        printf("send command: %02X\n", cmd22->getType());
        sendCommand(cmd22);
        printf("send command: %02X\n", cmd23->getType());
        sendCommand(cmd23);
        printf("send command: %02X\n", cmd24->getType());
        sendCommand(cmd24);
        //sendCommand(cmd);
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
    smsc_log_info(logger,"ChangeRole: %d->%d",role,role_);
    if(role == role_)
    {
      return;
    }

    Role oldRole;

    switch(role_)
    {
      case MASTER:
      {
        oldRole = role;
        role = MASTER;
        if(oldRole == SLAVE)
        {
          if(dispatcher)
          {
            dispatcher->Stop();
            dispatcher->WaitFor();
          }
        }
      }break;
    case SLAVE:
      {
        reader.Stop();
        reader.WaitFor();

        if( (role == MASTER) || (role == SINGLE) )
        {
          flushCommands();
          role = SLAVE;
        }

        reader.Start();

        if(dispatcher)
        {
          dispatcher->Start();
        }

      }break;
    case SINGLE:
      {
        oldRole = role;
        role = SINGLE;

        if(role == SLAVE)
        {
          if(dispatcher)
          {
            dispatcher->Stop();
            dispatcher->WaitFor();
          }
        }
       }break;
    };

    {
      MutexGuard mg(handlersLock);
      for(int i=0; i<= handlers.Count() - 1; i++)
      {
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
  memcpy((void*)buffer.get(), (const void*)&type, 4);
  uint32_t val32 = htonl(len);
  memcpy((void*)( buffer.get() + 4), (const void*)&val32, 4);
  memcpy((void*)( buffer.get() + 8), (const void*)buff.get(), len);

  if(attachedSocket.WriteAll(buffer.get(),size)!=size)
  {
    throw Exception("IM:Failed to write cmd to socket:%s", strerror(errno));
  }
}

/*
uint32_t InterconnectManager::readRole()
{
    uint8_t buffer[12];
    int size = 12;

    int toRead = size; char* readBuffer = (char *)buffer;
    while (toRead > 0)
    {
      int read = attachedSocket.canRead(10);
      if (read == 0)
      {
        throw Exception("Role read failed. Timeout expired.");
      }
      else
      if (read > 0)
      {
        read = attachedSocket.Read(readBuffer, toRead);
        if (read > 0)
        {
          readBuffer+=read;
          toRead-=read;
          continue;
        }
      }
      throw Exception("Role read failed. Socket closed. %s", strerror(errno));
    }

    uint32_t type, val, role_, len;
    memcpy((void*)&val,     (const void*)buffer, 4);
    type = ntohl(val);
    if(type != GETROLE_CMD)
    {
      throw Exception("Role read failed. Unxpected command type.");
    }

    memcpy((void*)&val,   (const void*)(buffer + 4), 4);
    len = ntohl(val);
    if(len != 4)
    {
      throw Exception("Role read failed. GETROLE_CMD length isn't 4.");
    }

    memcpy((void*)&val,   (const void*)(buffer + 8), 4);
    role_ = ntohl(val);

    return role_;

}
*/

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
      smsc_log_debug(logger, "Command %02X is sending", command->getType());
      send(command.get());
      smsc_log_debug(logger, "Command %02X is sent", command->getType());
    }
    catch(std::exception & e)
    {
      {
        MutexGuard mg(commandsMonitor);
        commands.Unshift(command.release());
      }
      smsc_log_warn(logger, "Send command failed: '%s'", e.what());
      while( attachedSocket.Connect()!=0 && !isStopping)
      {
        smsc_log_warn(logger, "Connect to attahced smsc failed");
        smsc::util::millisleep(1000);
      }
    }
  }
}


}}
