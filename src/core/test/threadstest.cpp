#include "core/threads/ThreadPool.hpp"
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>

#ifdef _WIN32
void sleep(int secs)
{
  Sleep(secs*1000);
}
#endif


using namespace smsc::core::threads;

#ifdef USEPOOL
ThreadPool tp;
#endif

class MyTask:public ThreadedTask{
  int tasknum;
  unsigned int seed;
public:
  MyTask(int num)
  {
    tasknum=num;
#ifdef _WIN32
    seed=1;//GetCurrentThreadId();
#else
    seed=thr_self();
#endif
    trace2("creating task:%d",num);
  }
  virtual int Execute();
  virtual const char* taskName(){return "testtask";}
  int rnd(int max);
};

int MyTask::rnd(int max)
{
  seed=seed*0x1F28A483+0xE386D92F;
  return (seed>>3 & 0x7fffffff)%max;
}

int MyTask::Execute()
{
  const int ZZZ=100;
  char *a[ZZZ];
  char *b[ZZZ];
  int i;
  trace2("Started:%d\n",tasknum);
  for(i=0;i<ZZZ;i++)
  {
#ifdef USEPOOL
    getMem(16+rnd(900),a[i]);
#else
    a[i]=new char[16+rnd(900)];
#endif
  }
#ifdef USEPOOL
  setCheckPoint();
#endif
  for(i=0;i<ZZZ;i++)
  {
#ifdef USEPOOL
    getRawMem(16+rnd(100),b[i]);
#else
    b[i]=new char[16+rnd(100)];
#endif
  }
#ifdef USEPOOL
  doRollBack();
#endif
  for(i=0;i<ZZZ;i++)
  {
#ifdef USEPOOL
    freeMem(a[i]);
#else
    delete [] a[i];
    delete [] b[i];
#endif
  }
  trace2("Finished:%d\n",tasknum);
  return 0;
}


class MyThread:public Thread{
public:
  MyThread(int cnt):t(cnt)
  {
  }
  virtual int Execute()
  {
    return t.Execute();
  }
private:
  MyTask t;
};

/*
int main()
{
  MemoryManager m;
  MyTask t(0);
  int raw,blocks;
  t.getMemoryInfo(raw,blocks);
  t.assignHeap(m.acquireHeap(t.taskName(),raw,blocks));
  t.Execute();
  return 0;
}
*/

int main(int argc,char* argv[])
{
#ifndef USEPOOL
  MyThread *t[5000];
  int i;
  for(i=0;i<5000;i++)
  {
    t[i]=new MyThread(i);
    t[i]->Start();
  }
  for(i=0;i<5000;i++)
  {
    t[i]->WaitFor();
  }
#else
  trace2("PreCreating threads\n");
  tp.preCreateThreads(100);
  //sleep(2);
  trace2("PreCreating done\n");
  for(int i=0;i<2000;i++)
  {
    //trace2("Starting task:%d\n",i);
    tp.startTask(new MyTask(i));
    //trace2("Task %d started\n",i);
  }
#endif
  return 0;
}
