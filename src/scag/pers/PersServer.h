#ifndef SCAG_PERS_SOCKETSERVER
#define SCAG_PERS_SOCKETSERVER

#include <string>
#include "core/network/Multiplexer.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "SerialBuffer.h"

namespace scag { namespace pers {

using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using smsc::logger::Logger;

class SocketBuffer
{
	uint32_t size;
	uint32_t read;
	SerialBuffer sb;
public:
};

class PersServer {
	Multiplexer::SockArray socks;
public:
    PersServer();
    ~PersServer();
    int Execute();
    void InitServer(const char* persHost_, int persPort_, int maxClientCount_);
	void Stop() { MutexGuard mt(mtx); isStopping = true; };
	bool isStopped() { MutexGuard mt(mtx); return isStopping; };
protected:
    Logger * log;
    std::string persHost;
    int persPort;
	int maxClientCount;
    Socket sock;
    bool isStopping;
    Multiplexer listener;
    Mutex mtx;
};

}
}

#endif

