#ifndef SCAG_PERS_SOCKETSERVER
#define SCAG_PERS_SOCKETSERVER

#include <string>
#include "RWMultiplexer.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "CmdDispatcher.h"

namespace scag { namespace pers {

using smsc::core::network::Socket;
using smsc::logger::Logger;

class PersServer {
	Multiplexer::SockArray socks;
	CommandDispatcher* CmdDispatcher;

public:
    PersServer(const char* persHost_, int persPort_, int maxClientCount_, CommandDispatcher *d);
    ~PersServer();
    int Execute();
	void Stop() { MutexGuard mt(mtx); isStopping = true; };
	bool isStopped() { MutexGuard mt(mtx); return isStopping; };

protected:
    void InitServer(const char* persHost_, int persPort_, int maxClientCount_);
	void process_read_socket(Socket* sock);
	void process_write_socket(Socket* sock);

    Logger * log;
    std::string persHost;
    int persPort;
	int maxClientCount;
	int clientCount;
    Socket sock;
    bool isStopping;
    RWMultiplexer listener;
    Mutex mtx;
};

}
}

#endif

