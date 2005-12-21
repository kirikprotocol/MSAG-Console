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

#define PERF_HTTP_REQUEST               0
#define PERF_HTTP_REQUEST_REJECTED      1
#define PERF_HTTP_RESPONSE              2
#define PERF_HTTP_RESPONSE_REJECTED     3
#define PERF_HTTP_DELIVERED             4
#define PERF_HTTP_FAILED                5
#define PERF_HTTP_COUNT                 6

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
    uint32_t smppCountersNumber;
    PerformanceCounter smppCounters[PERF_CNT_COUNT];
    /*uint32_t httpCountersNumber;
    PerformanceCounter httpCounters[PERF_HTTP_COUNT];*/
    uint32_t sessionCount;
    time_t uptime;
    time_t now;
    //uint32_t inProcessingCount;
    //uint32_t inScheduler;
};

struct SmppPerformanceCounter
{
    uint16_t                counters[PERF_CNT_COUNT];
    TimeSlotCounter<int>*   slots   [PERF_CNT_COUNT];

    SmppPerformanceCounter() { 
        memset(counters, 0, sizeof(counters));
        memset(slots, 0, sizeof(slots));
    };

    virtual ~SmppPerformanceCounter() {
        for (int i=0; i<PERF_CNT_COUNT; i++) 
            if (slots[i]) delete slots[i];
    };

    inline void clear() {
        memset(counters, 0, sizeof(counters));
    };
};

struct HttpPerformanceCounter
{
    uint16_t                counters[PERF_HTTP_COUNT];
    TimeSlotCounter<int>*   slots   [PERF_HTTP_COUNT];

    HttpPerformanceCounter() { 
        memset(counters, 0, sizeof(counters));
        memset(slots, 0, sizeof(slots));
    };

    virtual ~HttpPerformanceCounter() {
        for (int i=0; i<PERF_HTTP_COUNT; i++) 
            if (slots[i]) delete slots[i];
    };

    inline void clear() {
        memset(counters, 0, sizeof(counters));
    };
};

class PerformanceListener{
public:
    virtual void getSmppPerfData(uint64_t *cnt)=0;
    //virtual void getHttpPerfData(uint64_t *cnt)=0;
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
    uint64_t accepted;
    uint64_t rejected;
    uint64_t delivered;
    uint64_t gw_rejected;
    uint64_t failed;

    uint64_t cntHttpRequest;
    uint64_t cntHttpRequestRejected;
    uint64_t cntHttpResponse;
    uint64_t cntHttpResponseRejected;
    uint64_t cntHttpDelivered;
    uint64_t cntHttpFailed;

    Mutex statLock;
    GenStatistics()
        : accepted(0),
          rejected(0),
          delivered(0),
          gw_rejected(0),
          failed(0),

          cntHttpRequest(0),
          cntHttpRequestRejected(0),
          cntHttpResponse(0),
          cntHttpResponseRejected(0),
          cntHttpDelivered(0),
          cntHttpFailed(0)
    {}
    void clear()
    {
        MutexGuard g(statLock);

        accepted = 0;
        rejected = 0;
        delivered = 0;
        gw_rejected = 0;
        failed = 0;

        cntHttpRequest = 0;
        cntHttpRequestRejected = 0;
        cntHttpResponse = 0;
        cntHttpResponseRejected = 0;
        cntHttpDelivered = 0;
        cntHttpFailed = 0;
    }
    void incSmpp(int counter)
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

    void incHttp(int counter)
    {
        MutexGuard g(statLock);

        using namespace Counters;
        switch(counter)
        {
        case httpRequest:
            ++cntHttpRequest;
            break;
        case httpRequestRejected:
            ++cntHttpRequestRejected;
            break;
        case httpResponse:
            ++cntHttpResponse;
            break;
        case httpResponseRejected:
            ++cntHttpResponseRejected;
            break;
        case httpDelivered:
            ++cntHttpDelivered;
            break;
        case httpFailed:
            ++cntHttpFailed;
            break;
        }
    }

    void getSmppCounters(uint64_t * cnt)
    {
        MutexGuard g(statLock);

        cnt[0]=accepted;
        cnt[1]=rejected;
        cnt[2]=delivered;
        cnt[3]=gw_rejected;
        cnt[4]=failed;
    }

    void getHttpCounters(uint64_t * cnt)
    {
        MutexGuard g(statLock);

        cnt[0]=cntHttpRequest;
        cnt[1]=cntHttpRequestRejected;
        cnt[2]=cntHttpResponse;
        cnt[3]=cntHttpResponseRejected;
        cnt[4]=cntHttpDelivered;
        cnt[5]=cntHttpFailed;
    }

};

}
}

#endif
