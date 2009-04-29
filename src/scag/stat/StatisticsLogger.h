#ifndef SMSC_SCAG_STAT_STATISTICS_LOGGER
#define SMSC_SCAG_STAT_STATISTICS_LOGGER

#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include "logger/Logger.h"
#include "core/buffers/File.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/stat/base/Statistics2.h"
#include "core/threads/Thread.hpp"

namespace scag2 {
namespace stat {

using std::string;
using smsc::logger::Logger;
using smsc::core::buffers::File;
using smsc::core::synchronization::Mutex;

template<typename Event, typename Buffer>
class StatisticsLogger {
public:
  StatisticsLogger(const string& statDir, const string& saaDir, const string& prefix, uint32_t interval);
  ~StatisticsLogger();
  void rollover();
  void logEvent(Event* event);
  uint32_t getRollingInterval() const;

private:
  bool checkDir();
  time_t getFileTime(const char* fileName) const;
  string getFileName(const string& path, time_t curTime) const;
  void createSaaDir() const;

private:
  string statDir_;
  string saaDir_;
  string prefix_;
  string format_;
  uint32_t interval_; 
  File file_;
  time_t lastFileTime_;
  Buffer pdubuf_;
  Mutex mutex_;
  Logger* logger_;
};

template<typename Event, typename Buffer>
class StatisticsRoller : public smsc::core::threads::Thread {
public:
  StatisticsRoller(StatisticsLogger<Event, Buffer>* statlog):statlog_(statlog), interval_(statlog->getRollingInterval() * 1000) {
    if (statlog_) {
      Start();
      stopped_ = false;
    }
  }
  ~StatisticsRoller() {
    Stop();
  }
  int Execute() {
    while (!stopped_) {
      {
        MutexGuard mg(monitor_);
        monitor_.wait(interval_);
      }
      if (statlog_) {
        statlog_->rollover();
      }
    }
    return 1;
  }
  bool Stop() {
    if (stopped_) {
      return stopped_;
    }
    MutexGuard mg(monitor_);
    stopped_ = true;
    monitor_.notify();
    return stopped_;
  }
  bool isStopped() {
    return stopped_;
  }
private:
  StatisticsLogger<Event, Buffer>* statlog_;
  int interval_;
  bool stopped_;
  EventMonitor monitor_;
};

}
}

#endif


