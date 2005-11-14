#ifndef scag_performance_dot_h
#define scag_performance_dot_h

#include "core/network/Socket.hpp"
#include "util/timeslotcounter.hpp"
#include <string.h>

#define PERF_CNT_ACCEPTED       0
#define PERF_CNT_REJECTED       1
#define PERF_CNT_DELIVERED      2
#define PERF_CNT_GW_REJECTED    3
#define PERF_CNT_FAILED         4
#define PERF_CNT_COUNT          5

namespace scag {
namespace stat {

using smsc::util::TimeSlotCounter;
using smsc::core::network::Socket;

struct PerformanceCounter{
    int32_t lastSecond;
    int32_t average;
    uint64_t total;
};

struct PerformanceData{
  
    uint32_t size;
    uint32_t countersNumber;
    PerformanceCounter counters[PERF_CNT_COUNT];
    //uint32_t eventQueueSize;
    time_t uptime;
    time_t now;
    //uint32_t inProcessingCount;
    //uint32_t inScheduler;
};

struct SmePerformanceCounter
{
    uint16_t                counters[PERF_CNT_COUNT];
    TimeSlotCounter<int>*   slots   [PERF_CNT_COUNT];

    SmePerformanceCounter() { 
        memset(counters, 0, sizeof(counters));
        memset(slots, 0, sizeof(slots));
    };

    virtual ~SmePerformanceCounter() {
        for (int i=0; i<PERF_CNT_COUNT; i++) 
            if (slots[i]) delete slots[i];
    };

    inline void clear() {
        memset(counters, 0, sizeof(counters));
    };
};

class PerformanceListener{
public:
    virtual void getPerfData(uint64_t *cnt)=0;
    virtual void reportGenPerformance(PerformanceData * data)=0;
    virtual void reportSvcPerformance()=0;
    virtual void reportScPerformance()=0;
};

class PerformanceServer{
public:
    virtual void addSvcSocket(Socket * socket)=0;
    virtual void addScSocket(Socket * socket)=0;
    virtual void addGenSocket(Socket * socket)=0;
};

}
}

#endif
