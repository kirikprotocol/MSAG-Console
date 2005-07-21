#ifndef __SCAG_TRANSPORT_SMPP_SMPPIOBASE_H__
#define __SCAG_TRANSPORT_SMPP_SMPPIOBASE_H__

#include "core/network/Multiplexer.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "SmppSocket.h"

namespace scag{
namespace transport{
namespace smpp{

namespace sync=smsc::core::synchronization;
namespace net=smsc::core::network;
namespace buf=smsc::core::buffers;
namespace thr=smsc::core::threads;

class SmppIOBase:public thr::ThreadedTask{
public:
  int getSocketsCount()
  {
    MutexGuard mg(mon);
    return sockets.Count();
  }
  virtual void onAddSocket(SmppSocket* sock)
  {
  }
  virtual void onDeleteSocket(SmppSocket* sock)
  {
  }
  void addSocket(SmppSocket* sock)
  {
    MutexGuard mg(mon);
    sock->acquire();
    sock->setMonitor(&mon);
    sockets.Push(sock);
    onAddSocket(sock);
    mon.notify();
  }
  void deleteSocket(int index)
  {
    MutexGuard mg(mon);
    onDeleteSocket(sockets[index]);
    sockets[index]->release();
    sockets.Delete(index);
  }
protected:
  sync::EventMonitor mon;
  buf::Array<SmppSocket*> sockets;
  net::Multiplexer mul;
};

}//smpp
}//transport
}//scag


#endif
