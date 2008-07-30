#ifndef __SMSC_MCISME_MCAEVENTSSTORAGE_HPP__
# define __SMSC_MCISME_MCAEVENTSSTORAGE_HPP__

# include <stdio.h>
# include <time.h>
# include <sys/time.h>
# include <string>
# include <util/config/ConfigView.h>
# include <logger/Logger.h> // for test only!!!

namespace smsc {
namespace mcisme {

class MissedCall_GenericInfo {
public:
  MissedCall_GenericInfo(std::string callingAbonent,
                         std::string calledAbonent,
                         unsigned int calledProfileNotifyFlag,
                         unsigned int calledProfileWantNotifyMeFlag)
    : _callingAbonent(callingAbonent), _calledAbonent(calledAbonent),
      _calledProfileNotifyFlag(calledProfileNotifyFlag),
      _calledProfileWantNotifyMeFlag(calledProfileWantNotifyMeFlag)
  {
    gettimeofday(&_eventDate, 0);
  }

  std::string toString(char eventPrintableCode) const;
private:
  struct timeval _eventDate;
  std::string _callingAbonent;
  std::string _calledAbonent;
  unsigned int _calledProfileNotifyFlag;
  unsigned int _calledProfileWantNotifyMeFlag;
};

class Event_GotMissedCall : private MissedCall_GenericInfo {
public:
  Event_GotMissedCall(std::string callingAbonent,
                      std::string calledAbonent,
                      unsigned int calledProfileNotifyFlag,
                      unsigned int calledProfileWantNotifyMeFlag)
    : MissedCall_GenericInfo(callingAbonent, calledAbonent,
                             calledProfileNotifyFlag, calledProfileWantNotifyMeFlag)
  {}

  std::string toString() const;
};

class Event_MissedCallInfoDelivered : private MissedCall_GenericInfo {
public:
  Event_MissedCallInfoDelivered(std::string callingAbonent,
                                std::string calledAbonent,
                                unsigned int calledProfileNotifyFlag,
                                unsigned int calledProfileWantNotifyMeFlag)
    : MissedCall_GenericInfo(callingAbonent, calledAbonent,
                             calledProfileNotifyFlag, calledProfileWantNotifyMeFlag)
  {}

  std::string toString() const;
};

class MissedCallTransitionStateEvent {
public:
  MissedCallTransitionStateEvent(std::string callingAbonent,
                                 std::string calledAbonent)
    : _callingAbonent(callingAbonent), _calledAbonent(calledAbonent)
  {
    gettimeofday(&_eventDate, 0);
  }

  std::string toString(char eventPrintableCode) const;
private:
  struct timeval _eventDate;
  std::string _callingAbonent;
  std::string _calledAbonent;
};

class Event_MissedCallInfoDeliveringFailed : private MissedCallTransitionStateEvent {
public:
  Event_MissedCallInfoDeliveringFailed(std::string callingAbonent,
                                       std::string calledAbonent)
    : MissedCallTransitionStateEvent(callingAbonent, calledAbonent)
  {}

  std::string toString() const;
};

class Event_SendCallerNotification : private MissedCallTransitionStateEvent {
public:
  Event_SendCallerNotification(std::string callingAbonent,
                               std::string calledAbonent)
    : MissedCallTransitionStateEvent(callingAbonent, calledAbonent)
  {}

  std::string toString() const;
};

class Event_DeleteMissedCallInfo : private MissedCallTransitionStateEvent {
public:
  Event_DeleteMissedCallInfo(std::string callingAbonent,
                             std::string calledAbonent)
    : MissedCallTransitionStateEvent(callingAbonent, calledAbonent)
  {}

  std::string toString() const;
};

class Event_ChangeAbonentProfile {
public:
  Event_ChangeAbonentProfile(std::string abonent,
                             unsigned int profileNotify)
    : _abonent(abonent), _profileNotify(profileNotify)
  {
    gettimeofday(&_eventDate, 0);
    if ( _profileNotify > 1 )
      throw smsc::util::Exception("Event_ChangeAbonentProfile::Event_ChangeAbonentProfile::: invalid profileNotify value='%d', expected value is 0 or 1", _profileNotify);
  }

  std::string toString() const;
private:
  struct timeval _eventDate;
  std::string _abonent;
  unsigned int _profileNotify;
};

class MCAEventsStorage {
public:
  MCAEventsStorage(const std::string& dirName,
                   const std::string& fileNamePrefix,
                   time_t rollingInterval);
  ~MCAEventsStorage();

  template <class EVENT>
  void addEvent(const EVENT& event) {
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("mcastrg");
    smsc_log_info(logger, "MCAEventsStorage::addEvent::: write event [%s]", event.toString().c_str());
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    rollLoggingFileIfNeeded();
    fprintf(_eventFileFD, "%s\n", event.toString().c_str());
  }

private:
  std::string calculateTimeStamp(time_t curTime) const;
  void rollLoggingFileIfNeeded();
  void formLogFileNameAndOpenIt(time_t curTime);

  FILE* _eventFileFD;
  std::string _fileNamePrefix, _logFileName, _cvsFileName;
  time_t _rollingInterval, _lastRollingTime;
  smsc::core::synchronization::Mutex _lock;
};

class MCAEventsStorageRegister {
public:
  static void init(const smsc::util::config::ConfigView& config);
  static MCAEventsStorage& getMCAEventsStorage();
  
private:
  static MCAEventsStorage* _storage;
};

}}

#endif
