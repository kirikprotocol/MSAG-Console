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
    CommandDispatcher* CmdDispatcher;

public:
    PersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, CommandDispatcher *d);
    ~PersServer();
    void InitServer();
    int Execute();
    void Stop() { MutexGuard mt(mtx); isStopping = true; };
    bool isStopped() { MutexGuard mt(mtx); return isStopping; };

protected:
    void processReadSocket(Socket* sock);
    void processWriteSocket(Socket* sock);
    void removeSocket(Socket* s, int i = -1);
	void checkTimeouts();

    Logger * log;
    std::string persHost;
    int persPort, timeout, maxClientCount, clientCount;
    Socket sock;
    bool isStopping;
    RWMultiplexer listener;
    Mutex mtx;
    char tmp_buf[1024];
};

}}

#endif
