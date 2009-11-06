#ifndef _SMSC_INFOSME2_TASKLOCK_H
#define _SMSC_INFOSME2_TASKLOCK_H

#include <string>
#include <set>
#include "core/buffers/FixedRecordFile.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace infosme2 {

class TaskLock 
{
public:

  static void Init(const char* storeFile);
  static void Shutdown();

  static TaskLock& getInstance()
  {
    return *instance;
  }

  void Lock(uint32_t taskId)
  {
      smsc::core::synchronization::MutexGuard mg(mtx);
    IntSet::iterator it=lockSet.find(taskId);
    if(it!=lockSet.end())
    {
      smsc_log_warn(log,"Task %d already locked",taskId);
      return;
    }
    smsc_log_info(log,"Task %d locked",taskId);
    TaskLockRecord rec(taskId);
    rec.offset=store.Append(rec);
    lockSet.insert(rec);
  }

  bool Unlock(uint32_t taskId)
  {
      smsc::core::synchronization::MutexGuard mg(mtx);
      IntSet::iterator it=lockSet.find(taskId);
    if(it==lockSet.end())
    {
      smsc_log_info(log,"Task %d isn't locked",taskId);
      return false;
    }
    store.Delete(it->offset);
    lockSet.erase(it);
    return true;
  }

protected:

  //static const char* sig=;
  static const uint32_t ver=0x100;

  TaskLock():store("TASKLOCK",ver)
  {
  }

  static TaskLock* instance;
  std::string fileName;

  void Load()
  {
    store.Open(fileName.c_str());
    TaskLockRecord rec(0);
    while(rec.offset=store.Read(rec))
    {
      lockSet.insert(rec);
    }
  }

  struct TaskLockRecord{
    uint32_t uid;
      smsc::core::buffers::File::offset_type offset;

    TaskLockRecord(uint32_t argUid):uid(argUid)
    {
    }

    bool operator<(const TaskLockRecord& rhs)const
    {
      return uid<rhs.uid;
    }

      void Read(smsc::core::buffers::File& f)
    {
      uid=f.ReadNetInt32();
    }
      void Write(smsc::core::buffers::File& f)const
    {
      f.WriteNetInt32(uid);
    }
    static size_t Size()
    {
      return 4;
    }
  };
    smsc::logger::Logger* log;
    smsc::core::synchronization::Mutex mtx;
    typedef std::set<TaskLockRecord> IntSet;
    IntSet lockSet;
    smsc::core::buffers::FixedRecordFile<TaskLockRecord> store;
};

}
}

#endif
