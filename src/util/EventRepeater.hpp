#ifndef __SMSC_UTIL_EVENTREPEATER_HPP__
#define __SMSC_UTIL_EVENTREPEATER_HPP__

#include "util/TimeSource.h"
#include "util/sleep.h"

namespace smsc{
namespace util{

/*
  This function will call op functor specified number of times per second.
  If functor will return false, function will exit.
*/

template <class T>
inline void RepeatEvent(int timesPerSecond,T& op)
{
  TimeSourceSetup::HRTime::hrtime_type opStart;
  int64_t delay=1000000000/timesPerSecond;
  int64_t overDelay=0;
  for(;;)
  {
    opStart=TimeSourceSetup::HRTime::getHRTime();
    if(!op())
    {
      return;
    }
    TimeSourceSetup::HRTime::hrtime_type opTime=TimeSourceSetup::HRTime::getHRTime()-opStart;
    if(delay>opTime+overDelay)
    {
      int64_t toSleep=delay-opTime-overDelay;
      TimeSourceSetup::HRTime::hrtime_type delayStart=TimeSourceSetup::HRTime::getHRTime();
      millisleep(toSleep/1000000);
      TimeSourceSetup::HRTime::hrtime_type delayTime=TimeSourceSetup::HRTime::getHRTime()-delayStart;
      overDelay=delayTime-toSleep;
    }else
    {
      overDelay-=delay-opTime;
      if(overDelay < -delay)
      {
        overDelay=-delay;
      }
    }
  }
}

}
}

#endif
