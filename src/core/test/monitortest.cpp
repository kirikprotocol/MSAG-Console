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

struct Queue{
  thread_t id;
  Queue *next;
};

class ResourceClass{
  Array<int> used,free;
  EventMonitor m;
  Queue *head,*tail;
public:
  ResourceClass()
  {
    for(int i=0;i<3;i++)free.Push(i);
    head=tail=NULL;
  }
  int getResource(Queue* q,int id)
  {
    MutexGuard guard(m);
    //if(id==3)printf("get res:%d(%s)\n",id,head?"queue is not empty":"queue is empty");
    if(head || free.Count()==0)
    {
      if(tail)tail->next=q;
      tail=q;
      q->next=NULL;
      q->id=thr_self();
      if(head==NULL)head=tail;
      //if(id==3)printf("thread %d: waiting for resource\n",id);
      do{
        m.wait();
        head=head->next;
        if(!head)tail=NULL;
        //if(id==3)printf("awaiking:%d(%d)\n",id,free.Count());
      }while(free.Count()==0);
    }
    //if(id==3)printf("thread %d: got resource\n",id);
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
    if(head)
    {
      thr_kill(head->id,16);
    }
    //if(thr_kill(q->id,16))printf("fuck:%s\n",strerror(errno));
    //else printf("signal: ok\n");
    //m.notify();
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
    Queue q;
    r=rc.getResource(&q,num);
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

Mutex mmm;

void disp(int param)
{
  //printf("got a signal:%d\n",param);
//  signal(16,disp);
}


int main()
{
  sigset_t set,oldset;
  sigemptyset(&set);
  sigaddset(&set,16);
  //sigprocmask(SIG_SETMASK,&set,&oldset);
  //thr_sigsetmask(SIG_SETMASK,&set,&oldset);
  //signal(16,disp);
  sigset(16,disp);

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
