#include "StatisticsLogger.h"
#include <dirent.h>
#include <vector>
#include <exception>
#include "core/synchronization/Mutex.hpp"

using smsc::core::synchronization::MutexGuard;
using std::vector;

namespace scag2 {
namespace stat  {

static const size_t PREFIX_MAXSIZE = 30;
static const size_t SUFFIX_MAXSIZE = 30;
static const size_t STAT_FILE_NAME_MAXSIZE = PREFIX_MAXSIZE + SUFFIX_MAXSIZE + 1;

template<typename Event, typename Buffer>
StatisticsLogger<Event, Buffer>::StatisticsLogger(const string& statDir, const string& saaDir, const string& prefix, uint32_t interval):
    statDir_(statDir), saaDir_(saaDir), prefix_(prefix), format_(prefix_ + ".%lld"),
    interval_(interval), lastFileTime_(0), logger_(Logger::getInstance("statlog")) {
  //TODO: check prefix size
  checkDir();
  pdubuf_.resize(0xFFFF);
};

template<typename Event, typename Buffer>
StatisticsLogger<Event, Buffer>::~StatisticsLogger() {
  MutexGuard mg(mutex_);
  file_.Close();
}

template<typename Event, typename Buffer>
void StatisticsLogger<Event, Buffer>::logEvent(Event* event) {
  if (!event) {
    return;
  }
  pdubuf_.WriteNetInt32(0);
  pdubuf_.setPos(static_cast<uint32_t>(sizeof(uint32_t)));
  pdubuf_.WriteNetInt16(event->getEventType());
  event->write(pdubuf_);
  uint32_t bsize = pdubuf_.getPos();
  pdubuf_.setPos(0);
  pdubuf_.WriteNetInt32(bsize);
  pdubuf_.setPos(0);
  try {
    MutexGuard mg(mutex_);
    if (!file_.isOpened()) {
      lastFileTime_ = time(0);
      file_.WOpen(getFileName(statDir_, lastFileTime_).c_str());
      smsc_log_debug(logger_, "create stat file '%s'", getFileName(statDir_, lastFileTime_).c_str());
    }
    file_.Write(pdubuf_.getBuffer(), pdubuf_.getBufferSize());
  } catch (const std::exception& ex) {
    smsc_log_warn(logger_, "write event to file error: %s", ex.what());
  }
}

template<typename Event, typename Buffer>
void StatisticsLogger<Event, Buffer>::rollover() {
  try {
    time_t curTime = time(0);
    string currentName;
    string newName;
    {
      MutexGuard mg(mutex_);
      if (!file_.isOpened() || curTime - lastFileTime_ < interval_) {
        return;
      }
      currentName = file_.getFileName();
      newName = getFileName(saaDir_, lastFileTime_);
      file_.Close();
    }
    if (!currentName.empty() && File::Exists(currentName.c_str())) {
      File::Rename(currentName.c_str(), newName.c_str());
      smsc_log_debug(logger_, "rename stat file '%s' with '%s'", currentName.c_str(), newName.c_str());  
    }
  } catch (const std::exception& ex) {
    smsc_log_warn(logger_, "rollover stat file error: %s", ex.what());
  }
}

template<typename Event, typename Buffer>
void StatisticsLogger<Event, Buffer>::createSaaDir() const {
  if (!File::Exists(saaDir_.c_str())) {
    smsc_log_info(logger_, "create saa dir '%s'", saaDir_.c_str());
    File::MkDir(saaDir_.c_str());
  }
}

template<typename Event, typename Buffer>
bool StatisticsLogger<Event, Buffer>::checkDir() {
  smsc_log_info(logger_, "check statistics dir '%s'", statDir_.c_str());
  if (!File::Exists(statDir_.c_str())) {
    smsc_log_info(logger_, "create statistics dir '%s'", statDir_.c_str());
    File::MkDir(statDir_.c_str());
    createSaaDir();
    return true;
  }
  createSaaDir();
  string fullprefix = prefix_ + '.';
  time_t curTime = time(0);
  vector<string> dirFiles;
  File::ReadDir(statDir_.c_str(), dirFiles, File::rdfFilesOnly);
  for (vector<string>::iterator i = dirFiles.begin(); i != dirFiles.end(); ++i) {
    if ((*i).size() <= prefix_.size() || (*i).compare(0, prefix_.size(), prefix_)) {
      smsc_log_warn(logger_, "statistics file invalid name(prefix): '%s', must be '%s.[time_in_msec]'", (*i).c_str(), prefix_.c_str());
      continue;
    }
    time_t ftime = getFileTime((*i).c_str());
    if (!ftime) {
      smsc_log_warn(logger_, "statistics file invalid name(postfix): '%s', must be '%s.[time_in_msec]'", (*i).c_str(), prefix_.c_str());
      continue;
    }
    if (curTime - ftime <= 0) {
      smsc_log_warn(logger_, "statistics file creation time=%lld >= current time=%lld", ftime, curTime);
      //TODO: del invalid files
      continue;
    }
    if (curTime - ftime <= interval_) {
      smsc_log_info(logger_, "open statistics file: %s", (*i).c_str());
      file_.WOpen(string(statDir_+ '/' + *i).c_str());
      lastFileTime_ = ftime;
    } else {
      string currentName = statDir_ + '/' + *i;
      string newName = saaDir_ + '/' + *i;
      smsc_log_info(logger_, "rename statistics file cur name: '%s', new name: '%s'", currentName.c_str(), newName.c_str());
      if (!currentName.empty() && File::Exists(currentName.c_str())) {
        File::Rename(currentName.c_str(), newName.c_str());
      }
    }
  }

  smsc_log_info(logger_, "statistics dir '%s' checked", statDir_.c_str());
  return true;
}

template<typename Event, typename Buffer>
time_t StatisticsLogger<Event, Buffer>::getFileTime(const char* fname) const {
  time_t ftime = 0;
  sscanf(fname, format_.c_str(), &ftime);
  return ftime;
}

template<typename Event, typename Buffer>
string StatisticsLogger<Event, Buffer>::getFileName(const string& path, time_t curTime) const {
  char buf[STAT_FILE_NAME_MAXSIZE];
  memset(buf, 0, STAT_FILE_NAME_MAXSIZE);
  snprintf(buf, STAT_FILE_NAME_MAXSIZE, "%s.%d", prefix_.c_str(), curTime);
  return path + '/' + string(buf);
}

template<typename Event, typename Buffer>
uint32_t StatisticsLogger<Event, Buffer>::getRollingInterval() const {
  return interval_;
}


}
}


