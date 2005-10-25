#ifndef SMSC_LOGGER_ADDITIONAL_ROLLINGDAYAPPENDER
#define SMSC_LOGGER_ADDITIONAL_ROLLINGDAYAPPENDER

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

class RollingDayAppender : public Appender
{
public:
  RollingDayAppender(const char * const name, const Properties & properties);

  virtual void log(const char logLevelName, const char * const category, const char * const message) throw();

private:
    void clearLogDir(time_t dat);
  smsc::core::synchronization::Mutex mutex;
  unsigned int maxBackupIndex;
    std::string filename;
  smsc::core::buffers::File file;
  long currentFilePos;
    time_t date_;

  void rollover(time_t dat) throw();
};

}
}

#endif //SMSC_LOGGER_ADDITIONAL_ROLLINGFILEAPPENDER
