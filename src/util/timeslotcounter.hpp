#ifndef __SMSC_UTIL_TIMESLOTCOUNTER_HPP__
#define __SMSC_UTIL_TIMESLOTCOUNTER_HPP__

#include <stdlib.h>
#include <sys/time.h>
#include <algorithm>

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
    int diff=(now-lastTime)/1000000/slotRes;
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

  T Get()
  {
    Inc(0);
    return count;
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
