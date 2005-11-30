#include "stdio.h"

#include "util/config/Manager.h"
#include "logger/Logger.h"
#include "util/config/ConfigException.h"
#include "util/Exception.hpp"
#include "util/findConfigFile.h"
#include <memory>
#include "util/config/ConfigView.h"
#include <stdlib.h>
#include "core/network/Socket.hpp"
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

using smsc::util::Exception;
using smsc::util::config::ConfigException;
using smsc::util::findConfigFile;
using smsc::util::config::ConfigView;
using smsc::core::network::Socket;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;


static uint16_t COMMAND_MASTER=0;
static uint16_t COMMAND_SHUTDOWN=1;
static uint16_t COMMAND_PING=2;

void atExit()
{
  exit(0);
};


EventMonitor mon;
Socket clntSock;
bool signaled=false;

extern "C" void SigHandler(int sig)
{
  signaled=true;
  mon.Lock();
  mon.notify();
  mon.Unlock();
}

void SendCommand(Socket &socket, uint16_t val)
{
    uint8_t buffer[6];
    int size = 6;

    // Signature
    buffer[0] = 17;
    buffer[1] = 32;
    buffer[2] = 7;
    buffer[3] = 152;

    // Command
    uint16_t value = htons(val);

    memcpy((void*)(buffer + 4), (const void*)&val, 4);

    int toWrite = size;
    const char* writeBuffer = (const char *)buffer;
    while (toWrite > 0)
    {
      int write = socket.canWrite(10);
      if (write == 0)
      {
        throw Exception("Command send failed. Timeout expired.");
      }
      else
      if (write > 0)
      {
        write = socket.Write(writeBuffer, toWrite);
        if (write > 0)
        {
          writeBuffer+=write;
          toWrite-=write;
          continue;
        }
      }
      throw Exception("Command send failed. Socket error:'%s'", strerror(errno));
    }
}

int main(int argc, char **argv)
{
  smsc::logger::Logger::Init();
  smsc::util::config::Manager * cfgman;
  smsc::util::config::Manager::init(findConfigFile("config.xml"));
  cfgman= &cfgman->getInstance();
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("agent");


  sigset(SIGTERM,SigHandler);
  sigignore(SIGPIPE);

  std::auto_ptr<char> host( 0 );
  std::auto_ptr<char> mode( 0 );
  int port = -1;
  bool ishs = false;

  try
  {
    smsc_log_info(logger, "Agent is reading config..." );
    using smsc::util::config::ConfigView;
    std::auto_ptr<ConfigView> imConfig(new smsc::util::config::ConfigView(*cfgman, "cluster"));
    mode.reset( imConfig.get()->getString("mode", 0, 0) );

    if(!mode.get())
        throw Exception("Can't find parameter cluster.mode");

    if( strcmp(mode.get(), "hs") == 0 || strcmp(mode.get(), "ha") == 0)
    {
      ishs = true;
    }
    else
    {
      throw Exception("Agent cannot run in cluster.mode='%s'", mode.get());
    }

    host.reset( imConfig.get()->getString("agentHost", 0, 0) );
    if(!host.get())
        throw Exception("Can't find parameter cluster.agentHost");
    port = imConfig.get()->getInt("agentPort");
    smsc_log_info(logger, "Agent has read conig" );


  }catch(std::exception& e)
  {
    smsc_log_info(logger, "Read config failed. %s", e.what() );
    return 0;
  }catch(...)
  {
    smsc_log_info(logger, "Read config failed. Unknown exception." );
    return 0;
  }

  try
  {

    smsc_log_info(logger, "Agent is connecting to smsc by host: '%s', port: %d", host.get(), port );
    if(clntSock.Init(host.get(), port,0)!=0)
    {
      throw Exception("Failed to init socket at %s:%d",host.get(),port);
    }
    if( clntSock.Connect() !=0 )
    {
      smsc_log_info(logger, "Can't connect to smsc");
      atExit();
    }
    smsc_log_info(logger, "Agent has connected to smsc.");


    // Send command smsc to make it MASTER

    smsc_log_info(logger, "Agent is sending command to smsc");
    SendCommand(clntSock, COMMAND_MASTER);
    smsc_log_info(logger, "Command is sent");

    while(!signaled)
    {
      MutexGuard mg(mon);
      mon.wait(1000);
      SendCommand(clntSock,COMMAND_PING);
    }

    smsc_log_info(logger, "Agent is sending shutdown command to smsc");
    SendCommand(clntSock, COMMAND_SHUTDOWN);
    smsc_log_info(logger, "Command is sent");


  }catch(std::exception& e)
  {
    smsc_log_info(logger, "Exception:%s",e.what());
  }catch(...)
  {
    smsc_log_info(logger, "Unknown exception." );
  }
  return 0;
}
