#include <stdio.h>
#include <time.h>
#include <string>
#include <ctype.h>
#include <algorithm>
#include "core/buffers/FastMTQueue.hpp"
#include "core/threads/Thread.hpp"
#include <signal.h>

#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Event.hpp"

#ifndef _WIN32
#include <thread.h>
#include <unistd.h>
void Sleep(int msec)
{
  if(msec==0)thr_yield();
  else usleep(msec*1000);
}
#endif

using namespace smsc::core::buffers;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;

bool stopFlag=false;

template <class T>
class SyncQeuue{
public:
  void Push(const T& item)
  {
    MutexGuard g(mtx);
    q.Push(item);
    mtx.notify();
  }
  bool Pop(T& item)
  {
    MutexGuard g(mtx);
    return q.Pop(item);
  }
  void Wait()
  {
    MutexGuard g(mtx);
    mtx.wait();
  }
  int Count()
  {
    MutexGuard g(mtx);
    return q.Count();
  }
protected:
  CyclicQueue<T> q;
  EventMonitor mtx;
};

const int N=8;
//FastMTQueue<std::string> q[N];
SyncQeuue<std::string> q[N];

Event evt;

const int MAXCNT=5000000;
Mutex cntMtx;
int cnt;

int IncCnt()
{
  MutexGuard g(cntMtx);
  cnt++;
  return cnt;
}


class Writer:public Thread{
public:
  int Execute()
  {
    //for(;!stopFlag;)
    for(;!stopFlag;)
    {
      for(int i=0;i<N;i++)
      {
        if(q[i].Count()>50000)continue;
        q[i].Push("hello world");
        if(IncCnt()>MAXCNT)
        {
          stopFlag=true;
          break;
        };
        evt.Signal();
      }
    }
    stopFlag=true;
    return 1;
  }
};


class Reader:public Thread{
public:
  int Execute()
  {
    std::string s;
    FILE *f=fopen("out.txt","wt");
    if(!f)return 0;
    time_t last=time(NULL);
    int cnt=0;
    for(;!stopFlag;)
    {
      bool wasPop=false;
      for(int i=0;i<N;i++)
      {
        if(q[i].Pop(s))
        {
          fwrite(s.c_str(),s.length(),1,f);
          cnt++;
          wasPop=true;
        }
      }
      if(!wasPop)evt.Wait();
      time_t now=time(NULL);
      if(now!=last)
      {
        last=now;
        printf("Read speed = %d\n",cnt);
        cnt=0;
      }
    }
    fclose(f);
    return 1;
  }
};

void stop(int)
{
  stopFlag=true;
}

int main(int argc,char* argv[])
{
  sigset(SIGINT,stop);
  Reader r;
  Writer w1,w2;
  r.Start();
  w1.Start();
  w2.Start();
  r.WaitFor();
  w1.WaitFor();
  w2.WaitFor();
  return 0;
}
