#include "logger/additional/RollingDayAppender.h"

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

#include "util/cstrings.h"
#include "core/buffers/TmpBuf.hpp"
#include <dirent.h>
#include <errno.h>
#include <string>
#include <vector>

namespace smsc {
namespace logger {

using namespace smsc::util;
using namespace smsc::core::buffers;

void RollingDayAppender::clearLogDir(time_t dat)
{

  DIR *dirp;
  dirent *dp;
  dirp = opendir(pathname.c_str());

  std::vector<std::string> fnames(0);

  while (dirp) {
      errno = 0;
      if ((dp = readdir(dirp)) != NULL) {
          int year = 0, month = 0, day = 0, res = 0;
          std::string fname = prefix + "%02d_%02d_%04d.log";
            if((res = sscanf(fname.c_str(), dp->d_name, &day, &month, &year)) == 3){

                    // Saves old files
                    time_t fdate;
				    tm rtm;
				    rtm.tm_year = year - 1900;
				    rtm.tm_mon = month - 1;
				    rtm.tm_mday = day;
				    rtm.tm_hour = 0;
				    rtm.tm_min = 0;
				    rtm.tm_sec = 0;
				    rtm.tm_isdst = -1;
				    fdate = mktime(&rtm);
				    
				    int delay = (int)( difftime(date_, fdate)/(3600.*24.) );
				    //printf("delay: %d\n", delay);
				    if(delay >= maxBackupIndex)
					    fnames.push_back(pathname + std::string("/") + std::string(dp->d_name));
			    
		    }
      } else {
          if (errno == 0) {
              closedir(dirp);
			  break;             // NOT_FOUND;
		  }
          closedir(dirp);
		  break;
	  }
  }

  // Removes old files
  std::vector<std::string>::iterator it = fnames.begin();
  for(it = fnames.begin(); it != fnames.end(); it++)
	remove((*it).c_str());
}


RollingDayAppender::RollingDayAppender(const char * const _name, const Properties & properties)
  :Appender(_name)
{

  maxBackupIndex = 0;

  if (properties.Exists("maxindex"))
    maxBackupIndex = atoi(properties["maxindex"]);

  if (properties.Exists("dir")){
    std::auto_ptr<char> path( cStringCopy(properties["dir"]) );
    if(strlen(path.get()) == 0)
        pathname = ".";
    else{
        if(path.get()[strlen(path.get()) - 1] == '/')
            path.get()[strlen(path.get()) - 1] = 0;
        pathname = path.get();
    }
  }else
    pathname = ".";

  if (properties.Exists("prefix")){
    prefix = cStringCopy(properties["prefix"]);
  }else
    prefix = "";

  date_ = time(0);
  tm rtm; localtime_r(&date_, &rtm);
  int year = rtm.tm_year + 1900;
  int month = rtm.tm_mon + 1;
  int day = rtm.tm_mday;
  rtm.tm_hour = 0;
  rtm.tm_min = 0;
  rtm.tm_sec = 0;
  rtm.tm_isdst = -1;
  date_ = mktime(&rtm);

  // Clear log dir
  clearLogDir(date_);
  char name_[512];
  sprintf(name_, "%02d_%02d_%04d.log", day, month, year);
  std::string path = pathname + "/";
  path += prefix + name_;
  file = fopen(path.c_str(), "a");
}

void RollingDayAppender::rollover(time_t dat) throw()
{

  fclose(file);

  if(maxBackupIndex > 0)
    clearLogDir(dat);

  tm rtm; localtime_r(&dat, &rtm);
  int year = rtm.tm_year + 1900;
  int month = rtm.tm_mon + 1;
  int day = rtm.tm_mday;
  rtm.tm_hour = 0;
  rtm.tm_min = 0;
  rtm.tm_sec = 0;
  rtm.tm_isdst = -1;

  char name_[512];
  sprintf(name_, "%02d_%02d_%04d.log", day, month, year);
  std::string path = pathname + "/";
  path += prefix + name_;
  file = fopen(path.c_str(), "w");
  date_ = dat;

  currentFilePos = 0;
}

void RollingDayAppender::log(const char logLevelName, const char * const category, const char * const message) throw()
{
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
  ::tm ltm;
  ::localtime_r(&tp.tv_sec, &lcltm);
  ::localtime_r(&tp.tv_sec, &ltm);
  pthread_t thrId=::pthread_self();
  int msec=tp.tv_usec/1000;
#endif

  smsc::core::synchronization::MutexGuard guard(mutex);

  time_t dat;
  ltm.tm_hour = 0;  ltm.tm_min = 0;   ltm.tm_sec = 0; ltm.tm_isdst = -1;
  dat = mktime(&ltm);
  int delay = (int)( difftime(dat, date_)/(3600.*24.) );
  if(delay > 0)
      rollover(dat);

  char timeStr[32];
  const size_t timeStrLength = ::strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m-%Y %H:%M:%S", &lcltm);
  timeStr[timeStrLength] = 0;
  const size_t desiredLength = strlen(message)+128;
  TmpBuf<char, 4096> buffer(desiredLength+1);
  const size_t length = snprintf(buffer, desiredLength, "%c %s,%3.3u %3.3u % 10.10s: %s\n", logLevelName, timeStr, msec, thrId, category, message);
  buffer[desiredLength] = 0;

  fwrite(buffer, length < desiredLength ? length : desiredLength, 1, file != NULL ? file : stderr);
  fflush(file);
  currentFilePos += length;
}

}
}
