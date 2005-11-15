#ifndef scag_performance_dot_h
#define scag_performance_dot_h

#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/timeslotcounter.hpp"
#include "Statistics.h"
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
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

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

struct GenStatistics
{
    int accepted;
    int rejected;
    int delivered;
    int gw_rejected;
    int failed;
    Mutex statLock;
    GenStatistics()
        : accepted(0),
          rejected(0),
          delivered(0),
          gw_rejected(0),
          failed(0)
    {}
    void clear()
    {
        MutexGuard g(statLock);

        accepted = 0;
        rejected = 0;
        delivered = 0;
        gw_rejected = 0;
        failed = 0;
    }
    void inc(int counter)
    {
        MutexGuard g(statLock);

        using namespace Counters;
        switch(counter)
        {
        case cntAccepted:
            ++accepted;
            break;
        case cntRejected:
            ++rejected;
            break;
        case cntDelivered:
            ++delivered;
            break;
        case cntGw_Rejected:
            ++gw_rejected;
            break;
        case cntFailed:
            ++failed;
            break;
        }
    }

    void getCounters(uint64_t * cnt)
    {
        MutexGuard g(statLock);

        cnt[0]=accepted;
        cnt[1]=rejected;
        cnt[2]=delivered;
        cnt[3]=gw_rejected;
        cnt[4]=failed;
    }

};

}
}

#endif
