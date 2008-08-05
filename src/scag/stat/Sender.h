#ifndef stat_sender_dot_h
#define stat_sender_dot_h

#include <string>
#include <core/network/Socket.hpp>
#include <core/threads/Thread.hpp>
#include <time.h>
#include <sys/types.h>
#include "Performance.h"
#include "PerfSocketServer.h"
#include "logger/Logger.h"

namespace scag {
namespace stat {

using smsc::core::network::Socket;
using smsc::core::threads::Thread;
using smsc::logger::Logger;

class Sender : public Thread {
public:
    Sender();
    virtual ~Sender();
    virtual int Execute();
    void Stop();
    void Start();
    void init(PerformanceListener* pl, PerformanceServer* ps);
    void InitServer(std::string perfHost, int genPort, int svcPort, int scPort);
    void reinitPrfSrvLogger();
protected:
    PerfSocketServer perfServer;
    int smppPerfCnt[PERF_CNT_COUNT][60];
    int httpPerfCnt[PERF_HTTP_COUNT][60];
    int timeshift;
    time_t times[60];
    timespec start;
    PerformanceListener* perfListener;
    bool isStopping;
    Logger * logger;
private:
    // to make compiler happy
    void Start(int);
};

}
}

#endif
