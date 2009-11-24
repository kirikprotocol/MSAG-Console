#ifndef __SYSTEM_SMPPIO_SOCKETMANAGER_HPP__
#define __SYSTEM_SMPPIO_SOCKETMANAGER_HPP__

#include <map>

#include "core/buffers/Array.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "smsc/smeman/smeman.h"

#include "SmppIOTask.hpp"

#define SM_SOCK_PER_THREAD 16

namespace smsc{
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
    useAbortiveClose=true;
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
  void setDefaultConnectionsLimit(int lim)
  {
    defaultConnectLimit=lim;
  }
  void setLimitForIp(const char* ip,int lim)
  {
    MutexGuard mg(mtxRemove);
    connectLim.Insert(ip,lim);
    info2(log,"Set limit for %s to %d",ip,lim);
  }
  void setUseAbortiveClose(bool val)
  {
    useAbortiveClose=val;
  }
  bool getUseAbortiveClose()
  {
    return useAbortiveClose;
  }
protected:
  Array<SmppIOTask*> intasks,outtasks;
  ThreadPool *tp;
  int socketTimeOut;
  int inactivityTime;
  int inactivityTimeOut;
  bool useAbortiveClose;
  smsc::smeman::SmeManager* smeManager;
  Mutex mtxAdd,mtxRemove;
  smsc::logger::Logger* log;

  int defaultConnectLimit;
  typedef std::map<Socket*,std::string> Ptr2AddrMap;
  Ptr2AddrMap ptr2addr;
  smsc::core::buffers::Hash<int> connectCnt;
  smsc::core::buffers::Hash<int> connectLim;
};//SocketsManager

}//smppio
}//smsc

#endif
