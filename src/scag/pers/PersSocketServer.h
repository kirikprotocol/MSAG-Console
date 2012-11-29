#ifndef SCAG_PERS_SOCKETSERVER
#define SCAG_PERS_SOCKETSERVER

#include <string>
#include "RWMultiplexer.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "scag/util/storage/SerialBuffer.h"

namespace scag { namespace pers {

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using smsc::core::network::Socket;

const uint32_t PACKET_LENGTH_SIZE = 4;

class ConnectionContext{
public:
	ConnectionContext(Socket* s) : wantRead(true), lastActivity(time(NULL)),
                                   authed(false), region_id(0), socket(s), batch(false),
                                   batch_cmd_count(0), transact_batch(false) {};
	SerialBuffer inbuf, outbuf;
	bool wantRead, authed;
	time_t lastActivity;
	uint32_t packetLen;
    uint32_t region_id;
    Socket* socket;
    bool batch;
    bool transact_batch;
    uint16_t batch_cmd_count;
    void cancelBatch() {
      batch = false;
      transact_batch = false;
      batch_cmd_count = 0;
    }
};

class PersSocketServer {
public:
    PersSocketServer(const char* persHost_, int persPort_, int maxClientCount_,
                     int timeout_, int transactTimeout_);
    virtual ~PersSocketServer();
    void InitServer();
    int Execute();
    void Stop() { smsc::core::synchronization::MutexGuard mt(mtx); isStopping = true; };
    bool isStopped() { smsc::core::synchronization::MutexGuard mt(mtx); return isStopping; };

protected:

    virtual bool processPacket(ConnectionContext& ctx) { return true; };
    virtual void processUplinkPacket(ConnectionContext& ctx) {};
    virtual void onDisconnect(ConnectionContext& ctx) {};    
    virtual bool bindToCP() { return true; };
    virtual void checkTimeouts(time_t curTime);
    virtual void checkTransactionsTimeouts(time_t curTime) {};

    void processReadSocket(Socket* sock);
    void processWriteSocket(Socket* sock);
    void removeSocket(Socket* s, int i = -1);

    Logger * log;
    std::string persHost;
    int persPort, timeout, maxClientCount, clientCount;
    int transactTimeout;
    Socket sock;
    bool isStopping;
    RWMultiplexer listener;
    smsc::core::synchronization::Mutex mtx;
    char tmp_buf[1024];
};

}}

#endif
