#ifndef __SYSTEM_SMPPIO_IOTASK_HPP__
#define __SYSTEM_SMPPIO_IOTASK_HPP__

#include "core/buffers/Array.hpp"
#include "system/smppio/SmppSocket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "smeman/smeman.h"
#include "core/network/Multiplexer.hpp"

namespace smsc{
namespace system{
namespace smppio{

using smsc::core::buffers::Array;
using smsc::core::network::Socket;
using namespace smsc::core::synchronization;
using core::network::Multiplexer;

class SmppSocket;

class SmppIOTask:public smsc::core::threads::ThreadedTask{
public:
  int socketsCount()
  {
    return sockets.Count();
  }
  virtual void addSocket(Socket* sock)=0;
  virtual void removeSocket(Socket *sock)=0;

  void notify()
  {
    mon.Lock();
    mon.notify();
    mon.Unlock();
  }
protected:
  Multiplexer mul;
  EventMonitor mon;
  Array<SmppSocket*> sockets;
};

class SmppInputThread:public SmppIOTask{
public:
  SmppInputThread(smsc::smeman::SmeManager* manager):
    smeManager(manager){}
  virtual void addSocket(Socket* sock);
  virtual void removeSocket(Socket *sock);
  void killSocket(int idx);
  virtual int Execute();
  virtual char* taskName(){return "SmppInputThread";}
  void assignOut(SmppIOTask *out){outTask=out;}
protected:
  smsc::smeman::SmeManager* smeManager;
  EventMonitor *outthreadmon;
  SmppIOTask *outTask;
};

class SmppOutputThread:public SmppIOTask{
public:
  virtual void addSocket(Socket* sock);
  virtual void removeSocket(Socket *sock);
  void killSocket(int idx);
  virtual int Execute();
  virtual char* taskName(){return "SmppOutputThread";}
  void assignIn(SmppIOTask *in){inTask=in;}
protected:
  SmppIOTask *inTask;
};


};//smppio
};//system
};//smsc


#endif
