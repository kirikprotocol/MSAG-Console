#ifndef __SYSTEM_SMPPIO_SMPPPROXY_HPP__
#define __SYSTEM_SMPPIO_SMPPPROXY_HPP__

#include "smeman/smeproxy.h"
#include "core/buffers/Array.hpp"
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
  }
  virtual ~SmppProxy(){}
  virtual void close()
  {

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
          SmppStatusSet::ESME_RINVCMDID
        )
      );
    }
    {
      MutexGuard g(mutexout);
      if(outqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
      {
        throw ProxyQueueLimitException();
      }
      trace2("put command:%p",*((void**)&cmd));
      outqueue.Push(cmd);
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
    if(inqueue.Count()==SMPP_PROXY_QUEUE_LIMIT)
    {
      mutex.Unlock();
      throw ProxyQueueLimitException();
    }
    inqueue.Push(cmd);
    mutexin.Unlock();
    managerMonitor->Signal();
  }
  SmscCommand getOutgoingCommand()
  {
    mutexout.Lock();
    SmscCommand cmd;
    outqueue.Shift(cmd);
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

  std::string getId(){return id;}

protected:
  mutable Mutex mutex,mutexin,mutexout;
  std::string id;
  smsc::core::buffers::Array<SmscCommand> inqueue,outqueue;
  int seq;
  int proxyType;
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
        case SUBMIT_RESP:
        case DELIVERY_RESP:
        case SUBMIT:
        case QUERY:
        default:
          return false;
      }
    case proxyTranmitter:
      switch(cmd->get_commandId())
      {
        case GENERIC_NACK:
        case SUBMIT_RESP:
        case UNBIND_RESP:
          return true;
        case DELIVERY:
        case DELIVERY_RESP:
        case SUBMIT:
        case QUERY:
        default:
          return false;
      }
    case proxyTransceiver:return true;
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
        case DELIVERY_RESP:
        case GENERIC_NACK:
          return true;
        case SUBMIT_RESP:
        case DELIVERY:
        case SUBMIT:
        case QUERY:
        case UNBIND_RESP:
        default:
          return false;
      }
    case proxyTranmitter:
      switch(cmd->get_commandId())
      {
        case SUBMIT:
        case GENERIC_NACK:
        case DELIVERY_RESP:
          return true;
        case DELIVERY:
        case SUBMIT_RESP:
        case QUERY:
        case UNBIND_RESP:
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
