//------------------------------------
//  ProfilesStorage.hpp
//  Routman Michael, 2006
//------------------------------------


#ifndef ___PROFILESSTORAGE_H
#define ___PROFILESSTORAGE_H

#include <sys/types.h>

#include <logger/Logger.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/threads/ThreadedTask.hpp>

#include <core/buffers/DiskHash.hpp>
#include <core/buffers/File.hpp>

#include "AbntAddr.hpp"
#include "Profiler.h"
#include "ProfilesStorageKeys.hpp"
#include "MCAEventsStorage.hpp"

namespace smsc {
namespace mcisme {

using namespace core::synchronization;
using namespace core::buffers;

typedef DiskHash<StrKey<28>,AbntProfKey> AddrDiskHash;

class ProfilesStorage
{
public:

  static void Open(const string& path)
  {
    MutexGuard lock(mutex);
    logger = smsc::logger::Logger::getInstance("mci.ProfStor");
    string file = path + "abntprof.hsh";
    if(!File::Exists(file.c_str()))
      profiles.Create(file.c_str(), 1000000, false);
    profiles.Open(file.c_str());
    _isOpen = true;
  }

  static void Close(void)
  {
    MutexGuard lock(mutex);
    if(!_isOpen) return;
    try
    {
      profiles.Close();
      _isOpen = false;
    } catch(...)
    {
      smsc_log_debug(logger, "Exception in ProfileStorage::Close");
    }
  }

  static inline bool isOpen()
  {
    MutexGuard lock(mutex);
    return _isOpen;
  }

  static ProfilesStorage* GetInstance(void)
  {
    MutexGuard lock(mutex);
    if(!_isOpen) return 0;
    if(!pInstance)
      pInstance = new ProfilesStorage();
    return pInstance;
  }

  void Set(const AbntAddr& abnt, const AbonentProfile& prof)
  {
    MutexGuard lock(mutex);
    if(_isOpen)	_Set(abnt, prof);
  }
  bool Get(const AbntAddr& abnt, AbonentProfile& prof)
  {
    MutexGuard lock(mutex);
    if(_isOpen)
      return _Get(abnt, prof);
    return false;
  }
  void Delete(const AbntAddr& abnt)
  {
    MutexGuard lock(mutex);
    if(_isOpen)
      _Delete(abnt);
  }

  int Count(const char* abnt)
  {
    MutexGuard lock(mutex);
    if(_isOpen)
      return profiles.Count();
    else
      return 0;
  }

private:
  void store_P_Event_in_logstore(const std::string& abonent,
                                 unsigned int notifyFlag)
  {
    MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_ChangeAbonentProfile(abonent, notifyFlag));
  }

  void _Set(const AbntAddr& abnt, const AbonentProfile& prof)
  {
    StrKey<28>	key = abnt.toString().c_str();
    AbntProfKey	value(prof.eventMask, prof.informTemplateId, prof.notifyTemplateId, prof.inform, prof.notify, prof.wantNotifyMe);
    store_P_Event_in_logstore(abnt.getText(), prof.notify);
    profiles.Delete(key);
    profiles.Insert(key, value);
  }
  bool _Get(const AbntAddr& abnt, AbonentProfile& prof)
  {
    StrKey<28>	key = abnt.toString().c_str();
    AbntProfKey	value;
    bool res = profiles.LookUp(key, value);
    if(res)
    {
      prof.eventMask = value.key.fields.eventMask;
      prof.informTemplateId = value.key.fields.informTemplateId;
      prof.notifyTemplateId = value.key.fields.notifyTemplateId;
      prof.inform = value.key.fields.inform;
      prof.notify = value.key.fields.notify;
      prof.wantNotifyMe = value.key.fields.wantNotifyMe;
    }
    return res;
  }
  void _Delete(const AbntAddr& abnt)
  {
    StrKey<28>	key = abnt.toString().c_str();
    profiles.Delete(key);
  }
  ProfilesStorage()
  {}

  static ProfilesStorage* pInstance;
  static AddrDiskHash	profiles;
  static Mutex	mutex;
  static bool		_isOpen;
  static smsc::logger::Logger *logger;
};

}	//  namespace msisme
}	//  namespace smsc
#endif
