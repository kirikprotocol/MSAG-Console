#ifndef __UTIL_TIMER_HPP__
#define __UTIL_TIMER_HPP__

#include <stdio.h>
#include <sys/time.h>

class Timer{
public:
  void Start()
  {
    gettimeofday(&_start,0);
  }
  void Finish()
  {
    gettimeofday(&_end,0);
  }
  long long Get()
  {
    long long rv=_end.tv_sec-_start.tv_sec;
    long long usec=_end.tv_usec-_start.tv_usec;
    return rv*1000+usec/1000;
  }
protected:
  timeval _start;
  timeval _end;
};

class TimeThis{
public:
  TimeThis(const char* argMsg,int argCount):msg(argMsg),count(argCount)
  {
    t.Start();
  }
  operator bool(){return false;}
  ~TimeThis()
  {
    t.Finish();
    long long ms=t.Get();
    printf("%s:count=%d,time=%lldms,speed=%.2f/sec\n",msg,count,ms,(double)(count*1000.0/ms));
  }
protected:
  const char* msg;
  int count;
  Timer t;
};

#define TIMETHIS(msg,n) if(TimeThis tt=TimeThis(msg,n));else for(int i=0;i<n;i++)
#endif
