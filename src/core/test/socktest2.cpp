#include <stdio.h>
#include <string.h>

#include "core/buffers/Array.hpp"
#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"

using namespace smsc::core::buffers;
using namespace smsc::core::threads;
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

class MonitorThread:public Thread{
  int lastcount;
public:
  MonitorThread():lastcount(0){}

  virtual int Execute();
};


int MonitorThread::Execute()
{
  printf("Monitor started\n");
  lastcount=counter.Get();
  int newcount;
  for(;;)
  {
    sleep(1);
    newcount=counter.Get();
    printf("%d(%d-%d)\n",newcount-lastcount,newcount,lastcount);
    fflush(stdout);
    lastcount=newcount;
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


  Array<pollfd> fds;
  Array<Socket*> clnt;
  pollfd fd;
  int i;
  fds[0].fd=s.getSocket();
  fds[0].events=POLLRDNORM;

  char buf[512];
  char tmp[512];

  for(;;)
  {
    int res=poll(&fds[0],fds.Count(),-1);
    if(res==-1)break;
    for(i=1;i<fds.Count();i++)
    {
      if(fds[i].revents&POLLRDNORM)
      {
        int size=0,rd;
        do{
          rd=clnt[i-1]->Read(tmp,512-size);
          memcpy(buf+size,tmp,rd);
          size+=rd;
        }while(size<512);
        counter.Increment();
      }
    }
    if(fds[0].revents&POLLRDNORM)
    {
      cs=s.Accept();
      clnt.Push(cs);
      fd.fd=cs->getSocket();
      fd.events=POLLRDNORM;
      fds.Push(fd);
    }
  }
  return 0;
}
