#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "core/buffers/Array.hpp"
#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"

#include <sys/devpoll.h>

using namespace smsc::core::buffers;
using namespace smsc::core::threads;
using namespace smsc::core::network;
using namespace smsc::core::synchronization;

class SynchCounter{
public:
  SynchCounter():count(0){}
  int Increment(int num=1)
  {
    int retval;
    lock.Lock();
    retval=(count+=num);
    lock.Unlock();
    return retval;
  }
  int Decrement(int num=1)
  {
    int retval;
    lock.Lock();
    retval=(count-=num);
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
  long long count;
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

pollfd PollFD(int fd,int events)
{
  pollfd pfd;
  pfd.fd=fd;
  pfd.events=events;
  return pfd;
}

int main(int argc,char* argv[])
{
  Socket s,*cs;
  MonitorThread m;
  int i;
  m.Start();
  if(s.InitServer("smsc",9876,0)==-1)return -1;
  if(s.StartServer()==-1)return -2;

  Array<Socket*> clnts;
  Array<pollfd> p;
  p.Push(PollFD(s.getSocket(),POLLIN));

  int wfd=open("/dev/poll",O_RDWR);
  if(wfd<0)
  {
    printf("failed to open /dev/poll\n");
    return -1;
  }
  if(write(wfd,&p[0],sizeof(pollfd))!=sizeof(pollfd))
  {
    printf("write failedn\n");
    close(wfd);
    return -1;
  };

  char buf[512];
  char tmp[512];

  dvpoll dp;
  for(;;)
  {
    dp.dp_timeout=-1;
    dp.dp_nfds=p.Count();
    dp.dp_fds=&p[0];

    int res=ioctl(wfd,DP_POLL,&dp);
    if(res<0)
    {
      printf("ioctl failed\n");
      break;
    }
    int j=0;
    for(i=0;i<res;i++)
    {
      if(dp.dp_fds[i].fd==s.getSocket())
      {
        cs=s.Accept();
        if(cs)
        {
          clnts.Push(cs);
          p.Push(PollFD(clnts[-1]->getSocket(),POLLIN));
          write(wfd,&p[-1],sizeof(pollfd));
          printf("Connected:%d\n",clnts.Count());
        }
      }else
      {
        for(;j<clnts.Count();j++)
        {
          if(clnts[j]->getSocket()==dp.dp_fds[i].fd)
          {
            int size=0,rd;
            do{
              rd=clnts[j]->Read(tmp,512-size);
              memcpy(buf+size,tmp,rd);
              size+=rd;
            }while(size<512);
            counter.Increment();
            break;
          }
        }
      }
    }
  }

  close(wfd);
  return 0;
}

#if 0

if ((wfd = open("/dev/poll", O_RDWR)) < 0) {
        exit(-1);
}
pollfd = (struct pollfd* )malloc(sizeof(struct pollfd) * MAXBUF);
if (pollfd == NULL) {
        close(wfd);
        exit(-1);
}
/*
 * initialize buffer
 */
for (i = 0; i < MAXBUF; i++) {
        pollfd[i].fd = fds[i];
        pollfd[i].events = POLLIN;
        pollfd[i].revents = 0;
}
if (write(wfd, &pollfd[0], sizeof(struct pollfd) * MAXBUF) !=
                sizeof(struct pollfd) * MAXBUF) {
        perror("failed to write all pollfds");
        close (wfd);
        free(pollfd);
        exit(-1);
}
/*
 * read from the devpoll driver
 */
dopoll.dp_timeout = -1;
dopoll.dp_nfds = MAXBUF;
dopoll.dp_fds = pollfd;
result = ioctl(wfd, DP_POLL, &dopoll);
if (result < 0) {
        perror("/dev/poll ioctl DP_POLL failed");
        close (wfd);
        free(pollfd);
        exit(-1);
}
for (i = 0; i < result; i++) {
        read(dopoll.dp_fds[i].fd, rbuf, STRLEN);
}
#endif
