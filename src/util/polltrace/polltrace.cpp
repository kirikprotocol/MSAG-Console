#include <stdio.h>
//#include "debug.h"
#include <string>
#include <dlfcn.h>
#include <unistd.h>
#include <poll.h>
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/XHash.hpp"
#include "core/threads/Thread.hpp"

using namespace smsc::util;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::threads;

struct PollEnterInfo{
  time_t t;
  void *ra;
};

XHash<int,PollEnterInfo> stats;
Mutex mtx;
FILE *f;
int cnt=0;
int stopping=0;

class DumpThread:public Thread{
public:
  int Execute()
  {
    while(!stopping)
    {
      sleep(60);
      mtx.Lock();
      time_t now=time(NULL);;
      fprintf(f,"Dump started at %d\n",now);
      stats.First();
      int key;
      PollEnterInfo *pi;
      while(stats.Next(key,pi))
      {
        fprintf(f,"RA=%p, StartTime=%d(%d)\n",pi->ra,pi->t,now-pi->t);
      }
      fprintf(f,"Dump finished at %d(%d)\n\n",time(NULL));
      fflush(f);
      mtx.Unlock();
    }
    return 0;
  }
}dumpthr;


typedef int (*pollFn)(struct pollfd fds[], nfds_t nfds, int timeout);

pollFn oldpoll;

extern "C"
int _poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
  mtx.Lock();
  int id=cnt++;
  PollEnterInfo pi;
  pi.t=time(NULL);
  pi.ra=__builtin_return_address(0);
  stats[id]=pi;
  mtx.Unlock();
  int rv=oldpoll(fds,nfds,timeout);
  mtx.Lock();
  stats.Delete(id);
  mtx.Unlock();
  return rv;
}

struct Dummy{
Dummy()
{
  void* dlhandle = dlopen("/usr/lib/libc.so", RTLD_LAZY);
  if(!dlhandle)
  {
    fprintf(stderr,"dlopen failed\n");
    return;
  }
  oldpoll = (pollFn)dlsym(dlhandle, "_poll");
  if(!oldpoll)
  {
    fprintf(stderr,"dlsym failed\n");
    return;
  }
  f=fopen("polltrace.log","wt");
  dumpthr.Start();
}
~Dummy()
{
  fclose(f);
  stopping=1;
  dumpthr.WaitFor();
}
}dummy;
