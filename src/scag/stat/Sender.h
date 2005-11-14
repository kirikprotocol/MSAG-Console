#ifndef stat_sender_dot_h
#define stat_sender_dot_h

#include <string>
#include <core/network/Socket.hpp>
#include <core/threads/Thread.hpp>
#include <time.h>
#include <sys/types.h>
#include "Performance.h"
#include "PerfSocketServer.h"

namespace scag {
namespace stat {

using smsc::core::network::Socket;
using smsc::core::threads::Thread;

class Sender : public Thread {
public:
	Sender();
	virtual ~Sender();
    virtual int Execute();
    void Stop();
    void Start();
    void init(PerformanceListener* pl, PerformanceServer* ps);
    void InitServer(std::string perfHost, int genPort, int svcPort, int scPort);
protected:
	PerfSocketServer perfServer;
    int perfCnt[PERF_CNT_COUNT][60];
    int timeshift;
    time_t times[60];
    timespec start;
    PerformanceListener* perfListener;
    bool isStopping;
};

}
}

#endif
