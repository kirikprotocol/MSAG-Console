#ifndef SMSC_LOGGER_ADDITIONAL_ROLLINGINTERVALAPPENDER
#define SMSC_LOGGER_ADDITIONAL_ROLLINGINTERVALAPPENDER

#include <stdio.h>

#include <logger/Appender.h>
#include <core/synchronization/Mutex.hpp>
#include <util/Properties.h>

#include <time.h>
#include <sys/time.h>

#include "core/buffers/File.hpp"

namespace smsc {
namespace logger {

using namespace smsc::util;

class RollingIntervalAppender : public Appender
{
public:
  RollingIntervalAppender(const char * const name, const Properties & properties, const char* suffix = NULL);
#ifdef NEWLOGGER
  virtual size_t logPrefix(char* buf, size_t bufsize, timeval tv, const char logLevelName, const char* category) throw();
  virtual void write(timeval tv,char* buf, size_t bufsize) throw();
#else  
  virtual void log(timeval tv,const char logLevelName, const char * const category, const char * const message) throw();
#endif
private:
  void clearLogDir(time_t dat);
  smsc::core::synchronization::Mutex mutex;
  unsigned int maxBackupIndex, numFieldsInSuffix;
  long interval;
  std::string fullFileName, suffixFormat, path, fileName, postfix_;
  smsc::core::buffers::File file;
  long currentFilePos;
  time_t lastIntervalStart;

  void rollover(time_t dat, bool useLast = false) throw();
  time_t roundTime(time_t dat, struct ::tm* rtm = NULL);
  bool findLastFile(time_t dat, size_t suffixSize, std::string& lastFileName);
};

}}

#endif //SMSC_LOGGER_ADDITIONAL_ROLLINGINTERVALAPPENDER
