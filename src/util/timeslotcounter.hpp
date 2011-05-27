#ifndef __SMSC_UTIL_TIMESLOTCOUNTER_HPP__
#define __SMSC_UTIL_TIMESLOTCOUNTER_HPP__

#include <stdlib.h>
//#include <vector>
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
#ifdef __MACH__
typedef long long hrtime_t;

inline hrtime_t gethrtime()
{
    timeval tv = {0,0};
    gettimeofday(&tv,0);
    return hrtime_t(tv.tv_sec)*1000000000+tv.tv_usec*1000;
}
#endif

namespace smsc{
namespace util{

template <class T=int,T init_v=0,T inc_v=1>
class TimeSlotCounter{
public:
  TimeSlotCounter(unsigned int nsec,unsigned int msecres=100)
  {
    slotsCount=(int)(nsec*1000UL/msecres);
    slotRes=msecres;
    slot=new T[slotsCount];
    lastTime=gethrtime();
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
    hrtime_t hrdif=(now-lastTime)/1000000;
    unsigned int diff=(unsigned int)((hrdif+slotRes/2)/slotRes);
    //diff=(diff+500000)/1000000;
    if(diff==0)
    {
      slot[last]+=inc;
      count+=inc;
      return;
    }
    lastTime=lastTime+1000000ll*diff*slotRes;
    if(diff>slotsCount)
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
        if(last>=slotsCount)last=0;
        if(first==last)
        {
          count-=slot[first];
          first++;
          if(first>=slotsCount)first=0;
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
    if(count>maxperslot*slotsCount)
    {
      //inc-=count-maxperslot*slotsCount-1;
      //count=maxperslot*slotsCount+1;
      maxperslot+=(count-maxperslot*slotsCount)/slotsCount;
    }
    if(first!=last)
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
      if(l<0)l=slotsCount-1;
      if(l==first)break;
    }
    //count-=inc;
    slot[last]+=inc;
  }

  void IncEven(T inc)
  {
    int mps=inc/slotsCount;
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
    int used=(last-first+slotsCount)%slotsCount;
    if(used==0)return 0;
    return count*timeDiff/(used*slotRes);
  }

  int getSlotRes()
  {
    return slotRes;
  }
  
  /*void dump(std::vector<T>& v)
  {
    v.insert(v.begin(),slot,slot+slotsCount);
  }*/

protected:
  T *slot;
  T count;
  unsigned int first,last,slotsCount;
  int slotRes; // slot resolution
  hrtime_t lastTime;

  TimeSlotCounter();
  TimeSlotCounter(const TimeSlotCounter&);
};

}//util
}//smsc

#endif
