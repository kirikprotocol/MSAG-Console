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

const int performanceCounters=6;

struct PerformanceData{
  /*
  PerformanceCounter submitOk;
  PerformanceCounter submitErr;
  PerformanceCounter deliverOk;
  PerformanceCounter deliverErrTerm;
  PerformanceCounter deliverErrPerm;
  PerformanceCounter rescheduled;
  */
  uint32_t size;
  uint32_t countersNumber;
  PerformanceCounter counters[performanceCounters];
  uint32_t eventQueueSize;
  time_t uptime;
  time_t now;
  uint32_t inProcessingCount;
  uint32_t inScheduler;
};

struct SmePerformanceCounter
{
    uint16_t   accepted, rejected;
    uint16_t   delivered, rescheduled;
    uint16_t   tempError, permError;

    SmePerformanceCounter() 
        : accepted(0), rejected(0), delivered(0), rescheduled(0), tempError(0), permError(0) {};
};

class SmePerformanceMonitor
{
private:

    Mutex                           countersLock;
    IntHash<uint16_t>               errCounters;
    Hash   <SmePerformanceCounter>  smeCounters;

    IntHash<TimeSlotCounter<int>*>   errorCounters;
    Hash   <TimeSlotCounter<int>*>   acceptedCounters,  rejectedCounters;
    Hash   <TimeSlotCounter<int>*>   deliveredCounters, rescheduledCounters;
    Hash   <TimeSlotCounter<int>*>   tempErrorCounters, permErrorCounters;
    
    inline TimeSlotCounter<int>* newSlotCounter() {
        return new TimeSlotCounter<int>(3600, 1000);
    }

    void incError(int errcode)
    {
        uint16_t* errCounter = errCounters.GetPtr(errcode);
        if (errCounter) errCounter++;
        else errCounters.Insert(errcode, 1);

        TimeSlotCounter<int>* errHourCounter = 0;
        if (errorCounters.Exist(errcode)) errHourCounter = errorCounters.Get(errcode);
        else {
            errHourCounter = newSlotCounter();
            errorCounters.Insert(errcode, errHourCounter);
        }
        if (errHourCounter) errHourCounter->Inc();
    };

    void clean(Hash<TimeSlotCounter<int>*>& hash)
    {
        char* key = 0; TimeSlotCounter<int>* counter = 0; hash.First();
        while (hash.Next(key, counter))
            if (counter) delete counter;
        hash.Empty();
    }
    void clean(IntHash<TimeSlotCounter<int>*>& hash)
    {
        IntHash<TimeSlotCounter<int>*>::Iterator it = hash.First();
        int key = 0; TimeSlotCounter<int>* counter = 0;
        while (it.Next(key, counter))
            if (counter) delete counter;
        hash.Empty();
    }

public:

    ~SmePerformanceMonitor()
    {
        MutexGuard guard(countersLock);

        clean(errorCounters);
        clean(acceptedCounters);  clean(rejectedCounters);
        clean(deliveredCounters); clean(rescheduledCounters);
        clean(tempErrorCounters); clean(permErrorCounters);
    }

    void incAccepted(const char* sme)
    {
        if (!sme || !sme[0]) return;
        
        MutexGuard guard(countersLock);
        
        SmePerformanceCounter* counter = smeCounters.GetPtr(sme);
        if (counter) counter->accepted++;
        else {
            SmePerformanceCounter cnt; cnt.accepted = 1;
            smeCounters.Insert(sme, cnt);
        }
        
        TimeSlotCounter<int>* hourCounter = 0;
        if (acceptedCounters.Exists(sme)) hourCounter = acceptedCounters.Get(sme);
        else {
            hourCounter = newSlotCounter();
            acceptedCounters.Insert(sme, hourCounter);
        }
        if (hourCounter) hourCounter->Inc();
    };
    void incRejected(const char* sme, int errcode)
    {
        if (!sme || !sme[0]) return;
        
        MutexGuard guard(countersLock);

        SmePerformanceCounter* counter = smeCounters.GetPtr(sme);
        if (counter) counter->rejected++;
        else {
            SmePerformanceCounter cnt; cnt.rejected = 1;
            smeCounters.Insert(sme, cnt);
        }
        
        TimeSlotCounter<int>* rejHourCounter = 0;
        if (rejectedCounters.Exists(sme)) rejHourCounter = rejectedCounters.Get(sme);
        else {
            rejHourCounter = newSlotCounter();
            rejectedCounters.Insert(sme, rejHourCounter);
        }
        if (rejHourCounter) rejHourCounter->Inc();
        
        incError(errcode);
    };
    void incDelivered(const char* sme)
    {
        if (!sme || !sme[0]) return;

        MutexGuard guard(countersLock);

        SmePerformanceCounter* counter = smeCounters.GetPtr(sme);
        if (counter) counter->delivered++;
        else {
            SmePerformanceCounter cnt; cnt.delivered = 1;
            smeCounters.Insert(sme, cnt);
        }

        TimeSlotCounter<int>* delHourCounter = 0;
        if (deliveredCounters.Exists(sme)) delHourCounter = deliveredCounters.Get(sme);
        else {
            delHourCounter = newSlotCounter();
            deliveredCounters.Insert(sme, delHourCounter);
        }
        if (delHourCounter) delHourCounter->Inc();
        
        incError(smsc::system::Status::OK);
    };
    void incRescheduled(const char* sme)
    {
        if (!sme || !sme[0]) return;
        
        MutexGuard guard(countersLock);

        SmePerformanceCounter* counter = smeCounters.GetPtr(sme);
        if (counter) counter->rescheduled++;
        else {
            SmePerformanceCounter cnt; cnt.rescheduled = 1;
            smeCounters.Insert(sme, cnt);
        }

        TimeSlotCounter<int>* resHourCounter = 0;
        if (rescheduledCounters.Exists(sme)) resHourCounter = rescheduledCounters.Get(sme);
        else {
            resHourCounter = newSlotCounter();
            rescheduledCounters.Insert(sme, resHourCounter);
        }
        if (resHourCounter) resHourCounter->Inc();
    };
    void incFailed(const char* sme, int errcode)
    {
        if (!sme || !sme[0]) return;
        
        bool permanent = smsc::system::Status::isErrorPermanent(errcode);
        MutexGuard guard(countersLock);
        
        SmePerformanceCounter* counter = smeCounters.GetPtr(sme);
        if (counter) {
            if (permanent) counter->permError++; 
            else counter->tempError++;
        }
        else {
            SmePerformanceCounter cnt;
            if (permanent) cnt.permError = 1;
            else cnt.tempError = 1;
            smeCounters.Insert(sme, cnt);
        }
        
        TimeSlotCounter<int>* errHourCounter = 0;
        if (permanent && permErrorCounters.Exists(sme)) errHourCounter = permErrorCounters.Get(sme);
        else if (!permanent && tempErrorCounters.Exists(sme)) errHourCounter = tempErrorCounters.Get(sme);
        else {
            errHourCounter = newSlotCounter();
            if (permanent) permErrorCounters.Insert(sme, errHourCounter);
            else tempErrorCounters.Insert(sme, errHourCounter);
        }
        if (errHourCounter) errHourCounter->Inc();

        incError(errcode);
    };

    uint8_t* dump(uint32_t& smePerfDataSize)
    {
        MutexGuard guard(countersLock);

        smePerfDataSize = sizeof(uint32_t)+sizeof(uint16_t)*2;
        char* sme = 0; SmePerformanceCounter* smeCounter = 0;
        smeCounters.First();
        while (smeCounters.Next(sme, smeCounter))
            if (sme && sme[0] && smeCounter)
                smePerfDataSize += strlen(sme)+sizeof(uint16_t)*13;
        smePerfDataSize += (sizeof(uint32_t)+sizeof(uint16_t)*2)*errCounters.Count();

        uint8_t* data = new uint8_t[smePerfDataSize];
        uint8_t* packet = data;

        // uint32_t     Total packet size
        uint32_t i32val = htonl(smePerfDataSize-sizeof(uint32_t));
        memcpy(packet, &i32val, sizeof(i32val)); packet += sizeof(i32val);
        
        // uint16_t     Sme(s) count
        uint16_t i16val = htons((uint16_t)smeCounters.GetCount());
        memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
        
        smeCounters.First();
        while (smeCounters.Next(sme, smeCounter))
        {
            if (!sme || !sme[0] || !smeCounter) continue;
            // uint16_t     sme name size
            int smeIdLen = strlen(sme); i16val = htons((uint16_t)smeIdLen);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            // uint8_t[]    sme name
            memcpy(packet, sme, smeIdLen); packet += smeIdLen;
            TimeSlotCounter<int>* cnt = 0;
            // uint16_t(2)  accepted counter + avg (hour)
            i16val = htons(smeCounter->accepted);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            cnt = acceptedCounters.Exists(sme) ? acceptedCounters.Get(sme):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            // uint16_t(2)  rejected counter + avg (hour)
            i16val = htons(smeCounter->rejected);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            cnt = rejectedCounters.Exists(sme) ? rejectedCounters.Get(sme):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            // uint16_t(2)  delivered counter + avg (hour)
            i16val = htons(smeCounter->delivered);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            cnt = deliveredCounters.Exists(sme) ? deliveredCounters.Get(sme):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            // uint16_t(2)  rescheduled counter + avg (hour)
            i16val = htons(smeCounter->rescheduled);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            cnt = rescheduledCounters.Exists(sme) ? rescheduledCounters.Get(sme):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            // uint16_t(2)  tempError counter + avg (hour)
            i16val = htons(smeCounter->tempError);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            cnt = tempErrorCounters.Exists(sme) ? tempErrorCounters.Get(sme):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            // uint16_t(2)  permError counter + avg (hour)
            i16val = htons(smeCounter->permError);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            cnt = permErrorCounters.Exists(sme) ? permErrorCounters.Get(sme):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
        }   

        // uint16_t     Error(s) count
        i16val = htons((uint16_t)errCounters.Count());
        memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);

        IntHash<uint16_t>::Iterator it = errCounters.First();
        int errcode = 0; uint16_t counter = 0;
        while (it.Next(errcode, counter))
        {
            if (!counter) continue;
            // uint32_t     error code
            i32val = htonl((uint32_t)errcode);
            memcpy(packet, &i32val, sizeof(i32val)); packet += sizeof(i32val);
            //uint16_t(2)  error counter + avg (hour)
            i16val = htons(counter);
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
            TimeSlotCounter<int>* cnt = errorCounters.Exist(errcode) ? errorCounters.Get(errcode):0;
            i16val = (cnt) ? htons((uint16_t)cnt->Avg()):0;
            memcpy(packet, &i16val, sizeof(i16val)); packet += sizeof(i16val);
        }
        
        errCounters.Empty(); smeCounters.Empty();
        return data;
    };
};

class PerformanceListener{
public:
  virtual void reportGenPerformance(PerformanceData* data)=0;
  virtual void reportSmePerformance(uint8_t* data, uint32_t size)=0;
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
    int high,low;

    ld.size=htonl(sizeof(ld));
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
      uint64_t tmp=ld.counters[i].total;
      unsigned char *ptr=(unsigned char *)&ld.counters[i].total;
      ptr[0]=(unsigned char)(tmp>>56);
      ptr[1]=(unsigned char)(tmp>>48)&0xFF;
      ptr[2]=(unsigned char)(tmp>>40)&0xFF;
      ptr[3]=(unsigned char)(tmp>>32)&0xFF;
      ptr[4]=(unsigned char)(tmp>>24)&0xFF;
      ptr[5]=(unsigned char)(tmp>>16)&0xFF;
      ptr[6]=(unsigned char)(tmp>>8)&0xFF;
      ptr[7]=(unsigned char)(tmp&0xFF);
    }

    ld.uptime=htonl(ld.uptime);
    ld.now=htonl(ld.now);
    ld.eventQueueSize=htonl(ld.eventQueueSize);
    ld.inProcessingCount=htonl(ld.inProcessingCount);
    ld.inScheduler=htonl(ld.inScheduler);

    for(int i=0;i<sockets.Count();i++)
    {
      int wr=sockets[i]->WriteAll((char*)&ld,sizeof(ld));

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

      if(wr!=size)
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

class PerformanceServer:public ThreadedTask{
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
    setsockopt(srv.getSocket(),SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));
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
  string host;
  int port;
};

}//performance
}//system
}//smsc

#endif
