#ifndef __SYSTEM_SMPPIO_SMPPPROXY_HPP__
#define __SYSTEM_SMPPIO_SMPPPROXY_HPP__

#include "smeman/smeproxy.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include <list>
#include "core/buffers/PriorityQueue.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "system/smppio/SmppSocket.hpp"
#include "smeman/smsccmd.h"
#include <string>
#include "util/Exception.hpp"

namespace smsc{
namespace system{
namespace smppio{

using namespace smsc::smeman;
using namespace smsc::core::synchronization;
using smsc::util::Exception;

const int proxyTransmitter=smeTX;
const int proxyReceiver=smeRX;
const int proxyTransceiver=smeTRX;


class SmppProxy:public SmeProxy{
public:
  SmppProxy(SmppSocket* sock,int limit,int procLimit,int timeout):smppReceiverSocket(sock),smppTransmitterSocket(sock)
  {
    smppReceiverSocket->assignProxy(this);
    dualChannel=false;
    seq=1;
    refcnt=2;
    rxcnt=0;
    txcnt=0;
    managerMonitor=NULL;
    proxyType=proxyTransceiver;
    opened=true;
    if(timeout==0)timeout=8;
    totalLimit=procLimit*5;
    if(totalLimit<200)totalLimit=200;
    //limit*timeout/2;
    submitLimit=totalLimit/2;
    submitCount=0;
    processLimit=procLimit;
    processTimeout=timeout;
    inQueueCount=0;
    disconnecting=false;
    disconnectionStart=0;
    unbinding=false;
    __trace2__("SmppProxy: processLimit=%d, processTimeout=%u",processLimit,processTimeout);
  }
  virtual ~SmppProxy(){}
  virtual void close()
  {
    opened=false;
  }
  virtual void disconnect()
  {
    if(disconnecting)return;
    //if(smppReceiverSocket)smppReceiverSocket->getSocket()->Close();
    //if(smppTransmitterSocket)smppTransmitterSocket->getSocket()->Close();
    disconnecting=true;
    disconnectionStart=time(NULL);
    MutexGuard g(mutexout);
    if(!dualChannel)
    {
      outqueue.Push(SmscCommand::makeUnbind(getNextSequenceNumber()),0);
    }else
    {
      outqueue.Push(SmscCommand::makeUnbind(getNextSequenceNumber(),1),0);
      outqueue.Push(SmscCommand::makeUnbind(getNextSequenceNumber(),2),0);
    }
  }

  bool isDisconnecting()
  {
    return disconnecting;
  }

  time_t getDisconnectTime()
  {
    return disconnectionStart;
  }

  void Unbind()
  {
    MutexGuard mg(mutex);
    unbinding=true;
  }

  bool isUnbinding()
  {
    MutexGuard mg(mutex);
    return unbinding;
  }

  inline bool CheckValidIncomingCmd(const SmscCommand& cmd);
  inline bool CheckValidOutgoingCmd(const SmscCommand& cmd);


  virtual void putCommand(const SmscCommand& cmd)
  {
    trace("put command:enter");
    if(disconnecting)
    {
      throw smsc::util::Exception("proxy is disconnecting");
    }
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
      bool ussdSession=false;
      if(cmd->get_commandId()==DELIVERY)
      {
        SMS& sms=*cmd->get_sms();
        if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
        {
          if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=USSD_PSSR_IND &&
             !(
               sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==USSD_USSR_REQ &&
               sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
              )
            )
          {
            ussdSession=true;
          }
        }
      }
      if(!ussdSession && cmd->get_commandId()!=SUBMIT_RESP && outqueue.Count()>=totalLimit)
      {
        throw ProxyQueueLimitException(outqueue.Count(),totalLimit);
      }
      if(!ussdSession && cmd->get_commandId()==DELIVERY)
      {
        checkProcessLimit(cmd);//can throw ProxyQueueLimitException
      }
      trace2("put command:total %d commands",outqueue.Count());
      outqueue.Push(cmd,cmd->get_priority());
    }
    {
      MutexGuard g(mutex);
      if(dualChannel)
      {
        if(smppReceiverSocket)smppReceiverSocket->notifyOutThread();
        if(smppTransmitterSocket)smppTransmitterSocket->notifyOutThread();
      }else
      {
        if(smppReceiverSocket)smppReceiverSocket->notifyOutThread();
      }
    }
  }
  virtual bool getCommand(SmscCommand& cmd)
  {
    if(!inqueue.Pop(cmd))return false;
    trace2("get command:%p",*((void**)&cmd));
    MutexGuard g(mutexin);
    inQueueCount--;
    if(cmd->get_commandId()==SUBMIT)submitCount--;
    return true;
  }
  virtual int getCommandEx(std::vector<SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    if(inQueueCount==0)
    {
      mx=0;
      return 0;
    }
    MutexGuard g(mutexin);
    int cnt=inQueueCount<mx?inQueueCount:mx;
    mx=0;
    SmscCommand cmd;
    for(int i=0;i<cnt;i++)
    {
      if(!inqueue.Pop(cmd))break;
      inQueueCount--;
      cmd.setProxy(prx);
      cmds.push_back(cmd);
      mx++;
      if(cmd->get_commandId()==SUBMIT)submitCount--;
    }
    return inQueueCount;
  }

  void putIncomingCommand(const SmscCommand& cmd,int ct)
  {
    if(!CheckValidOutgoingCmd(cmd) || disconnecting)
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
          errresp=SmscCommand::makeUnbindResp(cmd->get_dialogId(),Status::INVBNDSTS,cmd->dta);
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
      SmppHeader* pdu=errresp.makePdu(0);
      errresp=SmscCommand::makeSmppPduCommand(pdu,ct);
      //cmd->get_dialogId(),SmppStatusSet::ESME_RINVBNDSTS
      //putCommand(errresp);
      __trace2__("SmppProxy::putIncomingCommand: error answer cmdid=%d",errresp->get_commandId());
      {
        MutexGuard g(mutexout);
        if(!opened)return;
        outqueue.Push(errresp,errresp->get_priority());
      }
      {
        MutexGuard g(mutex);
        if(dualChannel)
        {
          if(smppReceiverSocket)smppReceiverSocket->notifyOutThread();
          if(smppTransmitterSocket)smppTransmitterSocket->notifyOutThread();
        }else
        {
          if(smppReceiverSocket)smppReceiverSocket->notifyOutThread();
        }
      }
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
      {
        MutexGuard g(mutexin);
        if(!opened)
        {
          return;
        }
        if(cmd->get_commandId()==SUBMIT)
        {
          bool ussdSession=false;
          SMS& sms=*cmd->get_sms();
          if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
          {
            if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=USSD_PSSR_IND &&
               !(
                 sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==USSD_USSR_REQ &&
                 sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
                )
              )
            {
              ussdSession=true;
            }
          }

          if(!ussdSession && submitCount>submitLimit)
          {
            throw ProxyQueueLimitException(submitCount,submitLimit);
          }
          submitCount++;
        }
        else
        if(inQueueCount>=totalLimit)
        {
          throw ProxyQueueLimitException(inQueueCount,totalLimit);
        }
        if(cmd->get_commandId()==DELIVERY_RESP)
        {
          processResponse(cmd);
        }
      }
      inqueue.Push(cmd);
      inQueueCount++;
    }
    if(managerMonitor)managerMonitor->Signal();
  }
  bool getOutgoingCommand(int ct,SmscCommand& cmd)
  {
    MutexGuard g(mutexout);
    if(outqueue.Count()==0)return false;
    if(!outqueue.Peek(cmd))return false;
    if(ct==-1)
    {
      //
    }
    int cmdMode=-1;
    int cmdid=cmd->get_commandId();

    if(cmdid==UNBIND)
    {
      if(cmd->get_mode()==1)
      {
        cmdMode=1;
      }else if(cmd->get_mode()==2)
      {
        cmdMode=2;
      }
    }
    else
    if((cmdid==ENQUIRELINK || cmdid==ENQUIRELINK_RESP || cmdid==UNBIND_RESP))
    {
      if(((int)cmd->dta)==ctReceiver)
      {
        cmdMode=1;
      }else if(((int)cmd->dta)==ctTransmitter)
      {
        cmdMode=2;
      }

    }if(cmdid==SMPP_PDU)
    {
      cmdMode=cmd->get_dialogId();//1 receiver 2 transmitter
    }
    else
    {
      if(!(
                 cmdid==SUBMIT_RESP ||
                 cmdid==SUBMIT_MULTI_SM_RESP ||
                 cmdid==CANCEL_RESP ||
                 cmdid==QUERY_RESP ||
                 cmdid==REPLACE_RESP
        ))
      {
        cmdMode=1;
      }
      if(
                   cmdid==SUBMIT_RESP ||
                   cmdid==SUBMIT_MULTI_SM_RESP ||
                   cmdid==CANCEL_RESP ||
                   cmdid==QUERY_RESP ||
                   cmdid==REPLACE_RESP ||
                   cmdid==ENQUIRELINK ||
                   cmdid==ENQUIRELINK_RESP ||
                   cmdid==UNBIND ||
                   cmdid==UNBIND_RESP ||
                   cmdid==GENERIC_NACK
        )
      {
        cmdMode=2;
      }
    }

    if((cmdMode==1 && ct==ctReceiver) || (cmdMode==2 && ct==ctTransmitter) || ct==ctTransceiver)
    {
      outqueue.Pop(cmd);
      return true;
    }
    if((cmdMode==1 && !smppReceiverSocket) || (cmdMode==2 && !smppTransmitterSocket))
    {
      outqueue.Pop(cmd);
    }
    return false;
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
    //MutexGuard g(mutexin);
    return inQueueCount!=0;
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
    MutexGuard g(mutex);
    proxyType=newtype;
    if(proxyType!=proxyTransceiver && !dualChannel)
    {
      dualChannel=true;
      if(newtype==proxyReceiver)rxcnt+=2;
      if(newtype==proxyTransmitter)txcnt+=2;
    }
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

  bool isAlive()
  {
    MutexGuard mg(mutex);
    return opened && !disconnecting && !unbinding;
  }

  void AddRef(int ct)
  {
    MutexGuard g(mutex);
    if(disconnecting || unbinding)throw smsc::util::Exception("AddRef failed: disconnecting or unbinding");
    if(dualChannel)
    {
      if(ct==proxyTransmitter)
      {
        if(txcnt!=0)throw smsc::util::Exception("Transmitter channel (%s) not ready for upgrade",id.c_str());
        txcnt+=2;
      }
      else if(ct==proxyReceiver)
      {
        if(rxcnt!=0)throw smsc::util::Exception("Receiver channel (%s) not ready for upgrade",id.c_str());
        rxcnt+=2;
      }else
      {
        throw smsc::util::Exception("Transsceiver cannot be upgraded");
      }
    }else
    {
      if(refcnt>0) throw smsc::util::Exception("Attempt to rebind single channel proxy");
    }
    refcnt+=2;
    __warning2__("SmppProxy::AddRef(%s): pt=%d; cnts=%d/%d/%d",id.c_str(),proxyType,refcnt,rxcnt,txcnt);
  }

  int Unref(int ct)
  {
    MutexGuard g(mutex);
    __warning2__("SmppProxy: unref(%s) %p;rc=%d/%d/%d;pt=%d;ct=%d",id.c_str(),this,refcnt,rxcnt,txcnt,proxyType,ct);
    if(ct!=-1)
    {
      if(dualChannel)
      {
        if(ct==ctReceiver)
        {
          if(proxyType==proxyTransceiver)
          {
            proxyType=proxyTransmitter;
            smppReceiverSocket=0;
            __warning2__("SmppProxy: downgrade(%s) %p to transmitter(rc=%d,pt=%d)",id.c_str(),this,refcnt,proxyType);
          }
          rxcnt--;
          if(rxcnt==0)unbinding=false;
        }else if(ct==ctTransmitter)
        {
          if(proxyType==proxyTransceiver)
          {
            proxyType=proxyReceiver;
            smppTransmitterSocket=0;
            __warning2__("SmppProxy: downgrade(%s) %p to receiver(rc=%d,pt=%d)",id.c_str(),this,refcnt,proxyType);
          }
          txcnt--;
          if(txcnt==0)unbinding=false;
        }
      }
    }

    if(dualChannel)
    {
      if(rxcnt!=2)smppReceiverSocket=0;
      if(txcnt!=2)smppTransmitterSocket=0;
    }
    else
    {
      smppReceiverSocket=0;
      smppTransmitterSocket=0;
    }

    int cnt=--refcnt;
    if(refcnt==1 || (rxcnt<=1 && txcnt<=1))
    {
      close();
      disconnecting=true;
      disconnectionStart=time(NULL);
    }

    if(refcnt<0 || rxcnt<0 || txcnt<0 || (dualChannel && refcnt!=rxcnt+txcnt))abort();

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
    if(!opened)return false;
    if(!smppReceiverSocket && !smppTransmitterSocket)return false;
    if(smppReceiverSocket && smppReceiverSocket->getSocket() ) smppReceiverSocket->getSocket()->GetPeer(in);
    else in[0]=0;
    if(smppTransmitterSocket && smppTransmitterSocket->getSocket() ) smppTransmitterSocket->getSocket()->GetPeer(out);
    else out[0]=0;
    return in[0]!=0 || out[0]!=0;
  }

  void setReceiverSocket(SmppSocket* ss)
  {
    MutexGuard g(mutex);
    smppReceiverSocket=ss;
    if(smppReceiverSocket)smppReceiverSocket->assignProxy(this);
  }
  void setTransmitterSocket(SmppSocket* ss)
  {
    MutexGuard g(mutex);
    smppTransmitterSocket=ss;
    if(smppTransmitterSocket)smppTransmitterSocket->assignProxy(this);
  }

  bool getForceDc()
  {
    return forceDc;
  }

  void setForceDc(bool val)
  {
    forceDc=val;
  }

protected:
  mutable Mutex mutex,mutexin,mutexout;
  std::string id;
  SmeIndex smeIndex;
  //smsc::core::buffers::Array<SmscCommand> inqueue;
  smsc::core::buffers::FastMTQueue<SmscCommand> inqueue;
  int inQueueCount;
  smsc::core::buffers::PriorityQueue<SmscCommand,smsc::core::buffers::CyclicQueue<SmscCommand>,0,31> outqueue;
  bool forceDc;

  struct ControlItem{
    time_t submitTime;
    int seqNum;
  };

  std::list<ControlItem> limitQueue;
  smsc::core::buffers::IntHash<std::list<ControlItem>::iterator> limitHash;
  int processLimit;
  int processTimeout;

  void checkProcessLimit(const SmscCommand& cmd)
  {
    if(processLimit==0)return;
    MutexGuard g(mutex);
    time_t now=time(NULL);
    while(!limitQueue.empty() && limitQueue.begin()->submitTime+processTimeout<now)
    {
      limitHash.Delete(limitQueue.begin()->seqNum);
      limitQueue.erase(limitQueue.begin());
    }
    if(limitQueue.size()==processLimit)throw ProxyQueueLimitException(processLimit,limitQueue.size());
    ControlItem ci={now,cmd->get_dialogId()};
    limitQueue.push_back(ci);
    limitHash.Insert(ci.seqNum,--limitQueue.end());
  }

  void processResponse(const SmscCommand& cmd)
  {
    if(processLimit==0)return;
    MutexGuard g(mutex);
    if(limitHash.Exist(cmd->get_dialogId()))
    {
      limitQueue.erase(limitHash.Get((int)cmd->get_dialogId()));
      limitHash.Delete(cmd->get_dialogId());
    }
  }

  volatile int seq;
  int proxyType;
  bool opened;
  SmeProxyState state;
  ProxyMonitor *managerMonitor;
  volatile SmppSocket *smppReceiverSocket;
  volatile SmppSocket *smppTransmitterSocket;
  volatile int refcnt;
  volatile int rxcnt;
  volatile int txcnt;
  int totalLimit;
  int submitLimit;
  int submitCount;

  bool disconnecting;
  time_t disconnectionStart;
  bool unbinding;
  bool dualChannel;
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
    case ALERT_NOTIFICATION:
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
    case SMEALERT:
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


}//smppio
}//system
}//smsc


#endif
