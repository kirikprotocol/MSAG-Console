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
  TimeThis(const char* argMsg):msg(argMsg)
  {
    t.Start();
  }
  operator bool(){return false;}
  ~TimeThis()
  {
    t.Finish();
    printf("%s:%lldms\n",msg,t.Get());
  }
protected:
  const char* msg;
  Timer t;
};

#define TIMETHIS(msg,n) if(TimeThis tt=TimeThis(msg));else for(int i=0;i<n;i++)
#endif
