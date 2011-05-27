#ifndef __SMSC_UTIL_SLEEP_H__
#define __SMSC_UTIL_SLEEP_H__

#include <time.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "util/int.h"
#ifdef __MACH__
#include <sys/time.h>
#endif

namespace smsc{
namespace util{

inline uint64_t getmillis()
{
#if defined(_WIN32) || defined(linux)
  return 0;
#elif defined(linux)
  timespec ts;
  clock_gettime(CLOCK_REALTIME,&ts);
  return uint64_t(ts.tv_sec)*1000 + ts.tv_nsec/1000000;
#elif defined(__MACH__)
  timeval tv;
  gettimeofday(&tv,0);
  return uint64_t(tv.tv_sec)*1000 + tv.tv_usec/1000;
#else
  return gethrtime()/1000000l;
#endif
}

inline void millisleep(unsigned msec)
{
  if(msec>0x80000000)return;
#ifdef _WIN32
  Sleep(msec);
#else
  timespec ts,rm={0,0};
  ts.tv_sec=msec/1000;
  ts.tv_nsec=(msec%1000)*1000000;
  do{
    if(nanosleep(&ts,&rm)==0)break;
    if(errno==EINVAL)break;
    ts=rm;
  }while(rm.tv_sec!=0 || rm.tv_nsec!=0);
#endif
}

}//namespace util
}//namespace smsc

#endif
