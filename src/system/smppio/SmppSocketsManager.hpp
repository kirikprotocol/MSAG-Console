#ifndef __SYSTEM_SMPPIO_SOCKETMANAGER_HPP__
#define __SYSTEM_SMPPIO_SOCKETMANAGER_HPP__

#include "core/buffers/Array.hpp"
#include "system/smppio/SmppIOTask.hpp"
#include "core/threads/ThreadPool.hpp"
#include "smeman/smeman.h"
#include "core/synchronization/Mutex.hpp"

#define SM_SOCK_PER_THREAD 16

namespace smsc{
namespace system{
namespace smppio{

using smsc::core::buffers::Array;
using smsc::core::threads::ThreadPool;
using namespace smsc::core::synchronization;

class SmppSocketsManager{
public:
  SmppSocketsManager(ThreadPool *newtp,smsc::smeman::SmeManager* manager):
    tp(newtp),smeManager(manager){}
  void registerSocket(Socket* sock);
  int removeSocket(Socket* sock);
  void setSmppSocketTimeout(int to)
  {
    socketTimeOut=to;
  }
  void setInactivityTime(int tm)
  {
    inactivityTime=tm;
  }
  void setInactivityTimeOut(int to)
  {
    inactivityTimeOut=to;
  }
protected:
  Array<SmppIOTask*> intasks,outtasks;
  ThreadPool *tp;
  int socketTimeOut;
  int inactivityTime;
  int inactivityTimeOut;
  smsc::smeman::SmeManager* smeManager;
  Mutex mtxAdd,mtxRemove;
};//SocketsManager

}//smppio
}//system
}//smsc

#endif
