#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/buffers/Array.hpp"

#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

struct Queue{
  cond_t c;
  int res;
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
  int getResource(int id)
  {
    Queue q;
    MutexGuard guard(m);
    //if(id==3)printf("get res:%d(%s)\n",id,head?"queue is not empty":"queue is empty");
    if(head || free.Count()==0)
    {
      if(tail)tail->next=&q;
      tail=&q;
      q.next=NULL;
      cond_init(&q.c,0,0);
      if(head==NULL)head=tail;
      m.wait(&q.c);
      cond_destroy(&q.c);
      return q.res;
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
    if(head)
    {
      Queue *q=head;
      head=head->next;
      if(!head)tail=NULL;
      q->res=res;
      m.notify(&q->c);
      return;
    }
    for(int i=0;i<used.Count();i++)
    {
      if(used[i]==res)
      {
        free.Push(res);
        used.Delete(i);
        break;
      }
    }
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

static int ru[3];
static int busy[3];

int working=0;
Mutex wm;

int TestTask::Execute()
{
  int r;
  wm.Lock();
  working++;
  wm.Unlock();
  int *leak=new int;
  for(int i=0;i<1000;i++)
  {
    r=rc.getResource(num);
    if(busy[r])
    {
      printf("FUCK!!!!\n");
      exit(-1);
    }
    busy[r]=1;
    ru[r]++;
    timeval tv;
    tv.tv_sec=0;
    tv.tv_usec=200;
    //select(0,0,0,0,&tv);
    thr_yield();
    busy[r]=0;
    rc.freeResource(r,num);
    count++;
  }
  wm.Lock();
  working--;
  wm.Unlock();
  return 0;
}


int main()
{
  ThreadPool *tp=new ThreadPool;
  const int n=25;
  TestTask *t[n];
  int i;
  tp->preCreateThreads(n);
  sleep(1);
  for(i=0;i<n;i++)
  {
    t[i]=new TestTask(i);
    tp->startTask(t[i]);
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
    for(i=0;i<3;i++)printf("%d ",ru[i]);
    printf("\n");
    wm.Lock();
    if(working==0)
    {
      wm.Unlock();
      break;
    }
    wm.Unlock();
  }

}
