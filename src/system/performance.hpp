#ifndef __SMSC_SYSTEM_PERFORMANCE_HPP__
#define __SMSC_SYSTEM_PERFORMANCE_HPP__

#include <inttypes.h>
#include <time.h>
#include <string>

#include "core/threads/ThreadedTask.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace system{
namespace performance{

using namespace smsc::core::synchronization;
using smsc::core::buffers::Array;
using smsc::core::network::Socket;

struct PerformanceCounter{
  int lastSecond;
  int average;
  union{
    uint64_t total;
    struct{
      uint32_t low;
      uint32_t high;
    };
  };
};

const int performanceCounters=5;

struct PerformanceData{
  /*
  PerformanceCounter submitOk;
  PerformanceCounter submitErr;
  PerformanceCounter deliverOk;
  PerformanceCounter deliverErr;
  PerformanceCounter rescheduled;
  */
  PerformanceCounter counters[performanceCounters];
  int eventQueueSize;
  time_t uptime;
  time_t now;
};

class PerformanceListener{
public:
  virtual void reportPerformance(PerformanceData* data)=0;
};

class PerformanceDataDispatcher:public PerformanceListener{
public:
  PerformanceDataDispatcher(){}
  ~PerformanceDataDispatcher(){}
  void reportPerformance(PerformanceData* data)
  {
    MutexGuard g(mtx);
    PerformanceData ld=*data;
    int high,low;

    for(int i=0;i<performanceCounters;i++)
    {
      ld.counters[i].lastSecond=htonl(ld.counters[i].lastSecond);
      ld.counters[i].average=htonl(ld.counters[i].average);

      low=ld.counters[i].total&0xffffffff;
      high=ld.counters[i].total>>32;
      ld.counters[i].high=htonl(high);
      ld.counters[i].low=htonl(low);
    }

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
  void addSocket(Socket* s)
  {
    MutexGuard g(mtx);
    s->setNonBlocking(1);
    sockets.Push(s);
  }
protected:
  Array<Socket*> sockets;
  Mutex mtx;
};

class PerformanceServer:public ThreadedTask{
public:
  PerformanceServer(const char* host,int port,PerformanceDataDispatcher* disp):disp(disp)
  {
    if(srv.InitServer(host,port,0)==-1)
      throw Exception("Failed to init PerformanceServer");
    if(srv.StartServer()==-1)
      throw Exception("Failed to start PerformanceServer");
  }
  const char* taskName(){return "PerformanceServer";}
  int Execute()
  {
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
};

};//performance
};//system
};//smsc

#endif
