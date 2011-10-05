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
  :Appender(name), mutex(MTXFORCENOCHECK)
{
}

#ifdef NEWLOGGER
unsigned StderrAppender::getPrefixLength() throw()
{
    static unsigned pl = 0;
    if (!pl) {
        char buf[200];
        timeval tp;
        gettimeofday(&tp,0);
        pl = logPrefix(buf,tp,'I',"hello") + 1;
    }
    return pl;
}

unsigned StderrAppender::logPrefix(char* buf, timeval tp, const char logLevelName, const char* category) throw()
#else
void StderrAppender::log(timeval tp,const char logLevelName, const char * const category, const char * const message) throw()
#endif
{
  ///TODO smsc::core::synchronization::MutexGuard guard(mutex);
  //D dd-mm hh:mm:ss,sss TTT CatLast___:message
  ::tm lcltm;
  ::localtime_r(&tp.tv_sec, &lcltm);
  pthread_t thrId=::pthread_self();
  long msec=tp.tv_usec/1000;
#ifdef NEWLOGGER
  int res = sprintf(buf,"%c %02d-%02d %02d:%02d:%02d,%03d %03u %10.10s:",
                    logLevelName, lcltm.tm_mday, lcltm.tm_mon+1, lcltm.tm_hour,
                    lcltm.tm_min, lcltm.tm_sec, int(msec), unsigned(thrId), category);
  return unsigned(res);
}

void StderrAppender::write(timeval tp, const char logLevelName, const char* category, char* buf, size_t bufsize) throw()
{
    // printing prefix and replacing the trailing \0 with space.
    buf[logPrefix(buf,tp,logLevelName,category)] = ' ';
    buf[bufsize++] = '\n';
    fwrite(buf,bufsize,1,stderr);
}

#else
  char timeStr[32];
  const size_t timeStrLength = ::strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m %H:%M:%S", &lcltm);
  timeStr[timeStrLength] = 0;
#ifdef __APPLE__
  unsigned thrIdNum=(unsigned)pthread_mach_thread_np(thrId);
#else
  unsigned thrIdNum=(unsigned)thrId;
#endif
  fprintf(stderr, "%c %s,%03ld %03u %10.10s: %s\n", logLevelName, timeStr, msec, thrIdNum, category, message);
  ///TODO fflush(stderr);
}
#endif

}
}
