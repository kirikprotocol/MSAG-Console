#ifndef __SMSC_UTIL_TIMESLOTCOUNTER_HPP__
#define __SMSC_UTIL_TIMESLOTCOUNTER_HPP__

#include <stdlib.h>
#include <algorithm>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <windows.h>
typedef __int64 hrtime_t;
inline hrtime_t gethrfreq()
{
  hrtime_t freq;
  QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
  return freq;
}
inline hrtime_t gethrtime()
{
  static hrtime_t freq=gethrfreq();
  hrtime_t rv;
  QueryPerformanceCounter((LARGE_INTEGER*)&rv);
  rv=rv*1000000000ll/freq;
  return rv;
}
#endif

#ifdef linux
typedef long long hrtime_t;

inline hrtime_t gethrtime()
{
  timespec ts={0,0};
  clock_gettime(CLOCK_REALTIME,&ts);
  return hrtime_t(ts.tv_sec)*1000000000+ts.tv_nsec;
}
#endif

namespace smsc{
namespace util{

template <class T=int,T init_v=0,T inc_v=1>
class TimeSlotCounter{
public:
  TimeSlotCounter(int nsec,int msecres=100)
  {
    slotSize=nsec*1000/msecres;
    slotRes=msecres;
    slot=new T[slotSize];
    lastTime=0;
    first=0;
    last=0;
    slot[0]=init_v;
    count=init_v;
  }
  ~TimeSlotCounter()
  {
    delete [] slot;
  }

  void Inc(T inc=inc_v)
  {
    hrtime_t now=gethrtime();
    unsigned int diff=(now-lastTime)/1000000/slotRes;
    if(diff==0)
    {
      slot[last]+=inc;
      count+=inc;
      return;
    }
    lastTime=now;
    if(diff>slotSize)
    {
      first=0;
      last=0;
      slot[0]=inc;
      count=inc;
    }else
    {
      while(diff--)
      {
        last++;
        if(last>=slotSize)last=0;
        if(first==last)
        {
          count-=slot[first];
          first++;
          if(first>=slotSize)first=0;
        }
        slot[last]=init_v;
      }
      count+=inc;
      slot[last]+=inc;
    }
  }

  void IncDistr(T inc,T maxperslot)
  {
    Inc(0);
    int l=last;
    count+=inc;
    while(inc>0)
    {
      using namespace std;
      T v=min(maxperslot-slot[l],inc);
      if(v>0)
      {
        slot[l]+=v;
        inc-=v;
      }
      l--;
      if(l<0)l=slotSize-1;
      if(l==first)break;
    }
    //count-=inc;
    slot[last]+=inc;
  }

  void IncEven(T inc)
  {
    int mps=inc/slotSize;
    if(mps==0)mps=1;
    IncDistr(inc,mps);
  }

  T Get()
  {
    Inc(0);
    return count;
  }

  T Avg(int timeDiff=1000)
  {
    Inc(0);
    int used=(last-first+slotSize)%slotSize;
    if(used==0)return 0;
    return count*timeDiff/(used*slotRes);
  }

protected:
  T *slot;
  T count;
  int first,last,slotSize;
  int slotRes; // slot resolution
  hrtime_t lastTime;

  TimeSlotCounter();
  TimeSlotCounter(const TimeSlotCounter&);
};

}//util
}//smsc

#endif
