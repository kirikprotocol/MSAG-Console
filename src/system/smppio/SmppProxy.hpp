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
#include "util/Exception.hpp"

namespace smsc{
namespace system{
namespace smppio{

#define SMPP_PROXY_QUEUE_LIMIT 4096

using namespace smsc::smeman;
using namespace smsc::core::synchronization;
using smsc::util::Exception;

const int proxyTransmitter=smeTX;
const int proxyReceiver=smeRX;
const int proxyTransceiver=smeTRX;


class SmppProxy:public SmeProxy{
public:
  SmppProxy(SmppSocket* sock):smppReceiverSocket(sock),smppTransmitterSocket(sock)
  {
    smppReceiverSocket->assignProxy(this);
    seq=1;
    refcnt=2;
    managerMonitor=NULL;
    proxyType=proxyTransceiver;
    opened=true;
  }
  virtual ~SmppProxy(){}
  virtual void close()
  {
    opened=false;
  }
  virtual void disconnect()
  {
    MutexGuard g(mutex);
    if(smppReceiverSocket)smppReceiverSocket->getSocket()->Close();
    if(smppTransmitterSocket)smppTransmitterSocket->getSocket()->Close();
  }
  inline bool CheckValidIncomingCmd(const SmscCommand& cmd);
  inline bool CheckValidOutgoingCmd(const SmscCommand& cmd);


  virtual void putCommand(const SmscCommand& cmd)
  {
    trace("put command:enter");
    if(!CheckValidIncomingCmd(cmd))
    {
      /*putIncomingCommand
      (
        SmscCommand::makeGenericNack
        (
          cmd->get_dialogId(),
          SmppStatusSet::ESME_RINVBNDSTS
        )
      );*/
      __warning2__("SmppProxy::putCommand: command is invalid for bindstate:%d",cmd->get_commandId());
      throw InvalidProxyCommandException();
      return;
    }
    {
      MutexGuard g(mutexout);
      if(!opened)return;
      if(cmd->get_commandId()!=SUBMIT_RESP && outqueue.Count()>=SMPP_PROXY_QUEUE_LIMIT)
      {
        throw ProxyQueueLimitException();
      }
      trace2("put command:total %d commands",outqueue.Count());
      outqueue.Push(cmd,cmd->get_priority());
    }
    if(smppReceiverSocket)smppReceiverSocket->notifyOutThread();
    if(smppTransmitterSocket)smppTransmitterSocket->notifyOutThread();
  }
  virtual SmscCommand getCommand()
  {
    MutexGuard g(mutexin);
    if(inqueue.Count()==0)throw Exception("SmppProxy::getCommand: no commands in input queue");
    SmscCommand cmd;
    inqueue.Shift(cmd);
    trace2("get command:%p",*((void**)&cmd));
    return cmd;
  }

  void putIncomingCommand(const SmscCommand& cmd)
  {
    if(!CheckValidOutgoingCmd(cmd))
    {
      __trace2__("SmppProxy::putIncomingCommand: command for invalid bind state: %d",cmd->get_commandId());
      SmscCommand errresp;
      switch(cmd->get_commandId())
      {
        case DELIVERY:
          errresp=SmscCommand::makeDeliverySmResp("",cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case SUBMIT:
          errresp=SmscCommand::makeSubmitSmResp("",cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case SUBMIT_MULTI_SM:
          errresp=SmscCommand::makeSubmitMultiResp("",cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case QUERY:
          errresp=SmscCommand::makeQuerySmResp(cmd->get_dialogId(),Status::INVBNDSTS,0,0,0,0);
          break;
        case UNBIND:
          errresp=SmscCommand::makeUnbindResp(cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case REPLACE:
          errresp=SmscCommand::makeReplaceSmResp(cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case CANCEL:
          errresp=SmscCommand::makeCancelSmResp(cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        default:
          errresp=SmscCommand::makeGenericNack(cmd->get_dialogId(),Status::INVBNDSTS);
      }
      //cmd->get_dialogId(),SmppStatusSet::ESME_RINVBNDSTS
      //putCommand(errresp);
      __trace2__("SmppProxy::putIncomingCommand: error answer cmdid=%d",errresp->get_commandId());
      {
        MutexGuard g(mutexout);
        if(!opened)return;
        outqueue.Push(errresp,errresp->get_priority());
      }
      if(smppReceiverSocket)smppReceiverSocket->notifyOutThread();
      if(smppTransmitterSocket)smppTransmitterSocket->notifyOutThread();
      return;
    }
    /*if(cmd->get_commandId()==ENQUIRELINK)
    {
      MutexGuard g(mutexout);
      if(!opened)return;
      outqueue.Push
      (
        SmscCommand::makeCommand
        (
          ENQUIRELINK_RESP,
          cmd->get_dialogId(),
          Status::OK
        )
        ,
        SmscCommandDefaultPriority
      );
    }else */
    if(cmd->get_commandId()==SMPP_PDU)
    {
      MutexGuard g(mutexout);
      if(!opened)return;
      outqueue.Push(cmd,SmscCommandDefaultPriority);
    }
    else
    {
      MutexGuard g(mutexin);
      if(!opened)
      {
        return;
      }
      if(inqueue.Count()>=SMPP_PROXY_QUEUE_LIMIT)
      {
        throw ProxyQueueLimitException();
      }
      inqueue.Push(cmd);
    }
    if(managerMonitor)managerMonitor->Signal();
  }
  SmscCommand getOutgoingCommand()
  {
    MutexGuard g(mutexout);
    SmscCommand cmd;
    outqueue.Pop(cmd);
    return cmd;
  }

  bool hasOutput(int ct)
  {
    MutexGuard g(mutexout);
    if(outqueue.Count()==0)return false;
    if(ct==ctReceiver)
    {
      SmscCommand cmd;
      if(outqueue.Peek(cmd))
      {
        int cmdid=cmd->get_commandId();
        __trace2__("check output for receiver:cmdid=%d",cmdid);
        return !(
                 cmdid==SUBMIT_RESP ||
                 cmdid==SUBMIT_MULTI_SM_RESP ||
                 cmdid==CANCEL_RESP ||
                 cmdid==QUERY_RESP ||
                 cmdid==REPLACE_RESP
               );
      }
    }else if(ct==ctTransmitter)
    {
      SmscCommand cmd;
      if(outqueue.Peek(cmd))
      {
        outqueue.Peek(cmd);
        int cmdid=cmd->get_commandId();
        __trace2__("check output for transmitter:cmdid=%d",cmdid);
        return (
                 cmdid==SUBMIT_RESP ||
                 cmdid==SUBMIT_MULTI_SM_RESP ||
                 cmdid==CANCEL_RESP ||
                 cmdid==QUERY_RESP ||
                 cmdid==REPLACE_RESP
               );
      }
    }
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

  void setId(const std::string& newid,SmeIndex idx)
  {
    id=newid;
    smeIndex=idx;
  }

  const char* getSystemId()const{return id.c_str();}

  bool isOpened()
  {
    return opened;
  }

  void AddRef()
  {
    MutexGuard g(mutex);
    refcnt+=2;
  }

  int Unref(int ct)
  {
    MutexGuard g(mutex);
    if(refcnt>2)
    {
      if(ct==ctReceiver && proxyType==proxyTransceiver)
      {
        proxyType=proxyTransmitter;
        __trace2__("SmppProxy: downgrade to transmitter");
      }else if(ct==ctTransmitter && proxyType==proxyTransceiver)
      {
        __trace2__("SmppProxy: downgrade to receiver");
        proxyType=proxyReceiver;
      }
    }
    int cnt=--refcnt;
    if(refcnt==1)close();
    return cnt;
  }


  int getSmeIndex()
  {
    return smeIndex;
  }

  int getBindMode()
  {
    return proxyType;
  }

  bool getPeers(char* in,char* out)
  {
    MutexGuard mg(mutex);
    if(!smppReceiverSocket && !smppTransmitterSocket)return false;
    if(smppReceiverSocket)smppReceiverSocket->getSocket()->GetPeer(in);
    else in[0]=0;
    if(smppTransmitterSocket)smppTransmitterSocket->getSocket()->GetPeer(out);
    else out[0]=0;
    return in[0]!=0 || out[0]!=0;
  }

  void setReceiverSocket(SmppSocket* ss)
  {
    smppReceiverSocket=ss;
  }
  void setTransmitterSocket(SmppSocket* ss)
  {
    smppTransmitterSocket=ss;
  }

protected:
  mutable Mutex mutex,mutexin,mutexout;
  std::string id;
  SmeIndex smeIndex;
  smsc::core::buffers::Array<SmscCommand> inqueue;
  smsc::core::buffers::PriorityQueue<SmscCommand,Array<SmscCommand>,0,31> outqueue;
  int seq;
  int proxyType;
  bool opened;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  SmppSocket *smppReceiverSocket;
  SmppSocket *smppTransmitterSocket;
  int refcnt;
};

bool SmppProxy::CheckValidIncomingCmd(const SmscCommand& cmd)
{
  switch(cmd->get_commandId())
  {
    case ENQUIRELINK:
    case ENQUIRELINK_RESP:
    case UNBIND:
    case UNBIND_RESP:
    case GENERIC_NACK:
    case SMPP_PDU:
      return true;
  }

  switch(proxyType)
  {
    case proxyReceiver:
      switch(cmd->get_commandId())
      {
        case DELIVERY:
          return true;
        default:
          return false;
      }
    case proxyTransmitter:
      switch(cmd->get_commandId())
      {
        case SUBMIT_RESP:
        case SUBMIT_MULTI_SM_RESP:
        case CANCEL_RESP:
        case QUERY_RESP:
        case REPLACE_RESP:
          return true;
        default:
          return false;
      }
    case proxyTransceiver:
      switch(cmd->get_commandId())
      {
        case SUBMIT_RESP:
        case SUBMIT_MULTI_SM_RESP:
        case CANCEL_RESP:
        case QUERY_RESP:
        case REPLACE_RESP:
        case DELIVERY:
          return true;
        default:
          return false;
      }
  }
  return false;
}

bool SmppProxy::CheckValidOutgoingCmd(const SmscCommand& cmd)
{
  switch(cmd->get_commandId())
  {
    case ENQUIRELINK:
    case ENQUIRELINK_RESP:
    case UNBIND:
    case UNBIND_RESP:
    case GENERIC_NACK:
    case SMPP_PDU:
      return true;
  }
  switch(proxyType)
  {
    case proxyReceiver:
      switch(cmd->get_commandId())
      {
        case DELIVERY_RESP:
          return true;
        default:
          return false;
      }
    case proxyTransmitter:
      switch(cmd->get_commandId())
      {
        case SUBMIT:
        case SUBMIT_MULTI_SM:
        case CANCEL:
        case REPLACE:
        case QUERY:
          return true;
        default:
          return false;
      }
    case proxyTransceiver:
      switch(cmd->get_commandId())
      {
        case SUBMIT:
        case SUBMIT_MULTI_SM:
        case CANCEL:
        case REPLACE:
        case QUERY:
        case DELIVERY_RESP:
          return true;
        default:
          return false;
      }
  }
  return false;
}


};//smppio
};//system
};//smsc


#endif
