#include "cluster/AgentListener.h"
#include "cluster/Interconnect.h"
#include <signal.h>
#include "system/smsc.hpp"

namespace smsc{

namespace system{
  extern "C" void sigShutdownHandler(int signo);
}

namespace cluster {

using smsc::core::network::Socket;
using namespace smsc::util;

void AgentListener::Start()
{
    stop = false;
    Thread::Start();
}

int AgentListener::Execute()
{
    smsc_log_info(logger, "Start server..." );
    if( srvSock.StartServer() )
    {
      smsc_log_info(logger, "Can't start socket server" );
      return 0;
    }
    smsc_log_info(logger, "Server started" );

    while(!stop)
    {

      smsc_log_info(logger, "Wait for a connection...." );
      if(clntSock)delete clntSock;
      clntSock=srvSock.Accept();
      if (clntSock)
      {
        smsc_log_info(logger, "Connection accepted" );
        for( ;; )
        {
          try
          {
            smsc_log_debug(logger, "Read a command..." );
            int res = -1;
            res = readCommand(clntSock);
            smsc_log_debug(logger, "Command is read, res: %d", res );

            if(res == 0)
            {
               Interconnect * icon = Interconnect::getInstance();
               if(icon)
               {
                 smsc_log_info(logger,"Change role to MASTER");
                 icon->changeRole(MASTER);
               }
            }else if(res == 1)
            {
              stopSmsc = true;
              stop = true;
              srvSock.Close();
              break;
            }
          }catch(...)
          {
            smsc_log_info(logger, "Exception during read command." );
            smsc_log_info(logger, "Restarts socket server ..." );
            if( srvSock.StartServer() )
                smsc_log_info(logger, "Can't start socket server" );
            else
                smsc_log_info(logger, "Socket server started" );

            break;
          }
        }
      }
    }

    if(stopSmsc)
    {
      shutdownSmsc();
    }
    return 0;
}

void AgentListener::init(const std::string& host, int port, pid_t pid_)
{
    pid = pid_;
    if(srvSock.InitServer(host.c_str(), port, 0)!=0)
    {
      smsc_log_warn(logger,"Failed to init AgentListener srv socket at %s:%d",host.c_str(),port);
    }
    linger l;
    l.l_onoff=1;
    l.l_linger=0;
    setsockopt(srvSock.getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,(int)sizeof(l));
};

void AgentListener::Stop()
{
    smsc_log_info(logger, "agent listener stopping" );
    stop = true;
    if(clntSock)clntSock->Abort();
    srvSock.Abort();
    WaitFor();
    smsc_log_info(logger, "agent listener stopped" );
}

int AgentListener::readCommand(Socket * socket)
{
  uint8_t buffer[6];
  smsc_log_debug(logger, "readCommand, read..." );
  read(socket, (void*)&buffer, 6);
  smsc_log_debug(logger, "readCommand, command is read" );
  smsc_log_debug(logger, "b[0]: %d, b[1]: %d, b[2]: %d, b[3]: %d", buffer[0], buffer[1], buffer[2], buffer[3] );
  // Checks signature
  if(buffer[0] != 17 || buffer[1] != 32 || buffer[2] != 7 || buffer[3] != 152)
  {
    smsc_log_warn(logger,"invalid signature in command from agent");
    return -1;
  }
  uint16_t val;
  memcpy((void*)&val, (const void*)(buffer + 4), 2);
  val = ntohs(val);
  smsc_log_debug(logger, "val: %d", val );
  return val;
}

void AgentListener::read(Socket * socket, void* buffer, int size)
{
  if(!socket)
  {
    throw Exception("Command read failed. Socket pointer is NULL.");
  }
  int toRead = size;
  char* readBuffer = (char *)buffer;
  while (toRead > 0)
  {
    int read = socket->canRead(60);
    if (read == 0) continue;
    else if (read > 0)
    {
      read = socket->Read(readBuffer, toRead);
      if (read > 0)
      {
        readBuffer+=read;
        toRead-=read;
        continue;
      }
    }
    throw Exception("Command read failed. Socket closed. %s", strerror(errno));
  }
}

void AgentListener::shutdownSmsc()
{
  //if(psmsc)((smsc::system::Smsc*)psmsc)->stop();
  smsc::system::sigShutdownHandler(15);
}

}
}
