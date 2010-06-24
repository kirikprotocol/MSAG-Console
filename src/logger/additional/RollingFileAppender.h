#ifndef SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER
#define SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER

#include <stdio.h>

#include <logger/Appender.h>
#include <core/synchronization/Mutex.hpp>
#include <util/Properties.h>
#include "core/buffers/File.hpp"

namespace smsc {
namespace logger {

using namespace smsc::util;

class RollingFileAppender : public Appender
{
public:
  RollingFileAppender(const char * const name, const Properties & properties);

  virtual void log(timeval tv,const char logLevelName, const char * const category, const char * const message) throw();

private:
  smsc::core::synchronization::Mutex mutex;
  unsigned int maxBackupIndex;
  long maxFileSize;
  std::auto_ptr<const char> filename;
  smsc::core::buffers::File file;
  long currentFilePos;
  bool directIO;

  void rollover() throw();
};

}
}

#endif //SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER
