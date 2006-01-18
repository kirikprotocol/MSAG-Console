#ifndef scag_perfsocketserver_dot_h
#define scag_perfsocketserver_dot_h

#include "core/threads/Thread.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/network/Socket.hpp"
#include "Performance.h"
#include "logger/Logger.h"
#include <string>

namespace scag {
namespace stat {

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using smsc::logger::Logger;

class PerfSocketServer : public Thread {
public:
    PerfSocketServer();
    virtual ~PerfSocketServer();
    virtual int Execute();
    void init(PerformanceServer * perfServer);
    void InitServer(std::string perfHost_, int perfGenPort_, int perfSvcPort_, int perfScPort_);
    void reinitLogger();
    void Stop();
    void Start();
protected:
    Logger * logger;
    std::string perfHost;
    int perfGenPort;
    int perfSvcPort;
    int perfScPort;
    Socket genSocket;
    Socket svcSocket;
    Socket scSocket;
    PerformanceServer * performanceServer;
    bool isStopping;
    Multiplexer listener;
};

}
}

#endif

