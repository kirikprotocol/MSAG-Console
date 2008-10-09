#include <time.h>
#include <sstream>
#include <util/Exception.hpp>
#include "MCAEventsStorage.hpp"

namespace smsc {
namespace mcisme {

static const std::string
time2string(const timeval& tp)
{
  tm lcltm;
  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y%m%d %H:%M:%S.", localtime_r(&tp.tv_sec, &lcltm));
  long msec=tp.tv_usec/1000;

  snprintf(timeStr + strlen(timeStr), sizeof(timeStr) - strlen(timeStr), "%3.3u", msec);
  return timeStr;
}

std::string
MissedCall_GenericInfo::toString(char eventPrintableCode) const
{
  std::ostringstream obuf;
  obuf << eventPrintableCode
       << "," << time2string(_eventDate)
       << "," << _callingAbonent
       << "," << _calledAbonent
       << "," << _calledProfileNotifyFlag
       << "," << _calledProfileWantNotifyMeFlag;

  return obuf.str();
}

std::string
Event_GotMissedCall::toString() const
{
  return MissedCall_GenericInfo::toString('A');
}

std::string
Event_MissedCallInfoDelivered::toString() const
{
  return MissedCall_GenericInfo::toString('D');
}

std::string
MissedCallTransitionStateEvent::toString(char eventPrintableCode) const
{
  std::ostringstream obuf;
  obuf << eventPrintableCode
       << "," << time2string(_eventDate)
       << "," << _callingAbonent
       << "," << _calledAbonent;

  return obuf.str();
}

std::string
Event_MissedCallInfoDeliveringFailed::toString() const
{
  return MissedCallTransitionStateEvent::toString('F');
}

std::string
Event_SendCallerNotification::toString() const
{
  return MissedCallTransitionStateEvent::toString('N');
}

std::string
Event_DeleteMissedCallInfo::toString() const
{
  return MissedCallTransitionStateEvent::toString('E');
}

std::string
Event_ChangeAbonentProfile::toString() const
{
  std::ostringstream obuf;
  obuf << "P," << time2string(_eventDate)
       << "," << _abonent
       << "," << _profileNotify;

  return obuf.str();
}

MCAEventsFileStorage::MCAEventsFileStorage(const std::string& dirName,
                                           const std::string& fileNamePrefix,
                                           time_t rollingInterval)
  : _rollingInterval(rollingInterval), _lastRollingTime(time(0)),
    _logger(logger::Logger::getInstance("mcastrg"))
{
  _fileNamePrefix = dirName + fileNamePrefix;

  formLogFileNameAndOpenIt(_lastRollingTime);
}

MCAEventsFileStorage::~MCAEventsFileStorage()
{
  fclose(_eventFileFD);
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

  _logFileName = _fileNamePrefix + timeStampString + ".log";
  _cvsFileName = _fileNamePrefix + timeStampString + ".csv";

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
