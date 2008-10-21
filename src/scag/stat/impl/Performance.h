#ifndef scag_performance2_dot_h
#define scag_performance2_dot_h

#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/timeslotcounter.hpp"
#include "scag/stat/base/Statistics2.h"
#include <string.h>

#define PERF_CNT_ACCEPTED       0
#define PERF_CNT_REJECTED       1
#define PERF_CNT_DELIVERED      2
#define PERF_CNT_GW_REJECTED    3
#define PERF_CNT_FAILED         4
#define PERF_CNT_COUNT          5

#define PERF_HTTP_REQUEST               0
#define PERF_HTTP_REQUEST_REJECTED      1
#define PERF_HTTP_RESPONSE              2
#define PERF_HTTP_RESPONSE_REJECTED     3
#define PERF_HTTP_DELIVERED             4
#define PERF_HTTP_FAILED                5
#define PERF_HTTP_COUNT                 6

namespace scag2 {
namespace stat {

using smsc::util::TimeSlotCounter;
using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

typedef struct {
    int32_t lastSecond;
    int32_t average;
    uint64_t total;
} PerformanceCounter;
//__attribute__((packed)) 
typedef struct {
  
//    uint32_t size;
    uint32_t sessionCount, sessionLoadedCount, sessionLockedCount;
    uint32_t smppReqQueueLen, smppRespQueueLen, smppLCMQueueLen;
    uint32_t httpReqQueueLen, httpRespQueueLen, httpLCMQueueLen;
    time_t uptime;
    time_t now;
    uint32_t smppCountersNumber;
    PerformanceCounter smppCounters[PERF_CNT_COUNT];
    uint32_t httpCountersNumber;
    PerformanceCounter httpCounters[PERF_HTTP_COUNT];
    //uint32_t inProcessingCount;
    //uint32_t inScheduler;
} PerformanceData;

struct CommonPerformanceCounter
{
    uint32_t count;
    uint16_t*                counters;
    TimeSlotCounter<int>**   slots;

    CommonPerformanceCounter(uint32_t cnt) { 
        count = cnt;

        counters = new uint16_t[cnt];
        slots = new TimeSlotCounter<int>*[cnt];

        memset(counters, 0, sizeof(uint16_t) * cnt);
        memset(slots, 0, sizeof(TimeSlotCounter<int>*) * cnt);
    };

    virtual ~CommonPerformanceCounter() {

        for (uint32_t i = 0; i < count; i++) 
            if (slots[i]) delete slots[i];

        delete counters;
        delete slots;
    };

    inline void clear() {
        memset(counters, 0, sizeof(uint16_t) * count);
    };
};

class PerformanceListener{
public:
    virtual void getSmppPerfData(uint64_t *cnt)=0;
    virtual void getHttpPerfData(uint64_t *cnt)=0;
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
    uint32_t count;
    uint64_t* counters;

    Mutex statLock;

    GenStatistics(uint32_t cnt)
    {
        count = cnt;
        counters = new uint64_t[cnt];
        memset(counters, 0, sizeof(uint64_t) * count);
    }

    ~GenStatistics()
    {
        delete counters;
    }

    void clear()
    {
        MutexGuard g(statLock);
        memset(counters, 0, sizeof(uint64_t) * count);
    }

    void inc(int counter)
    {
        MutexGuard g(statLock);

        if(uint32_t(counter) >= count)
            return;

        counters[counter]++;
    }

    void getCounters(uint64_t * cnt)
    {
        MutexGuard g(statLock);
        memcpy(cnt, counters, sizeof(uint64_t) * count);
    }
};

}
}

#endif
