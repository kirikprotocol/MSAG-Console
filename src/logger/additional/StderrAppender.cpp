#include "logger/additional/StderrAppender.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace smsc {
namespace logger {

StderrAppender::StderrAppender(const char * const name)
  :Appender(name)
{
}

void StderrAppender::log(const char logLevelName, const char * const category, const char * const message) throw()
{
  ///TODO smsc::core::synchronization::MutexGuard guard(mutex);
  //D dd-mm hh:mm:ss,sss TTT CatLast___:message
#ifdef _WIN32
  DWORD thrId=GetCurrentThreadId();
  time_t t=time(NULL);
  struct tm lcltm=*localtime(&t);
  SYSTEMTIME stm;
  GetLocalTime(&stm);
  int msec=stm.wMilliseconds;

#else
  ::timeval tp;
  ::gettimeofday(&tp, 0);
  ::tm lcltm;
  ::localtime_r(&tp.tv_sec, &lcltm);
  pthread_t thrId=::pthread_self();
  int msec=tp.tv_usec/1000;
#endif
  char timeStr[32];
  const size_t timeStrLength = ::strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m %H:%M:%S", &lcltm);
  timeStr[timeStrLength] = 0;
  fprintf(stderr, "%c %s,%3.3u %3.3u % 10.10s: %s\n", logLevelName, timeStr, msec, thrId, category, message);
  ///TODO fflush(stderr);
}

}
}
