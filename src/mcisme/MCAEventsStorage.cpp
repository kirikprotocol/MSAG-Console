#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sstream>

#include <util/Exception.hpp>
#include "MCAEventsStorage.hpp"

namespace smsc {
namespace mcisme {

static void time2string(const timeval& tp,char* timeStr,size_t size)
{
  tm lcltm;
  //char timeStr[64];
  strftime(timeStr, size, "%Y%m%d %H:%M:%S.", localtime_r(&tp.tv_sec, &lcltm));
  long msec=tp.tv_usec/1000;

  snprintf(timeStr + strlen(timeStr), size - strlen(timeStr), "%3.3u", msec);
}

void
MCAEventsFileStorage::findMatchedFiles(const std::string& dirName,
                                       const std::string& baseName,
                                       const std::string& suffix,
                                       std::vector<std::string>* machedFiles)
{
  errno = 0;
  DIR* dir = opendir(dirName.c_str());
  if (!dir) {
    std::string errMsg = "moveMatchedFiles::: can't open directory=[";
    errMsg += dirName + "]";
    throw util::SystemError(errMsg);
  }

  struct dirent dirEntry, *realDirEntry=NULL;
  char BUFFER[1024];
  while (!readdir_r(dir, (struct dirent*)BUFFER, &realDirEntry) && realDirEntry) {
    if ( !strncmp(realDirEntry->d_name, baseName.c_str(), baseName.length()) &&
         !strncmp(realDirEntry->d_name + strlen(realDirEntry->d_name) - suffix.length(), suffix.c_str(), suffix.length()) )
      machedFiles->push_back(realDirEntry->d_name);
    realDirEntry=NULL;
  }

  closedir(dir);

  if (errno)
    throw util::SystemError("moveMatchedFiles::: can't read directory entry");
}

void
MCAEventsFileStorage::moveFiles(const std::string& dirName,
                                const std::vector<std::string>& originalFiles,
                                const std::string& oldSuffix,
                                const std::string& newSuffix)
{
  for(std::vector<std::string>::const_iterator i = originalFiles.begin(), end_iter = originalFiles.end();
      i != end_iter; ++i) {
    std::string oldFullFileName = dirName + ((dirName[dirName.length()-1] == '/') ? "" : std::string("/")) + (*i);
    std::string newFullFileName = oldFullFileName.substr(0, oldFullFileName.length() - oldSuffix.length()) + newSuffix;
    smsc_log_debug(_logger, "MCAEventsFileStorage::moveFiles::: rename %s to %s", oldFullFileName.c_str(), newFullFileName.c_str());
    if ( rename(oldFullFileName.c_str(), newFullFileName.c_str()) ) {
      std::string errMsg("moveFiles::: can't rename file ");
      errMsg += oldFullFileName + "to " + newFullFileName;
      throw SystemError(errMsg);
    }
  }
}

void
MissedCall_GenericInfo::toString(char eventPrintableCode,char* buf,size_t size) const
{
  char timeBuf[64];
  time2string(_eventDate,timeBuf,sizeof(timeBuf));
  snprintf(buf,size,"%c,%s,%s,%s,%u,%u",eventPrintableCode,timeBuf,
      _callingAbonent.c_str(),_calledAbonent.c_str(),_calledProfileNotifyFlag,_callerProfileWantNotifyMeFlag);
  /*
  std::ostringstream obuf;
  obuf << eventPrintableCode
       << "," << time2string(_eventDate)
       << "," << _callingAbonent
       << "," << _calledAbonent
       << "," << _calledProfileNotifyFlag
       << "," << _callerProfileWantNotifyMeFlag;

  return obuf.str();
  */
}

void
Event_GotMissedCall::toString(char* buf,size_t size) const
{
  return MissedCall_GenericInfo::toString('A',buf,size);
}

void
Event_MissedCallInfoDelivered::toString(char* buf,size_t size) const
{
  return MissedCall_GenericInfo::toString('D',buf,size);
}

void
MissedCallTransitionStateEvent::toString(char eventPrintableCode,char* buf,size_t size) const
{
  char timeBuf[64];
  time2string(_eventDate,timeBuf,sizeof(timeBuf));
  snprintf(buf,size,"%c,%s,%s,%s",eventPrintableCode,timeBuf,_callingAbonent.c_str(),_calledAbonent.c_str());
  /*
  std::ostringstream obuf;
  obuf << eventPrintableCode
       << "," << time2string(_eventDate)
       << "," << _callingAbonent
       << "," << _calledAbonent;

  return obuf.str();
  */
}

void
Event_MissedCallInfoDeliveringFailed::toString(char* buf,size_t size) const
{
  return MissedCallTransitionStateEvent::toString('F',buf,size);
}

void
Event_SendCallerNotification::toString(char* buf,size_t size) const
{
  return MissedCallTransitionStateEvent::toString('N',buf,size);
}

void
Event_DeleteMissedCallInfo::toString(char* buf,size_t size) const
{
  return MissedCallTransitionStateEvent::toString('E',buf,size);
}

void
Event_ChangeAbonentProfile::toString(char* buf,size_t size) const
{
  char timeBuf[64];
  time2string(_eventDate,timeBuf,sizeof(timeBuf));
  snprintf(buf,size,"%c,%s,%s,%s",'P',timeBuf,_abonent.c_str(),_profileNotify);
  /*
  std::ostringstream obuf;
  obuf << "P," << time2string(_eventDate)
       << "," << _abonent
       << "," << _profileNotify;

  return obuf.str();
  */
}

MCAEventsFileStorage::MCAEventsFileStorage(const std::string& dirName,
                                           const std::string& fileNamePrefix,
                                           time_t rollingInterval)
  : _rollingInterval(rollingInterval), _lastRollingTime(time(0)),
    _logger(logger::Logger::getInstance("mcastrg"))
{
  _dirName = dirName;
  _fileNamePrefix = fileNamePrefix;

  moveAllPendingCSVFiles();
  formLogFileNameAndOpenIt(_lastRollingTime);
}

MCAEventsFileStorage::~MCAEventsFileStorage()
{
  fclose(_eventFileFD);
}

void
MCAEventsFileStorage::moveAllPendingCSVFiles()
{
  std::vector<std::string> matchedFiles;

  findMatchedFiles(_dirName, _fileNamePrefix, "log", &matchedFiles);

  moveFiles(_dirName,
            matchedFiles,
            ".log",
            ".csv");
}

std::string
MCAEventsFileStorage::calculateTimeStamp(time_t curTime) const
{
  tm lcltm;
  char fileNameSuffix[64];

  strftime(fileNameSuffix, sizeof(fileNameSuffix), ".%Y%m%d%H%M%S", localtime_r(&curTime, &lcltm));

  return fileNameSuffix;
}

void
MCAEventsFileStorage::rollLoggingFileIfNeeded()
{
  time_t curTime = time(0);
  if ( curTime - _lastRollingTime < _rollingInterval )
    return;

  fclose(_eventFileFD);

  if ( rename(_logFileName.c_str(), _cvsFileName.c_str()) < 0 )
    throw smsc::util::SystemError("MCAEventsFileStorage::rollLoggingFileIfNeeded::: call to rename() failed");

  _lastRollingTime = curTime;

  formLogFileNameAndOpenIt(curTime);
}

void
MCAEventsFileStorage::formLogFileNameAndOpenIt(time_t curTime)
{
  std::string timeStampString = calculateTimeStamp(curTime);

  _logFileName = _dirName + _fileNamePrefix + timeStampString + ".log";
  _cvsFileName = _dirName + _fileNamePrefix + timeStampString + ".csv";

  if ( !(_eventFileFD = fopen(_logFileName.c_str(), "a+")) ) {
    char errMsg[128];
    snprintf(errMsg, sizeof(errMsg), "MCAEventsFileStorage::formLogFileNameAndOpenIt::: fopen failed for file='%s'", _logFileName.c_str());
    throw smsc::util::SystemError(errMsg);
  }

  if ( setvbuf(_eventFileFD, NULL, _IOLBF, 0) )
    throw smsc::util::SystemError("MCAEventsFileStorage::formLogFileNameAndOpenIt::: call to setvbuf() failed");
}

void
MCAEventsFileStorage::addEvent(const Event_GotMissedCall& event)
{
  _addEvent(event);
}

void
MCAEventsFileStorage::addEvent(const Event_MissedCallInfoDelivered& event)
{
  _addEvent(event);
}

void
MCAEventsFileStorage::addEvent(const Event_MissedCallInfoDeliveringFailed& event)
{
  _addEvent(event);
}

void
MCAEventsFileStorage::addEvent(const Event_SendCallerNotification& event)
{
  _addEvent(event);
}

void
MCAEventsFileStorage::addEvent(const Event_DeleteMissedCallInfo& event)
{
  _addEvent(event);
}

void
MCAEventsFileStorage::addEvent(const Event_ChangeAbonentProfile& event)
{
  _addEvent(event);
}

void
MCAEventsStorageRegister::init(const smsc::util::config::ConfigView& config)
{
  std::string storageDir;
  try {
    storageDir = config.getString("location");

    if ( storageDir.empty() || storageDir.find_first_not_of(" ") == std::string::npos )
      _storage = new FakeMCAEventsStorage();
    else {
      time_t rollingInterval;
      try {
        rollingInterval = config.getInt("rollingInterval");
      } catch(smsc::util::config::ConfigException& ex) {
        rollingInterval = 60;
      }

      if (storageDir[storageDir.size() - 1] != '/')
        storageDir.push_back('/');

      _storage = new MCAEventsFileStorage(storageDir, "mcajournal", rollingInterval);
    }
  } catch(smsc::util::config::ConfigException& ex) {
    _storage = new FakeMCAEventsStorage();
  }

}

MCAEventsStorage&
MCAEventsStorageRegister::getMCAEventsStorage()
{
  if ( _storage )
    return *_storage;
  else
    throw smsc::util::Exception("MCAEventsStorageRegister::getMCAEventsStorage::: MCAEventsStorage wasn't initialized");
}

MCAEventsStorage*
MCAEventsStorageRegister::_storage;

}}
