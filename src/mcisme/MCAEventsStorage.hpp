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
                         unsigned int callerProfileWantNotifyMeFlag)
    : _callingAbonent(callingAbonent), _calledAbonent(calledAbonent),
      _calledProfileNotifyFlag(calledProfileNotifyFlag),
      _callerProfileWantNotifyMeFlag(callerProfileWantNotifyMeFlag)
  {
    gettimeofday(&_eventDate, 0);
  }

  void toString(char eventPrintableCode,char* buf,size_t size) const;
private:
  struct timeval _eventDate;
  std::string _callingAbonent;
  std::string _calledAbonent;
  unsigned int _calledProfileNotifyFlag;
  unsigned int _callerProfileWantNotifyMeFlag;
};

class Event_GotMissedCall : private MissedCall_GenericInfo {
public:
  Event_GotMissedCall(std::string callingAbonent,
                      std::string calledAbonent,
                      unsigned int calledProfileNotifyFlag,
                      unsigned int callerProfileWantNotifyMeFlag)
    : MissedCall_GenericInfo(callingAbonent, calledAbonent,
                             calledProfileNotifyFlag, callerProfileWantNotifyMeFlag)
  {}

  void toString(char* buf,size_t size) const;
};

class Event_MissedCallInfoDelivered : private MissedCall_GenericInfo {
public:
  Event_MissedCallInfoDelivered(std::string callingAbonent,
                                std::string calledAbonent,
                                unsigned int calledProfileNotifyFlag,
                                unsigned int callerProfileWantNotifyMeFlag)
    : MissedCall_GenericInfo(callingAbonent, calledAbonent,
                             calledProfileNotifyFlag, callerProfileWantNotifyMeFlag)
  {}

  void toString(char* buf,size_t size) const;
};

class MissedCallTransitionStateEvent {
public:
  MissedCallTransitionStateEvent(std::string callingAbonent,
                                 std::string calledAbonent)
    : _callingAbonent(callingAbonent), _calledAbonent(calledAbonent)
  {
    gettimeofday(&_eventDate, 0);
  }

  void toString(char eventProntableCode,char* buf,size_t size) const;
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

  void toString(char* buf,size_t size) const;
};

class Event_SendCallerNotification : private MissedCallTransitionStateEvent {
public:
  Event_SendCallerNotification(std::string callingAbonent,
                               std::string calledAbonent)
    : MissedCallTransitionStateEvent(callingAbonent, calledAbonent)
  {}

  void toString(char* buf,size_t size) const;
};

class Event_DeleteMissedCallInfo : private MissedCallTransitionStateEvent {
public:
  Event_DeleteMissedCallInfo(std::string callingAbonent,
                             std::string calledAbonent)
    : MissedCallTransitionStateEvent(callingAbonent, calledAbonent)
  {}

  void toString(char* buf,size_t size) const;
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

  void toString(char* buf,size_t size) const;
private:
  struct timeval _eventDate;
  std::string _abonent;
  unsigned int _profileNotify;
};

class MCAEventsStorage {
public:
  virtual ~MCAEventsStorage() {}

  virtual void addEvent(const Event_GotMissedCall& event) = 0;
  virtual void addEvent(const Event_MissedCallInfoDelivered& event) = 0;
  virtual void addEvent(const Event_MissedCallInfoDeliveringFailed& event) = 0;
  virtual void addEvent(const Event_SendCallerNotification& event) = 0;
  virtual void addEvent(const Event_DeleteMissedCallInfo& event) = 0;
  virtual void addEvent(const Event_ChangeAbonentProfile& event) = 0;
};

class MCAEventsFileStorage : public MCAEventsStorage {
public:
  MCAEventsFileStorage(const std::string& dirName,
                       const std::string& fileNamePrefix,
                       time_t rollingInterval);
  ~MCAEventsFileStorage();

  virtual void addEvent(const Event_GotMissedCall& event);
  virtual void addEvent(const Event_MissedCallInfoDelivered& event);
  virtual void addEvent(const Event_MissedCallInfoDeliveringFailed& event);
  virtual void addEvent(const Event_SendCallerNotification& event);
  virtual void addEvent(const Event_DeleteMissedCallInfo& event);
  virtual void addEvent(const Event_ChangeAbonentProfile& event);
private:
  template <class EVENT>
  void _addEvent(const EVENT& event) {
    char eventBuf[256];
    event.toString(eventBuf,sizeof(eventBuf));
    smsc_log_info(_logger, "MCAEventsStorage::_addEvent::: write event [%s]", eventBuf);
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    rollLoggingFileIfNeeded();
    fprintf(_eventFileFD, "%s\n", eventBuf);
  }

  void findMatchedFiles(const std::string& dirName,
                        const std::string& baseName,
                        const std::string& suffix,
                        std::vector<std::string>* machedFiles);

  void moveFiles(const std::string& dirName,
                 const std::vector<std::string>& originalFiles,
                 const std::string& oldSuffix,
                 const std::string& newSuffix);

  void moveAllPendingCSVFiles();

  std::string calculateTimeStamp(time_t curTime) const;
  void rollLoggingFileIfNeeded();
  void formLogFileNameAndOpenIt(time_t curTime);

  FILE* _eventFileFD;
  std::string _dirName, _fileNamePrefix, _logFileName, _cvsFileName;
  time_t _rollingInterval, _lastRollingTime;
  smsc::core::synchronization::Mutex _lock;
  smsc::logger::Logger* _logger;
};

class FakeMCAEventsStorage : public  MCAEventsStorage {
public:
  virtual void addEvent(const Event_GotMissedCall& event) {}
  virtual void addEvent(const Event_MissedCallInfoDelivered& event) {}
  virtual void addEvent(const Event_MissedCallInfoDeliveringFailed& event) {}
  virtual void addEvent(const Event_SendCallerNotification& event) {}
  virtual void addEvent(const Event_DeleteMissedCallInfo& event) {}
  virtual void addEvent(const Event_ChangeAbonentProfile& event) {}
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
