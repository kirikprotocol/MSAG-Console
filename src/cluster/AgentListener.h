#ifndef AgentListener_dot_h
#define AgentListener_dot_h

#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include <string>
#include "logger/Logger.h"
#include <unistd.h>

namespace smsc {

namespace cluster {

using smsc::core::network::Socket;
using smsc::core::threads::Thread;
using smsc::logger::Logger;

class AgentListener : public Thread
{
public:
    AgentListener()
        : logger(Logger::getInstance("AgentLst")),clntSock(0), stop(true), stopSmsc(false),psmsc(0)
    {
  };
  virtual ~AgentListener()
  {
    Stop();
    if(clntSock)delete clntSock;
    clntSock=0;
  };

    void init(const std::string& host, int port, pid_t pid_);
    void assignSmsc(void* smscptr)
    {
      psmsc=smscptr;
    }
    using Thread::Start;
    void Start();
    void Stop();
    virtual int Execute();

protected:
    Logger *logger;
    Socket srvSock;
    Socket *clntSock;
    bool stop;
    bool stopSmsc;
    pid_t pid;
    void* psmsc;

    int readCommand(Socket * socket);
    void read(Socket * socket, void* buffer, int size);
    void shutdownSmsc();
};

}
}

#endif
