#include "logger/additional/RollingIntervalAppender.h"

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

#include "util/debug.h"

namespace smsc {
namespace logger {

using namespace smsc::util;
using namespace smsc::core::buffers;

time_t RollingIntervalAppender::roundTime(time_t dat, struct ::tm* rtm)
{
  struct ::tm t={0,};
#ifdef _WIN32
  t=*localtime(&dat);
#else
  localtime_r(&dat, &t);
#endif
  if(!(interval % 60)) t.tm_sec = 0;
  if(!(interval % 3600)) t.tm_min = 0;
  if(!(interval % 86400)) t.tm_hour = 0;
  t.tm_isdst = -1;
  if(rtm)
  {
    *rtm = t;
    rtm->tm_year += 1900;
    rtm->tm_mon++;
  }
  return mktime(&t);
}

bool RollingIntervalAppender::findLastFile(time_t dat, size_t suffixSize, std::string& lastFileName)
{
  dirent *dp=0;
  DIR *dirp = opendir(path.c_str());
  if(!dirp) return false;
  std::string fname_ = fileName + suffixFormat;
  size_t lastFileNameSize = fileName.size() + suffixSize;
  // making curInterval a very big number to prevent losing a previous file after restart
  long curInterval = 86400*365*10; // 10 years
  lastFileName.clear();
  bool result = false;
  tm rtm;
  while((dp = readdir(dirp)))
  {
    if (strlen(dp->d_name) != lastFileNameSize) {
      continue;
    }
    memset(&rtm, 0, sizeof(rtm));
    if(sscanf(dp->d_name, fname_.c_str(), &rtm.tm_year, &rtm.tm_mon, &rtm.tm_mday, &rtm.tm_hour, &rtm.tm_min, &rtm.tm_sec) == numFieldsInSuffix)
    {
      rtm.tm_year -= 1900;
      rtm.tm_mon--;
      rtm.tm_isdst = -1;
      time_t fdate = mktime(&rtm);

      if(dat - fdate < curInterval)
      {
        lastFileName = dp->d_name;
        lastIntervalStart = fdate;
        curInterval = dat - fdate;
        result = true;
      }
      else
      {
        result = false;
      }
    }
  }
  closedir(dirp);
  return result;
}

void RollingIntervalAppender::clearLogDir(time_t curTime)
{
  dirent *dp;
  DIR *dirp = opendir(path.c_str());
  if(!dirp) return;
  std::vector<std::string> fnames;
  struct tm rtm;
  memset(&rtm, 0, sizeof(rtm));
  rtm.tm_isdst = -1;

  std::string fname_ = fileName + suffixFormat;
  while((dp = readdir(dirp)))
  {
    if(sscanf(dp->d_name, fname_.c_str(), &rtm.tm_year, &rtm.tm_mon, &rtm.tm_mday, &rtm.tm_hour, &rtm.tm_min, &rtm.tm_sec) == numFieldsInSuffix)
    {
        rtm.tm_year -= 1900;
        rtm.tm_mon--;
        if((curTime - mktime(&rtm)) / interval >= maxBackupIndex)
            fnames.push_back(path + '/' + dp->d_name);
    }
  }

  for(std::vector<std::string>::iterator it = fnames.begin(); it != fnames.end(); it++)
    remove((*it).c_str());

  closedir(dirp);
}

RollingIntervalAppender::RollingIntervalAppender(const char * const _name, const Properties & properties, const char* suffix)
  :Appender(_name), mutex(MTXFORCENOCHECK), maxBackupIndex(0), interval(86400), path("."), lastIntervalStart(0)
{
  if (properties.Exists("maxindex"))
    maxBackupIndex = atoi(properties["maxindex"]);

  if (properties.Exists("interval"))
  {
    uint32_t hour, min, sec;
    if(sscanf(properties["interval"], "%u:%u:%u", &hour, &min, &sec) == 3)
        interval = hour * 3600 + min * 60 + sec;
  }

  suffixFormat = properties.Exists("suffixFormat") ? properties["suffixFormat"] : suffix ? suffix : ".%04d%02d%02d%02d%02d%02d";
  if(suffixFormat[0] != '.') suffixFormat = '.' + suffixFormat;
  const char* p = suffixFormat.c_str();
  bool is_esc = false;
  numFieldsInSuffix = 0;
  while(*p)
  {
    if(*p == '%' && p[1] && p[1] != '%' && !is_esc) numFieldsInSuffix++;
    is_esc = *p++ == '%' && !is_esc;
  }

  fullFileName = properties.Exists("name") ? properties["name"] : "smsc.log";
  postfix_ = properties.Exists("postfix") ? properties["postfix"] : ".log";

  const char *c = fullFileName.c_str();
  p = strrchr(c, '/');
  if(p) path.assign(c, p - c);
  fileName = p ? p + 1 : c;

  rollover(time(NULL), true);
}

void RollingIntervalAppender::rollover(time_t dat, bool useLast) throw()
{
  try{
    std::string currentName = file.getFileName();
    file.Close();

    struct ::tm rtm;
    memset(&rtm, 0, sizeof(rtm));
    lastIntervalStart = roundTime(dat, &rtm);

    char name_[128];
    memset(name_, 0, 128);
    int suffixSize = sprintf(name_, suffixFormat.c_str(), rtm.tm_year, rtm.tm_mon, rtm.tm_mday, rtm.tm_hour, rtm.tm_min, rtm.tm_sec);

    if(maxBackupIndex > 0)
      clearLogDir(lastIntervalStart);

    if(useLast)
    {
      std::string lastFile;
      if(findLastFile(dat, suffixSize, lastFile))
      {
        file.Append((path+'/'+lastFile).c_str());
      } else {
        lastIntervalStart = 0;
      }
      return;
    }

    if (!currentName.empty() && File::Exists(currentName.c_str())) {
      File::Rename(currentName.c_str(), (currentName + postfix_).c_str());
    }
    std::string p = fullFileName + name_;
    file.WOpen(p.c_str());
  }catch(std::exception& e)
  {
    fprintf(stderr,"RollingIntervalAppender::rollover exception: %s\n",e.what());
  }
}

#ifdef NEWLOGGER
unsigned RollingIntervalAppender::getPrefixLength() throw()
{
    static unsigned pl = 0;
    if (!pl) {
        char buf[200];
        timeval tp;
        gettimeofday(&tp,0);
        pl = logPrefix(buf,tp,'I',"hello");
    }
    return pl;
}

unsigned RollingIntervalAppender::logPrefix(char* buf, timeval tp, const char logLevelName, const char* category) throw()
#else
void RollingIntervalAppender::log(timeval tp,const char logLevelName, const char * const category, const char * const message) throw()
#endif
{
  //D dd-mm hh:mm:ss,sss TTT CatLast___:message
  ::tm ltm;
  time_t dat = tp.tv_sec;
  ::localtime_r(&tp.tv_sec, &ltm);
  pthread_t thrId=::pthread_self();
  long msec=tp.tv_usec/1000;
#ifdef NEWLOGGER
  int res = sprintf(buf,"%c %02d-%02d-%04d %02d:%02d:%02d,%03d %03u %10.10s:",
                    logLevelName, ltm.tm_mday, ltm.tm_mon+1, ltm.tm_year+1900,
                    ltm.tm_hour, ltm.tm_min, ltm.tm_sec,
                    int(msec), unsigned(thrId), category);
  return unsigned(res);
}

void RollingIntervalAppender::write(timeval tp, const char logLevelName, const char* category, char* buffer, size_t length) throw()
{
  time_t dat = tp.tv_sec;
  buffer[logPrefix(buffer,tp,logLevelName,category)] = ' ';
  buffer[length++] = '\n';
  const size_t desiredLength = length;
#endif
  smsc::core::synchronization::MutexGuard guard(mutex);

  if((dat - lastIntervalStart) / interval > 0)
      rollover(dat);

#ifndef NEWLOGGER
  char timeStr[32];
  const size_t timeStrLength = ::strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m-%Y %H:%M:%S", &ltm);
  timeStr[timeStrLength] = 0;
  const size_t desiredLength = strlen(message)+128;
  TmpBuf<char, 4096> buffer(desiredLength+1);
  const size_t length = snprintf(buffer, desiredLength, "%c %s,%3.3u %3.3u %10.10s: %s\n", logLevelName, timeStr, unsigned(msec), unsigned(thrId), category, message);
  buffer[desiredLength] = 0;
#endif

  try {
      if(file.isOpened())
      {
          file.Write(buffer, length < desiredLength ? length : desiredLength);
          file.Flush();
      }else {
          fwrite(buffer, length < desiredLength ? length : desiredLength, 1, stderr);
      }
  } catch (std::exception& e) {
      fprintf(stderr,"EXCEPTION IN LOGGER:%s\n",e.what());
  }
}

}}
