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

using smsc::util::Exception;
using smsc::util::config::ConfigException;
using smsc::util::findConfigFile;
using smsc::util::config::ConfigView;
using smsc::core::network::Socket;

void atExit()
{
	exit(0);
};

void send(Socket &socket, uint16_t val);

int main(int argc, char **argv)
{   
    smsc::logger::Logger::Init();
    smsc::util::config::Manager * cfgman;
    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    cfgman= &cfgman->getInstance();
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("agent");

    int signo;
    sigset_t newmask;

    if( sigemptyset(&newmask) )
    {
        smsc_log_info(logger, "sigemptyset error: '%s'", strerror(errno) );
        return 0;
    }
        
    if( sigaddset(&newmask, SIGTERM) )
    {
        smsc_log_info(logger, "sigaddset error: '%s'", strerror(errno) );
        return 0;
    }

    if( sigprocmask(SIG_BLOCK, &newmask, NULL) )
    {
        smsc_log_info(logger, "sigprocmask error: '%s'", strerror(errno) );
        return 0;
    }

    std::auto_ptr<char> host( 0 );
    std::auto_ptr<char> mode( 0 );
    int port = -1;
    bool ishs = false;

  try{

      
      smsc_log_info(logger, "Agent is reading config..." );

      using smsc::util::config::ConfigView;
      std::auto_ptr<ConfigView> imConfig(new smsc::util::config::ConfigView(*cfgman, "cluster"));

      mode.reset( imConfig.get()->getString("mode", 0, 0) );

      if(!mode.get())
          throw Exception("Can't find parameter cluster.mode");

      if( strcmp(mode.get(), "hs") == 0)
          ishs = true;
      else if(strcmp(mode.get(), "single") != 0)
          throw Exception("Unexpected value of parameter cluster.mode: '%s'", mode.get());

      

      host.reset( imConfig.get()->getString("agentHost", 0, 0) );


      if(!host.get())
          throw Exception("Can't find parameter cluster.agentHost");

      port = imConfig.get()->getInt("agentPort");

      

       smsc_log_info(logger, "Agent has read conig" );
      

  }catch(ConfigException e)
  {
      smsc_log_info(logger, "Read config failed. %s", e.what() );
      return 0;
  }catch(Exception e)
  {
      smsc_log_info(logger, "Read config failed. %s", e.what() );
      return 0;
  }catch(...)
  {
      smsc_log_info(logger, "Read config failed. Unknown exception." );
      return 0;
  }

  try {
      smsc_log_info(logger, "Agent is connecting to smsc by host: '%s', port: %d", host.get(), port );

      Socket socket;
      socket.Init(host.get(), port, 10);

      if(!ishs){
          while(socket.Connect()){
              sleep(1);
          }

          ishs = true;
      }

      //if(ishs)
      //    if(socket.Connect())
      //        atExit();

      smsc_log_info(logger, "Agent has connected to smsc.");

      for( ;; ){
          smsc_log_info(logger, "Wait for a signal...");

          if( sigwait(&newmask, &signo) ){
              smsc_log_info(logger, "sigwait error: '%s'", strerror(errno) );
              continue;
          }

          if(signo == SIGTERM){

              smsc_log_info(logger, "Signal is got");

              smsc_log_info(logger, "Agent is connecting to smsc by host: '%s', port: %d", host.get(), port );
              if(socket.Connect()){
                  smsc_log_info(logger, "Connect failed");
                  continue;
              }

              try {
                  smsc_log_info(logger, "Agent is sending command to smsc");
                  uint16_t val = 1;
                  send(socket, val);
                  smsc_log_info(logger, "Command is sent");
              }catch(Exception e)
              {
                  smsc_log_info(logger, "Send failed. %s", e.what());
              }
              catch(...)
              {
                  smsc_log_info(logger, "Send failed. Unknown exception");
              }

          }
      }


  }catch(...)
  {
      smsc_log_info(logger, "Unknown exception." );
      return 0;
  }

	printf("Okkey\n");
	return 0;
}

void send(Socket &socket, uint16_t val)
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

    int toWrite = size; const char* writeBuffer = (const char *)buffer;
    while (toWrite > 0) {
        int write = socket.canWrite(10);
        if (write == 0) throw Exception("Command send failed. Timeout expired.");
        else if (write > 0) {
            write = socket.Write(writeBuffer, toWrite);
            if (write > 0) { writeBuffer+=write; toWrite-=write; continue; }
        }
        throw Exception("Command send failed. Socket closed. %s", strerror(errno));
    }
}
