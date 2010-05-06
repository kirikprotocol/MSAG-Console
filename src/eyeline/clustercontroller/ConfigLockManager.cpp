/*
 * ConfigLockManager.cpp
 *
 *  Created on: Feb 17, 2010
 *      Author: skv
 */

#include "eyeline/clustercontroller/ConfigLockManager.hpp"
#include "util/TimeSource.h"

namespace eyeline{
namespace clustercontroller{

ConfigLockManager* ConfigLockManager::instance=0;

const char* configName[ctConfigsCount]={
    "Main Config",
    "Profiles",
    "Msc",
    "Routes",
    "Sme",
    "Closed Groups",
    "Aliases",
    "Map Limits",
    "Regions",
    "Resources",
    "Reschedule",
    "Snmp",
    "Time Zones",
    "Fraud"
};

ConfigLockManager::Lock::Lock(int argConnId,bool argWriteLock):connId(argConnId),writeLock(argWriteLock)
{
  lockTime=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
}


void ConfigLockManager::LockForRead(ConfigType ct,int connId)
{
  sync::MutexGuard mg(mon);
  while(writeLocks[ct])
  {
    mon.wait(1000);
    time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
    if(now-locks[ct].front().lockTime>10*60)
    {
      smsc_log_warn(log,"Config '%s' locked by connId=%d for too long(%d sec)",configName[ct],locks[ct].front().connId,now-locks[ct].front().lockTime);
      locks[ct].erase(locks[ct].begin());
      writeLocks[ct]=false;
      break;
    }
  }
  readLocks[ct]++;
  locks[ct].push_back(Lock(connId,false));
  smsc_log_info(log,"Config '%s' locked for read(%d) by connId=%d",configName[ct],readLocks[ct],connId);
}

void ConfigLockManager::LockForWrite(ConfigType ct,int connId)
{
  sync::MutexGuard mg(mon);
  while(readLocks[ct] || writeLocks[ct])
  {
    mon.wait(1000);
    time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
    LockVector& lv=locks[ct];
    for(LockVector::iterator it=lv.begin(),end=lv.end();it!=end;it++)
    {
      if(now-it->lockTime>10*60)
      {
        smsc_log_warn(log,"Config '%s' locked by connId=%d for too long(%d sec)",configName[ct],it->connId,now-it->lockTime);
        if(it->writeLock)
        {
          writeLocks[ct]=false;
        }else
        {
          readLocks[ct]--;
        }
        lv.erase(it);
        break;
      }
    }
  }
  smsc_log_info(log,"Config '%s' locked for write by connId=%d",configName[ct],connId);
  writeLocks[ct]=true;
  locks[ct].push_back(Lock(connId,true));
}


bool ConfigLockManager::TryLockForWrite(ConfigType ct,int connId)
{
  sync::MutexGuard mg(mon);
  if(readLocks[ct] || writeLocks[ct])
  {
    return false;
  }
  smsc_log_info(log,"Config '%s' locked for write by connId=%d",configName[ct],connId);
  writeLocks[ct]=true;
  locks[ct].push_back(Lock(connId,true));
  return true;
}

void ConfigLockManager::Unlock(ConfigType ct,int connId)
{
  sync::MutexGuard mg(mon);
  if(writeLocks[ct])
  {
    if(locks[ct].empty() || locks[ct].front().connId!=connId)
    {
      smsc_log_warn(log,"Attempt to unlock config '%s' by connId=%d which was locked by connId=%d",configName[ct],connId,locks[ct].front().connId);
      return;
    }
    smsc_log_info(log,"Config '%s' unlocked(write)",configName[ct]);
    locks[ct].erase(locks[ct].begin());
    writeLocks[ct]=false;
  }else
  {
    bool found=false;
    for(LockVector::iterator it=locks[ct].begin(),end=locks[ct].end();it!=end;it++)
    {
      if(it->connId==connId)
      {
        locks[ct].erase(it);
        found=true;
        break;
      }
    }
    if(found)
    {
      smsc_log_info(log,"Config '%s' unlocked(read:%d)",configName[ct],readLocks[ct]);
      readLocks[ct]--;
    }else
    {
      smsc_log_warn(log,"Attempt to unlock config '%s' by connId=%d which wasn't locked by this connId",configName[ct],connId);
    }
  }
  mon.notify();
}

void ConfigLockManager::UnlockByConn(int connId)
{
  sync::MutexGuard mg(mon);
  for(int i=0;i<ctConfigsCount;i++)
  {
    for(LockVector::iterator it=locks[i].begin(),end=locks[i].end();it!=end;it++)
    {
      if(it->connId==connId)
      {
        if(it->writeLock)
        {
          writeLocks[i]=false;
        }else
        {
          readLocks[i]--;
        }
        locks[i].erase(it);
        it--;
      }
    }
  }
  mon.notify();
}

ConfigLockManager* ConfigLockManager::getInstance()
{
  return instance;
}

void ConfigLockManager::Init()
{
  instance=new ConfigLockManager;
}
void ConfigLockManager::Shutdown()
{
  delete instance;
  instance=0;
}

}
}


