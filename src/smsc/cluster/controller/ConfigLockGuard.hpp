/*
 * ConfigLockGuard.hpp
 *
 *  Created on: Feb 18, 2010
 *      Author: skv
 */

#ifndef __SMSC_CLUSTER_CONTROLLER_CONFIGLOCKGUARD_HPP__
#define __SMSC_CLUSTER_CONTROLLER_CONFIGLOCKGUARD_HPP__

#include "NetworkDispatcher.hpp"
#include "eyeline/clustercontroller/ConfigLocks.hpp"
#include "protocol/messages/LockConfig.hpp"
#include "protocol/messages/UnlockConfig.hpp"

namespace smsc{
namespace cluster{
namespace controller{

class ConfigLockGuard{
protected:
  eyeline::clustercontroller::ConfigType ct;
public:
  ConfigLockGuard(eyeline::clustercontroller::ConfigType argCt):ct(argCt)
  {
    protocol::messages::LockConfig lock;
    lock.setConfigType(ct);
    lock.setWriteLock(false);
    while(NetworkDispatcher::getInstance().sendMessage(lock)!=0);
  }
  ~ConfigLockGuard()
  {
    protocol::messages::UnlockConfig unlock;
    unlock.setConfigType(ct);
    NetworkDispatcher::getInstance().enqueueMessage(unlock);
  }
};

}
}
}


#endif /* CONFIGLOCKGUARD_HPP_ */
