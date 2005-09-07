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
        : logger(Logger::getInstance("AgentListner")), stop(true), stopSmsc(false)
    {
	};
	virtual ~AgentListener()
	{
		sock.Close();
        Stop();
	};

    void init(std::string host, int port, pid_t pid_);
	void Start();
    virtual int Execute();
    
protected:
    Logger *logger;
	Socket sock;
    bool stop;
    bool stopSmsc;
    void Stop();
    pid_t pid;

    int readCommand(Socket * socket);
    void read(Socket * socket, void* buffer, int size);
    void shutdownSmsc();
};

}
}

#endif
