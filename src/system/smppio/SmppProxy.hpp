#ifndef __SYSTEM_SMPPIO_SMPPPROXY_HPP__
#define __SYSTEM_SMPPIO_SMPPPROXY_HPP__

#include "smeman/smeproxy.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/PriorityQueue.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "system/smppio/SmppSocket.hpp"
#include "smeman/smsccmd.h"
#include <string>

namespace smsc{
namespace system{
namespace smppio{

#define SMPP_PROXY_QUEUE_LIMIT 4096

const int proxyTranmitter=1;
const int proxyReceiver=2;
const int proxyTransceiver=3;

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

class SmppSocket;

class SmppProxy:public SmeProxy{
public:
  SmppProxy(SmppSocket* sock):smppSocket(sock)
  {
    smppSocket->assignProxy(this);
    seq=1;
    managerMonitor=NULL;
    proxyType=proxyTransceiver;
    opened=true;
  }
  virtual ~SmppProxy(){}
  virtual void close()
  {
    opened=false;
  }
  bool CheckValidIncomingCmd(const SmscCommand& cmd);
  bool CheckValidOutgoingCmd(const SmscCommand& cmd);


  virtual void putCommand(const SmscCommand& cmd)
  {
    trace("put command:enter");
    if(!CheckValidIncomingCmd(cmd))
    {
      putIncomingCommand
      (
        SmscCommand::makeGenericNack
        (
          cmd->get_dialogId(),
          SmppStatusSet::ESME_RINVBNDSTS
        )
      );
    }
    {
      MutexGuard g(mutexout);
      if(!opened)return;
      if(outqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
      {
        throw ProxyQueueLimitException();
      }
      trace2("put command:%p",*((void**)&cmd));
      outqueue.Push(cmd,cmd->get_priority());
    }
    smppSocket->notifyOutThread();
  }
  virtual SmscCommand getCommand()
  {
    MutexGuard g(mutexin);
    SmscCommand cmd;
    inqueue.Shift(cmd);
    trace2("get command:%p",*((void**)&cmd));
    return cmd;
  }

  void putIncomingCommand(const SmscCommand& cmd)
  {
    mutexin.Lock();
    if(!opened)
    {
      mutex.Unlock();
      return;
    }
    if(inqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
    {
      mutex.Unlock();
      throw ProxyQueueLimitException();
    }
    if(!CheckValidOutgoingCmd(cmd))
    {
      putCommand
      (
        SmscCommand::makeGenericNack
        (
          cmd->get_dialogId(),
          SmppStatusSet::ESME_RINVBNDSTS
        )
      );
    }
    inqueue.Push(cmd);
    mutexin.Unlock();
    managerMonitor->Signal();
  }
  SmscCommand getOutgoingCommand()
  {
    mutexout.Lock();
    SmscCommand cmd;
    outqueue.Pop(cmd);
    mutexout.Unlock();
    return cmd;
  }

  bool hasOutput()
  {
    MutexGuard g(mutexout);
    return outqueue.Count()!=0;
  }

  virtual SmeProxyState getState()const
  {
    return state;
  }
  void init()
  {
    managerMonitor=NULL;
    state=VALID;
  }
  virtual SmeProxyPriority getPriority()const{return SmeProxyPriorityDefault;}
  bool hasInput()const
  {
    MutexGuard g(mutexin);
    return inqueue.Count()!=0;
  }
  virtual void attachMonitor(ProxyMonitor* mon)
  {
    managerMonitor=mon;
  }
  virtual bool attached()
  {
    return managerMonitor!=NULL;
  }

  uint32_t getNextSequenceNumber()
  {
    MutexGuard g(mutex);
    return seq++;
  }

  void setProxyType(int newtype)
  {
    proxyType=newtype;
  }

  void setId(const std::string newid)
  {
    id=newid;
  }

  const char* getSystemId()const{return id.c_str();}

  bool isOpened()
  {
    return opened;
  }

protected:
  mutable Mutex mutex,mutexin,mutexout;
  std::string id;
  smsc::core::buffers::Array<SmscCommand> inqueue;
  smsc::core::buffers::PriorityQueue<SmscCommand,Array<SmscCommand>,0,31> outqueue;
  int seq;
  int proxyType;
  bool opened;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  SmppSocket *smppSocket;
};

bool SmppProxy::CheckValidIncomingCmd(const SmscCommand& cmd)
{
  switch(proxyType)
  {
    case proxyReceiver:
      switch(cmd->get_commandId())
      {
        case DELIVERY:
        case GENERIC_NACK:
        case UNBIND_RESP:
          return true;
        default:
          return false;
      }
    case proxyTranmitter:
      switch(cmd->get_commandId())
      {
        case GENERIC_NACK:
        case SUBMIT_RESP:
        case UNBIND_RESP:
        case CANCEL_RESP:
        case QUERY_RESP:
        case REPLACE_RESP:
        case UNBIND:
          return true;
        default:
          return false;
      }
    case proxyTransceiver:
      switch(cmd->get_commandId())
      {
        case GENERIC_NACK:
        case SUBMIT_RESP:
        case UNBIND_RESP:
        case CANCEL_RESP:
        case QUERY_RESP:
        case REPLACE_RESP:
        case UNBIND:
        case DELIVERY_RESP:
          return true;
        default:
          return false;
      }
  }
  return false;
}

bool SmppProxy::CheckValidOutgoingCmd(const SmscCommand& cmd)
{
  switch(proxyType)
  {
    case proxyReceiver:
      switch(cmd->get_commandId())
      {
        case UNBIND:
        case UNBIND_RESP:
        case DELIVERY_RESP:
        case GENERIC_NACK:
          return true;
        default:
          return false;
      }
    case proxyTranmitter:
      switch(cmd->get_commandId())
      {
        case SUBMIT:
        case GENERIC_NACK:
        case CANCEL:
        case REPLACE:
        case QUERY:
        case UNBIND:
        case UNBIND_RESP:
          return true;
        default:
          return false;
      }
    case proxyTransceiver:return true;
  }
  return true;
}


};//smppio
};//system
};//smsc


#endif
