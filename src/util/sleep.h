#ifndef __SMSC_UTIL_SLEEP_H__
#define __SMSC_UTIL_SLEEP_H__

#include <time.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace smsc{
namespace util{

inline void millisleep(unsigned msec)
{
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
