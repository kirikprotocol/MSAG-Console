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
      uint32_t high;
      uint32_t low;
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
    ld.success.lastSecond=htonl(ld.success.lastSecond);
    ld.success.average=htonl(ld.success.average);
    ld.success.high=htonl(ld.success.high);
    ld.success.low=htonl(ld.success.low);

    ld.error.lastSecond=htonl(ld.error.lastSecond);
    ld.error.average=htonl(ld.error.average);
    ld.error.high=htonl(ld.error.high);
    ld.error.low=htonl(ld.error.low);

    ld.rescheduled.lastSecond=htonl(ld.rescheduled.lastSecond);
    ld.rescheduled.average=htonl(ld.rescheduled.average);
    ld.rescheduled.high=htonl(ld.rescheduled.high);
    ld.rescheduled.low=htonl(ld.rescheduled.low);
    ld.uptime=htonl(ld.uptime);
    ld.now=htonl(ld.now);

    for(int i=0;i<sockets.Count();i++)
    {
      int wr=sockets[i]->WriteAll((char*)data,sizeof(*data));
      if(wr!=sizeof(*data))
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
