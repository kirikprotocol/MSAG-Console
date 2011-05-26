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

  virtual void log(timeval tv,const char logLevelName, const char * const category, const char * const message) throw();

private:
  smsc::core::synchronization::Mutex mutex;
};

}
}
#endif //SMSC_LOGGER_STDERR_APPENDER_HEADER

