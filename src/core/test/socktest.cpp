#include <stdio.h>
#include <string.h>

#include "core/buffers/Array.hpp"
#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"

using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::core::network;
using namespace smsc::core::synchronization;

class SynchCounter{
public:
  SynchCounter():count(0){}
  int Increment()
  {
    int retval;
    lock.Lock();
    retval=++count;
    lock.Unlock();
    return retval;
  }
  int Decrement()
  {
    int retval;
    lock.Lock();
    retval=--count;
    lock.Unlock();
    return retval;
  }
  int Get()
  {
    int retval;
    lock.Lock();
    retval=count;
    lock.Unlock();
    return retval;
  }
protected:
  Mutex lock;
  int count;
};

SynchCounter counter;


class TaskThread:public Thread{
  Socket *cs;
  int num;
public:
  TaskThread(Socket* newsocket,int n):cs(newsocket),num(n){}
  ~TaskThread(){delete cs;}
  virtual int Execute();
};

class MonitorThread:public Thread{
  int lastcount;
public:
  MonitorThread():lastcount(0){}

  virtual int Execute();
};


Mutex cntlock;
Array<int> cnt;

int MonitorThread::Execute()
{
  printf("Monitor started\n");
  lastcount=counter.Get();
  int newcount;
  int n=0;
  for(;;)
  {
    sleep(1);
    newcount=counter.Get();
    printf("%d(%d-%d)\n",newcount-lastcount,newcount,lastcount);
    fflush(stdout);
    lastcount=newcount;
    n++;
    if(n%10==0)
    {
      cntlock.Lock();
      int m=cnt.Count();
      int *x=new int[m];
      int i;
      memcpy(x,&cnt[0],m*sizeof(int));
      cntlock.Unlock();
      for(i=0;i<m;i++)
      {
        printf("%d ",x[i]);
      }
      printf("\n");
      delete [] x;
    }
  }
  return 0;
}


int TaskThread::Execute()
{
  char buf[512];
  char buf2[512];
  int size=0,rd;
  printf("Thread %d started\n",num);
  for(;;)
  {
    size=0;
    memset(buf,0,sizeof(buf));
    do{
      rd=cs->Read(buf2,512-size);
      if(rd==-1)return -1;
      memcpy(buf+size,buf2,rd);
      //printf("READ:%d\n",size);
      size+=rd;
    }while(size<512);
    for(int i=0;i<512;i++)
    {
      if((unsigned char)buf[i]!=(unsigned char)i)
      {
        printf("FUCK:%d!\n",i);
        return -1;
      }
    }
    cs->Write(buf,512);
    counter.Increment();
    cntlock.Lock();
    cnt[num]++;
    cntlock.Unlock();
  }
  return 0;
}

int main(int argc,char* argv[])
{
  Socket s,*cs;
  MonitorThread m;
  m.Start();
  if(s.InitServer("smsc",9876,0)==-1)return -1;
  if(s.StartServer()==-1)return -2;
  smsc::core::buffers::Array<TaskThread*> threads;
  for(;;)
  {
    cs=s.Accept();
    if(cs)
    {
      int n=threads.Count();
      cntlock.Lock();
      cnt[n]=0;
      cntlock.Unlock();
      printf("Connect:%d\n",n);
      threads.Push(new TaskThread(cs,n));
      threads[-1]->Start();
    }
  }
  return 0;
}
