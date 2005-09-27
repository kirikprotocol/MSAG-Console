#ifndef __SCAG_SYSTEM_PERFORMANCE_HPP__
#define __SCAG_SYSTEM_PERFORMANCE_HPP__

#include <inttypes.h>
#include <time.h>
#include <string>

#include "core/threads/ThreadedTask.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"

#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "util/timeslotcounter.hpp"

namespace scag{
namespace performance{

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::threads;

using smsc::util::TimeSlotCounter;
using smsc::core::network::Socket;

struct PerformanceCounter{
  int32_t lastSecond;
  int32_t average;
  uint64_t total;
};


namespace Counters{
  enum PerfCntEnum{
    cntAccepted,
    cntRejected,
    cntDelivered,
    cntDeliverErr,
    cntTransOk,
    cntTransFail
  };
}

const int performanceCounters=6;

struct PerformanceData{
  uint32_t size;
  uint32_t eventQueueSize;
  time_t uptime;
  time_t now;
  uint32_t padding;
  uint32_t countersNumber;
  PerformanceCounter counters[performanceCounters];
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
  std::string host;
  int port;
};

}//performance
}//system

#endif
