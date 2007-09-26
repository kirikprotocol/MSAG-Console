#ifndef SCAG_PERS_SOCKETSERVER
#define SCAG_PERS_SOCKETSERVER

#include <string>
#include "RWMultiplexer.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "SerialBuffer.h"

namespace scag { namespace pers {

using smsc::core::network::Socket;
using smsc::logger::Logger;

class PersSocketServer {
public:
    PersSocketServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_);
    virtual ~PersSocketServer();
    void InitServer();
    int Execute();
    virtual void processPacket(SerialBuffer& isb, SerialBuffer& osb) {};
    virtual void processUplinkPacket(SerialBuffer& isb, SerialBuffer& osb) {};
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
