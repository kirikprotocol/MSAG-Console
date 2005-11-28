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

using smsc::util::Exception;
using smsc::util::config::ConfigException;
using smsc::util::findConfigFile;
using smsc::util::config::ConfigView;
using smsc::core::network::Socket;

void send(Socket &socket, uint16_t val);
void Sigemptyset(sigset_t * newmask, smsc::logger::Logger *logger);
void Sigaddset(sigset_t * newmask, int sig, smsc::logger::Logger *logger);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oset, smsc::logger::Logger *logger);
void Sigsuspend(const sigset_t *set, smsc::logger::Logger *logger);
void Sigwait(const sigset_t *set, int *sig, smsc::logger::Logger *logger);
bool checkConnect(Socket &socket);
extern "C" void Signal(int sig, void (*desp)(int));

class Sender : public smsc::core::threads::Thread
{
public:
    Sender(pid_t pid_, Socket& socket_):
        pid(pid_), isStopping(false),socket(socket_),
        logger(smsc::logger::Logger::getInstance("agent.sndr"))
        {}
    ~Sender(){}
    virtual int Execute();
    void Stop()
    {
      isStopping=true;
    }
protected:
    pid_t pid;
    bool isStopping;
    Socket& socket;
    smsc::logger::Logger * logger;
};

int Sender::Execute()
{
    while (!isStopping)
    {
        sleep(5);
        if( !checkConnect(socket) ){
            smsc_log_info(logger, "Connect failed");
            kill(pid, SIGTERM);
        }
    }
    return 0;
}

void atExit()
{
  exit(0);
};

int main(int argc, char **argv)
{
    smsc::logger::Logger::Init();
    smsc::util::config::Manager * cfgman;
    smsc::util::config::Manager::init(findConfigFile("config.xml"));
    cfgman= &cfgman->getInstance();
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("agent");

    int signo;
    sigset_t newmask, oldmask, zeromask;

    Sigemptyset(&newmask, logger);
    Sigemptyset(&oldmask, logger);
    Sigemptyset(&zeromask, logger);
    Sigaddset(&newmask, SIGTERM, logger);
    Sigaddset(&newmask, SIGUSR1, logger);

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


  }catch(ConfigException& e)
  {
      smsc_log_info(logger, "Read config failed. %s", e.what() );
      return 0;
  }catch(Exception& e)
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
      }else{
          if( socket.Connect() )
          {
              smsc_log_info(logger, "Can't connect to smsc");
              atExit();
          }
      }
      smsc_log_info(logger, "Agent has connected to smsc.");


      // Send command smsc to make it MASTER
      try {
          smsc_log_info(logger, "Agent is sending command to smsc");
          uint16_t val = 0;
          send(socket, val);
          smsc_log_info(logger, "Command is sent");
      }catch(Exception& e)
      {
          smsc_log_info(logger, "Send failed. %s", e.what());
      }
      catch(...)
      {
          smsc_log_info(logger, "Send failed. Unknown exception");
      }

      Sender sender(getpid(), socket);
      sender.Start();


      smsc_log_info(logger, "Wait for a signal...");

     for( ;; ){
          Sigwait(&newmask, &signo, logger);
          smsc_log_info(logger, "signo: %d, SIGTERM: %d, SIGUSR1: %d", signo, SIGTERM, SIGUSR1);
          if(signo == SIGTERM){
              if(!checkConnect(socket))
                  atExit();
              break;
          }
      }
      smsc_log_info(logger, "Signal is got");


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


      sender.Stop();
      sender.WaitFor();





  }catch(...)
  {
      smsc_log_info(logger, "Unknown exception." );
      return 0;
  }

  ////printf("Okkey\n");
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

bool checkConnect(Socket &socket)
{
    try {

    uint8_t buffer[6];
    int size = 6;

    // Signature
    buffer[0] = 17;
    buffer[1] = 32;
    buffer[2] = 7;
    buffer[3] = 152;

    // Command
    uint16_t val = 2; // connect test
    uint16_t value = htons(val);

    memcpy((void*)(buffer + 4), (const void*)&val, 4);

    int toWrite = size; const char* writeBuffer = (const char *)buffer;
    while (toWrite > 0) {
        int write = socket.canWrite(10);
        if (write == 0) return true;
        else if (write > 0) {
            return true;
            write = socket.Write(writeBuffer, toWrite);
            if (write > 0) { writeBuffer+=write; toWrite-=write; continue; }
        }
        return false;
    }

    }catch(...)
    {
        return false;
    }

    return true;
}

void Sigemptyset(sigset_t * newmask, smsc::logger::Logger *logger)
{
    if( sigemptyset(newmask) )
    {
        smsc_log_info(logger, "sigemptyset error: '%s'", strerror(errno) );
        exit(0);
    }
}

void Sigaddset(sigset_t * newmask, int sig, smsc::logger::Logger *logger)
{
    if( sigaddset(newmask, sig) )
    {
        smsc_log_info(logger, "sigaddset error: '%s'", strerror(errno) );
        exit(0);
    }
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oset, smsc::logger::Logger *logger){
    if(sigprocmask(how, set, oset) != 0)
    {
        smsc_log_info(logger, "sigaddset error: '%s'", strerror(errno) );
        exit(0);
    }
}

void Sigsuspend(const sigset_t *set, smsc::logger::Logger *logger)
{
    sigsuspend(set);
}

void Sigwait(const sigset_t *set, int *sig, smsc::logger::Logger *logger)
{
    if( sigwait(set, sig) ){
        smsc_log_info(logger, "sigwait error: '%s'", strerror(errno) );
        exit(0);
    }
}

extern "C" void Signal(int sig, void (*desp)(int)){
        signal(sig, desp );
}
