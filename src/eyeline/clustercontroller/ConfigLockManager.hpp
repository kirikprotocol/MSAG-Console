/*
 * ConfigLockManager.hpp
 *
 *  Created on: Feb 17, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTER_CONTROLLER_CONFIGLOCKMANAGER_HPP__
#define __EYELINE_CLUSTER_CONTROLLER_CONFIGLOCKMANAGER_HPP__

#include "ConfigLocks.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include <vector>

namespace eyeline{
namespace clustercontroller{

namespace sync=smsc::core::synchronization;
namespace thr=smsc::core::threads;

class ConfigLockManager{
protected:
  int readLocks[ctConfigsCount];
  bool writeLocks[ctConfigsCount];

  struct Lock{
    Lock(int argConnId,bool argWriteLock=false);
    int connId;
    bool writeLock;
    time_t lockTime;
  };

  typedef std::vector<Lock> LockVector;
  LockVector locks[ctConfigsCount];

  sync::EventMonitor mon;
  smsc::logger::Logger* log;
  static ConfigLockManager* instance;
public:
  ConfigLockManager()
  {
    for(size_t i=0;i<ctConfigsCount;i++)
    {
      readLocks[i]=0;
      writeLocks[i]=false;
    }
    log=smsc::logger::Logger::getInstance("cfg.lock");
  }
  static ConfigLockManager* getInstance();
  static void Init();
  static void Shutdown();
  void LockForRead(ConfigType ct,int connId);
  void LockForWrite(ConfigType ct,int connId);
  bool TryLockForWrite(ConfigType ct,int connId);
  void Unlock(ConfigType ct,int connId);
  void UnlockByConn(int connId);
};


}
}

#endif /* CONFIGLOCKMANAGER_HPP_ */
