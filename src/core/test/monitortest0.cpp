#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/buffers/Array.hpp"

#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

class ResourceClass{
  Array<int> used,free;
  EventMonitor m;
public:
  ResourceClass()
  {
    for(int i=0;i<3;i++)free.Push(i);
  }
  int getResource(int id)
  {
    MutexGuard guard(m);
    while(free.Count()==0)
    {
      m.wait();
    }
    int res;
    free.Pop(res);
    used.Push(res);
    return res;
  }
  void freeResource(int res,int id)
  {
    MutexGuard guard(m);
    for(int i=0;i<used.Count();i++)
    {
      if(used[i]==res)
      {
        free.Push(res);
        used.Delete(i);
        break;
      }
    }
    m.notify();
  }
};


ResourceClass rc;

class TestTask:public ThreadedTask{
  int count;
  int num;
public:
  TestTask(int n):num(n){}
  virtual int Execute();
  virtual char* taskName(){return "testtask";}
  int getCount(){return count;}
};

int TestTask::Execute()
{
  int r;
  //sleep(1);
  for(;;)
  {
    r=rc.getResource(num);
    timeval tv;
    tv.tv_sec=0;
    tv.tv_usec=10;
    //select(0,0,0,0,&tv);
    thr_yield();
    rc.freeResource(r,num);
    count++;
  }
  return 0;
}

int main()
{
  ThreadPool tp;
  const int n=25;
  TestTask *t[n];
  int i;
  tp.preCreateThreads(n);
  sleep(1);
  for(i=0;i<n;i++)
  {
    t[i]=new TestTask(i);
    tp.startTask(t[i]);
  }
  for(;;)
  {
    sleep(1);
    int sum=0;
    for(i=0;i<n;i++)
    {
      printf("%d ",t[i]->getCount());
      sum+=t[i]->getCount();
    }
    printf(":%d\n",sum);
  }
}
