#ifndef __SYSTEM_SMPPIO_SOCKETMANAGER_HPP__
#define __SYSTEM_SMPPIO_SOCKETMANAGER_HPP__

#include "core/buffers/Array.hpp"
#include "system/smppio/SmppIOTask.hpp"
#include "core/threads/ThreadPool.hpp"
#include "smeman/smeman.h"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

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
    tp(newtp),smeManager(manager)
  {
    log=smsc::logger::Logger::getInstance("smpp.sock");
  }
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
  smsc::logger::Logger* log;
};//SocketsManager

}//smppio
}//system
}//smsc

#endif
