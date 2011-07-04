#ifndef _SCAG_TRANSPORT_SMPP_SMPPIOBASE2_H
#define _SCAG_TRANSPORT_SMPP_SMPPIOBASE2_H

#include "core/network/Multiplexer.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "SmppSocket2.h"
#include "logger/Logger.h"

namespace scag2 {
namespace transport {
namespace smpp {

namespace sync=smsc::core::synchronization;
namespace net=smsc::core::network;
namespace buf=smsc::core::buffers;
namespace thr=smsc::core::threads;

class SmppIOBase:public thr::ThreadedTask{
public:
  SmppIOBase()
  {
    log_=smsc::logger::Logger::getInstance("smpp.io");
    inactivityTimeout=120;
    enquireLinkTimeout=60;
  }

  ~SmppIOBase()
  {
    for(int i=sockets.Count()-1;i>=0;i--)
    {
      deleteSocketUnsync(i);
    }
  }

  int getSocketsCount()
  {
    MutexGuard mg(mon);
    return sockets.Count();
  }

  virtual void onAddSocket(SmppSocket& sock) {}
  virtual void onDeleteSocket(SmppSocket& sock) {}

  void addSocket(SmppSocketPtr& sock)
  {
    if (!sock) return;
    MutexGuard mg(mon);
    smsc_log_debug(log_, "add socket %p (%u)",sock.get(),sock->refCount());
    // sock->acquire();
    // sock->setMonitor(&mon);
    sockets.Push(sock);
    onAddSocket(*sock);
    mon.notify();
  }


  void deleteSocket(int index)
  {
    MutexGuard mg(mon);
    deleteSocketUnsync(index);
  }
protected:
  void deleteDisconnected()
  {
    for(int i=sockets.Count()-1;i>=0;i--)
    {
      if(!sockets[i]->isConnected())
      {
        deleteSocketUnsync(i);
      }
    }
  }

  void deleteSocketUnsync(int index)
  {
    smsc_log_debug(log_, "delete socket %p (%u)",sockets[index].get(),sockets[index]->refCount());
    onDeleteSocket(*sockets[index]);
    // sockets[index]->release();
    sockets.Delete(index);
  }
  sync::EventMonitor mon;
  buf::Array<SmppSocketPtr> sockets;
  net::Multiplexer mul;
  smsc::logger::Logger* log_;
  int inactivityTimeout;
  int enquireLinkTimeout;
};

}//smpp
}//transport
}//scag

#endif
