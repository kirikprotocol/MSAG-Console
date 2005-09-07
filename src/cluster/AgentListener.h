#ifndef AgentListener_dot_h
#define AgentListener_dot_h

#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include <string>
#include "logger/Logger.h"

namespace smsc {
namespace cluster {

using smsc::core::network::Socket;
using smsc::core::threads::Thread;
using smsc::logger::Logger;

class AgentListener : public Thread
{
public:
    AgentListener()
        : stop(true), logger(Logger::getInstance("AgentListner"))
    {
	};
	virtual ~AgentListener()
	{
		sock.Close();
        Stop();
	};

    void init(std::string host, int port);
	void Start();
    virtual int Execute();
    
protected:
	Socket sock;
    bool stop;
    void Stop();
    Logger *logger;

    int readCommand(Socket * socket);
    void read(Socket * socket, void* buffer, int size);
    void shutdownSmsc();
};

}
}

#endif
