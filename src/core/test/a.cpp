#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"

smsc::core::synchronization::Event e;

void disp(int sig){}

class mythread1:public smsc::core::threads::Thread{
  int id;
public:
  int Execute()
  {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,16);
    e.Wait();
    return 0;
  }
  void kill(int sig)
  {
    thr_kill(thread,sig);
  }
};

class mythread2:public smsc::core::threads::Thread{
  int id;
public:
  int Execute()
  {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,16);
    if(thr_sigsetmask(SIG_SETMASK,&set,NULL)!=0)
    e.Wait();
    return 0;
  }
  void kill(int sig)
  {
    thr_kill(thread,sig);
  }
};


int main(int argc,char* argv[])
{
  sigset(16,disp);
  sigset(SIGINT,disp);

  mythread1 t1;
  t1.Start();
  mythread2 t2;
  t2.Start();
  sleep(10);

  t1.kill(16);
  t2.kill(16);

  t1.WaitFor();
  t2.WaitFor();
  printf("Finishing\n");
  return 0;
}
