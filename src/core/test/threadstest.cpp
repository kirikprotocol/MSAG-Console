#include "core/threads/ThreadPool.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


using namespace smsc::core::threads;

ThreadPool tp;

class MyTask:public ThreadedTask{
  int tasknum;
  unsigned int seed;
public:
  MyTask(int num)
  {
    tasknum=num;
    seed=thr_self();
  }
  virtual int Execute();
  virtual const char* taskName(){return "testtask";}
  int rnd(int max);
};

int MyTask::rnd(int max)
{
  double x=rand_r(&seed);
  x*=max;
  x/=RAND_MAX;
  return (int)x;
}

int MyTask::Execute()
{
  char *a[100];
  char *b[100];
  int i;
  trace2("Started:%d\n",tasknum);
  for(i=0;i<100;i++)
  {
    getMem(16+rnd(900),a[i]);
  }
  setCheckPoint();
  for(i=0;i<100;i++)
  {
    getRawMem(16+rnd(100),b[i]);
  }
  doRollBack();
  for(i=0;i<100;i++)
  {
    freeMem(a[i]);
  }
  trace2("Finished:%d\n",tasknum);
}

int main(int argc,char* argv[])
{
  trace2("PreCreating threads\n");
  tp.preCreateThreads(50);
  sleep(2);
  trace2("PreCreating done\n");
  for(int i=0;i<1;i++)
  {
    trace2("Starting task:%d\n",i);
    tp.startTask(new MyTask(i));
    trace2("Task %d started\n",i);
  }
  sleep(1);
  return 0;
}
