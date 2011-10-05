#ifndef __SMSC_SYSTEM_PERFORMANCE_HPP__
#define __SMSC_SYSTEM_PERFORMANCE_HPP__

#include <inttypes.h>
#include <time.h>
#include <string>

#include "core/threads/ThreadedTask.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"

#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "util/timeslotcounter.hpp"
#include "util/Uint64Converter.h"

namespace smsc{
namespace system{
namespace performance{

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

using smsc::util::TimeSlotCounter;
using smsc::core::network::Socket;

struct PerformanceCounter{
  int32_t lastSecond;
  int32_t average;
  uint64_t total;
};

const int performanceCounters=12;

struct PerformanceData{
  uint32_t size;
  uint32_t countersNumber;
  PerformanceCounter counters[performanceCounters];
  time_t uptime;
  time_t now;
  uint32_t eventQueueSize;
  uint32_t inProcessingCount;
  uint32_t inScheduler;
  uint32_t dpfCount;
};

#define SME_PERF_CNT_ACCEPTED       0
#define SME_PERF_CNT_REJECTED       1
#define SME_PERF_CNT_DELIVERED      2
#define SME_PERF_CNT_RESCHEDULED    3
#define SME_PERF_CNT_TEMP_ERROR     4
#define SME_PERF_CNT_PERM_ERROR     5
#define SME_PERF_CNT_COUNT          6

struct SmePerformanceCounter
{
    TimeSlotCounter<int>*   slots   [SME_PERF_CNT_COUNT];
    uint16_t                counters[SME_PERF_CNT_COUNT];

    SmePerformanceCounter() {
        memset(counters, 0, sizeof(counters));
        memset(slots, 0, sizeof(slots));
    };
    virtual ~SmePerformanceCounter() {
        for (int i=0; i<SME_PERF_CNT_COUNT; i++)
            if (slots[i]) delete slots[i];
    };
    inline void clear() {
        memset(counters, 0, sizeof(counters));
    };
};
struct SmeErrorCounter
{
    TimeSlotCounter<int>*   slot;
    uint16_t                errors;

    SmeErrorCounter() : slot(0),errors(0) {};
    virtual ~SmeErrorCounter() {
        if (slot) delete slot;
    };
};

class SmePerformanceMonitor
{
private:

    Mutex                           countersLock;

    Hash   <SmePerformanceCounter*>  smeCounters;
    IntHash<SmeErrorCounter*>        errCounters;

    inline TimeSlotCounter<int>* newSlotCounter() {
        return new TimeSlotCounter<int>(3600, 1000);
    }

    void incError(int errcode)
    {
        SmeErrorCounter*  counter = 0;
        SmeErrorCounter** errCounter = errCounters.GetPtr(errcode);
        if (!errCounter) counter = new SmeErrorCounter();
        else if (*errCounter) counter = *errCounter;
        else {
            counter = new SmeErrorCounter();
            errCounter = 0; errCounters.Delete(errcode);
        }

        counter->errors++;
        if (!counter->slot) counter->slot = newSlotCounter();
        counter->slot->Inc();

        if (!errCounter) errCounters.Insert(errcode, counter);
    };

    void incCounter(const char* sme, int index)
    {
        if (!sme || !sme[0] || index<0 || index>=SME_PERF_CNT_COUNT) return;

        SmePerformanceCounter*  counter = 0;
        SmePerformanceCounter** smeCounter = smeCounters.GetPtr(sme);

        if (!smeCounter) counter = new SmePerformanceCounter();
        else if (*smeCounter) counter = *smeCounter;
        else {
            counter = new SmePerformanceCounter();
            smeCounter = 0; smeCounters.Delete(sme);
        }

        counter->counters[index]++;
        if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
        counter->slots[index]->Inc();

        if (!smeCounter) smeCounters.Insert(sme, counter);
    }

public:

    ~SmePerformanceMonitor()
    {
        MutexGuard guard(countersLock);

        char* key = 0; SmePerformanceCounter* smeCounter = 0;
        smeCounters.First();
        while (smeCounters.Next(key, smeCounter))
            if (smeCounter) delete smeCounter;
        smeCounters.Empty();

        IntHash<SmeErrorCounter*>::Iterator it = errCounters.First();
        int code = 0; SmeErrorCounter* errCounter = 0;
        while (it.Next(code, errCounter))
            if (errCounter) delete errCounter;
        errCounters.Empty();
    }

    void incAccepted(const char* sme)
    {
        MutexGuard guard(countersLock);
        incCounter(sme, SME_PERF_CNT_ACCEPTED);
    };
    void incRejected(const char* sme, int errcode)
    {
        MutexGuard guard(countersLock);
        incCounter(sme, SME_PERF_CNT_REJECTED);
        incError(errcode);
    };
    void incDelivered(const char* sme)
    {
        MutexGuard guard(countersLock);
        incCounter(sme, SME_PERF_CNT_DELIVERED);
        incError(smsc::system::Status::OK);
    };
    void incRescheduled(const char* sme)
    {
        MutexGuard guard(countersLock);
        incCounter(sme, SME_PERF_CNT_RESCHEDULED);
    };
    void incFailed(const char* sme, int errcode)
    {
        bool permanent = smsc::system::Status::isErrorPermanent(errcode);
        MutexGuard guard(countersLock);
        incCounter(sme, permanent ? SME_PERF_CNT_PERM_ERROR:SME_PERF_CNT_TEMP_ERROR);
        incError(errcode);
    };

    uint8_t* dump(uint32_t& smePerfDataSize)
    {
        MutexGuard guard(countersLock);

        size_t dmpsz = sizeof(uint32_t)+sizeof(uint16_t)*2+
            (sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+
             sizeof(uint16_t)*2*SME_PERF_CNT_COUNT)*smeCounters.GetCount()+
            (sizeof(uint32_t)+sizeof(uint16_t)*2)*errCounters.Count();

        smePerfDataSize=(uint32_t)dmpsz;

        uint8_t* data = new uint8_t[smePerfDataSize];
        uint8_t* packet = data; memset(packet, 0, smePerfDataSize);

        // uint32_t     Total packet size
        *((uint32_t*)packet) = htonl(smePerfDataSize-(uint32_t)sizeof(uint32_t)); packet += sizeof(uint32_t);
        // uint16_t     Sme(s) count
        *((uint16_t*)packet) = htons((uint16_t)smeCounters.GetCount()); packet += sizeof(uint16_t);

        smeCounters.First();
        char* sme = 0; SmePerformanceCounter* smeCounter = 0;
        while (smeCounters.Next(sme, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (sme) strncpy((char *)packet, sme, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;

            for (int i=0; i<SME_PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
        }

        // uint16_t     Errors count
        *((uint16_t*)packet) = htons((uint16_t)errCounters.Count()); packet += sizeof(uint16_t);

        IntHash<SmeErrorCounter*>::Iterator it = errCounters.First();
        int errcode = 0; SmeErrorCounter* errCounter = 0;
        while (it.Next(errcode, errCounter))
        {
            // uint32_t     error code
            *((uint32_t*)packet) = htonl((uint32_t)errcode); packet += sizeof(uint32_t);
            // uint16_t(2)  error counter + avg (hour)
            *((uint16_t*)packet) = (errCounter) ? htons(errCounter->errors):0; packet += sizeof(uint16_t);
            TimeSlotCounter<int>* cnt = (errCounter && errCounter->slot) ? errCounter->slot:0;
            *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            if (errCounter) errCounter->errors = 0;
        }

        return data;
    };
};

class PerformanceListener{
public:
  virtual void reportGenPerformance(PerformanceData* data)=0;
  virtual void reportSmePerformance(uint8_t* data, uint32_t size)=0;
  virtual ~PerformanceListener(){}
};

class PerformanceDataDispatcher:public PerformanceListener{
public:
  PerformanceDataDispatcher(){}
  virtual ~PerformanceDataDispatcher()
  {
    for(int i=0;i<sockets.Count();i++)
    {
      sockets[i]->Abort();
      delete sockets[i];
    }
  }
  void reportGenPerformance(PerformanceData* data)
  {
    MutexGuard g(mtx);
    PerformanceData ld=*data;
    //int high,low;

    ld.size=htonl((uint32_t)sizeof(ld));
    ld.countersNumber=htonl(ld.countersNumber);


    for(int i=0;i<performanceCounters;i++)
    {
      ld.counters[i].lastSecond=htonl(ld.counters[i].lastSecond);
      ld.counters[i].average=htonl(ld.counters[i].average);

      /*
      low=(int)(ld.counters[i].total&0xffffffff);
      high=(int)(ld.counters[i].total>>32);
      ld.counters[i].total=(((uint64_t)htonl(high))<<32) | htonl(low);
      */
      /*uint64_t tmp=ld.counters[i].total;
      unsigned char *ptr=(unsigned char *)&ld.counters[i].total;
      ptr[0]=(unsigned char)(tmp>>56);
      ptr[1]=(unsigned char)(tmp>>48)&0xFF;
      ptr[2]=(unsigned char)(tmp>>40)&0xFF;
      ptr[3]=(unsigned char)(tmp>>32)&0xFF;
      ptr[4]=(unsigned char)(tmp>>24)&0xFF;
      ptr[5]=(unsigned char)(tmp>>16)&0xFF;
      ptr[6]=(unsigned char)(tmp>>8)&0xFF;
      ptr[7]=(unsigned char)(tmp&0xFF);*/
      ld.counters[i].total=smsc::util::Uint64Converter::toNetworkOrder(ld.counters[i].total);
    }

    ld.uptime=smsc::util::Uint64Converter::toNetworkOrder(ld.uptime);
    ld.now=smsc::util::Uint64Converter::toNetworkOrder(ld.now);
    ld.eventQueueSize=htonl(ld.eventQueueSize);
    ld.inProcessingCount=htonl(ld.inProcessingCount);
    ld.inScheduler=htonl(ld.inScheduler);
    ld.dpfCount=htonl(ld.dpfCount);

    for(int i=0;i<sockets.Count();i++)
    {
      int wr=sockets[i]->WriteAll((char*)&ld,(int)sizeof(ld));

      if(wr!=sizeof(ld))
      {
        sockets[i]->Abort();
        delete sockets[i];
        sockets.Delete(i);
        i--;
      }
    }
  }
  void reportSmePerformance(uint8_t* data, uint32_t size)
  {
    if (!data || size < sizeof(uint32_t)) {
        __trace2__("invalid sme performance data (size=%d)", size);
        return;
    }

    for(int i=0;i<sockets.Count();i++)
    {
      int wr=sockets[i]->WriteAll((char*)data, size);

      if(wr!=(int)size)
      {
        sockets[i]->Abort();
        delete sockets[i];
        sockets.Delete(i);
        i--;
      }
    }
  }
  void addSocket(Socket* s)
  {
    MutexGuard g(mtx);
    s->setNonBlocking(1);
    char buf[32];
    s->GetPeer(buf);
    __trace2__("performance::add connect from %s",buf);
    sockets.Push(s);
  }
protected:
  Array<Socket*> sockets;
  Mutex mtx;
};

class PerformanceServer:public smsc::core::threads::ThreadedTask{
public:
  PerformanceServer(const char* h,int p,PerformanceDataDispatcher* disp):
    disp(disp),host(h),port(p)
  {
  }
  const char* taskName(){return "PerformanceServer";}
  int Execute()
  {
    while(srv.InitServer(host.c_str(),port,0)==-1)
    {
      __warning2__("Failed to init PerformanceServer:%s:%d",host.c_str(),port);
      sleep(5);
      if(isStopping)return 0;
    }
    while(srv.StartServer()==-1)
    {
      __warning2__("Failed to start PerformanceServer:%s:%d",host.c_str(),port);
      sleep(5);
      if(isStopping)return 0;
    }
    linger l;
    l.l_onoff=1;
    l.l_linger=0;
    setsockopt(srv.getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,(int)sizeof(l));
    while(!isStopping)
    {
      Socket *clnt=srv.Accept();
      if(!clnt)continue;
      disp->addSocket(clnt);
    }
    return 0;
  }
protected:
  Socket srv;
  PerformanceDataDispatcher* disp;
  std::string host;
  int port;
};

}//performance
}//system
}//smsc

#endif
