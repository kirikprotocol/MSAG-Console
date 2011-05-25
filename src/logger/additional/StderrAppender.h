#ifndef SMSC_LOGGER_STDERR_APPENDER_HEADER
#define SMSC_LOGGER_STDERR_APPENDER_HEADER

#include "logger/Appender.h"
#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace logger {

class StderrAppender : public Appender
{
public:
  StderrAppender(const char * const name);

#ifdef NEWLOGGER
  virtual size_t logPrefix(char* buf, size_t bufsize, timeval tv, const char logLevelName, const char* category) throw();
  virtual void write(timeval tv,char* buf, size_t bufsize) throw();
#else
  virtual void log(timeval tv,const char logLevelName, const char * const category, const char * const message) throw();
#endif

private:
  smsc::core::synchronization::Mutex mutex;
};

}
}
#endif //SMSC_LOGGER_STDERR_APPENDER_HEADER

