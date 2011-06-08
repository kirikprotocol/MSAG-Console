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
  virtual unsigned getPrefixLength() throw();
  virtual void write(timeval tv,const char logLevelName,const char* category,char* buf, size_t bufsize) throw();
private:
  unsigned logPrefix(char* buf, timeval tp, const char logLevelName, const char* category) throw();
#else
  virtual void log(timeval tv,const char logLevelName, const char * const category, const char * const message) throw();
#endif

private:
  smsc::core::synchronization::Mutex mutex;
};

}
}
#endif //SMSC_LOGGER_STDERR_APPENDER_HEADER

