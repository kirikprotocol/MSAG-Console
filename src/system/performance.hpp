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

struct PerformanceData{
  PerformanceCounter success;
  PerformanceCounter error;
  PerformanceCounter rescheduled;
  time_t uptime;
  time_t now;
};

class PerformanceListener{
public:
  virtual void reportPerformance(PerformanceData* data)=0;
};

class PerformanceDataDispatcher:public PerformanceListener{
public:
  void reportPerformance(PerformanceData* data)
  {
    MutexGuard g(mtx);
    PerformanceData ld=*data;
    int high,low;
    ld.success.lastSecond=htonl(ld.success.lastSecond);
    ld.success.average=htonl(ld.success.average);

    low=ld.success.total&0xffffffff;
    high=ld.success.total>>32;
    ld.success.high=htonl(high);
    ld.success.low=htonl(low);

    ld.error.lastSecond=htonl(ld.error.lastSecond);
    ld.error.average=htonl(ld.error.average);
    low=ld.error.total&0xffffffff;
    high=ld.error.total>>32;
    ld.error.high=htonl(high);
    ld.error.low=htonl(low);

    ld.rescheduled.lastSecond=htonl(ld.rescheduled.lastSecond);
    ld.rescheduled.average=htonl(ld.rescheduled.average);
    low=ld.rescheduled.total&0xffffffff;
    high=ld.rescheduled.total>>32;
    ld.rescheduled.high=htonl(high);
    ld.rescheduled.low=htonl(low);
    ld.uptime=htonl(ld.uptime);
    ld.now=htonl(ld.now);

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
