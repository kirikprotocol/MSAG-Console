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
#include <map>
#include "util/Exception.hpp"
#include "util/timeslotcounter.hpp"

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
  SmppProxy(SmppSocket* sock,int limit,int procLimit,int timeout):smppReceiverSocket(sock),smppTransmitterSocket(sock),
    shapeCounterIn(60,500),shapeCounterOut(60,500)
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
    totalLimit=procLimit?procLimit:timeout*100;
    if(totalLimit<200)totalLimit=200;
    //if(totalLimit>1000)totalLimit=1000;
    //limit*timeout/2;
    submitLimit=totalLimit/2;
    submitCount=0;
    processLimit=procLimit;
    processTimeout=timeout;
    inQueueCount=0;
    disconnecting=false;
    disconnectionStart=0;
    unbinding=false;
    log=smsc::logger::Logger::getInstance("smppprx");
    info2(log,"SmppProxy: processLimit=%d, processTimeout=%u",processLimit,processTimeout);
    limitQueueSize=0;
  }
  virtual ~SmppProxy(){}

  bool deleteOnUnregister(){return true;}

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
    __trace__("put command:enter");
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
    bool wasEmpty=false;
    {
      MutexGuard g(mutexout);
      if(!opened)return;

      bool ussdSession=false;
      if(cmd->get_commandId()==DELIVERY)
      {
        SMS& sms=*cmd->get_sms();
        if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
        {
          if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=(uint32_t)USSD_PSSR_IND )
          {
            ussdSession=true;
          }
        }
      }
      bool outOfLimitCmd=cmd->get_commandId()==SUBMIT_RESP || cmd->get_commandId()==ALERT_NOTIFICATION;
      if(!ussdSession && !outOfLimitCmd && outqueue.Count()>=totalLimit)
      {
        throw ProxyQueueLimitException(outqueue.Count(),totalLimit);
      }
      if(!ussdSession && !outOfLimitCmd && shapeLimit>0 && shapeCounterOut.Get()>shapeLimit)
      {
        throw ProxyQueueLimitException(shapeCounterOut.Get(),shapeLimit);
      }
      if(cmd->get_commandId()==DELIVERY)
      {
        checkProcessLimit(cmd, ussdSession);//can throw ProxyQueueLimitException
      }
      if(shapeLimit>0)
      {
        shapeCounterOut.Inc(1);
      }
      debug2(log,"put command:total %d commands",outqueue.Count());
      if(outqueue.Count()==0)
      {
        wasEmpty=true;
      }
      outqueue.Push(cmd,cmd->get_priority());
    }
    if(wasEmpty)
    {
      volatile SmppSocket * rSck=0;
      volatile SmppSocket * tSck=0;
      {
        MutexGuard g(mutex);
        if(dualChannel)
        {
          if(smppReceiverSocket)rSck=smppReceiverSocket;
          if(smppTransmitterSocket)tSck=smppTransmitterSocket;
        }else
        {
          if(smppReceiverSocket)rSck=smppReceiverSocket;
        }
      }
      if(rSck)rSck->notifyOutThread();
      if(tSck)tSck->notifyOutThread();
    }
  }
  virtual bool getCommand(SmscCommand& cmd)
  {
    if(!inqueue.Pop(cmd))return false;
    debug2(log,"get command:%p",*((void**)&cmd));
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
      if(cmd.IsOk())
      {
        cmd.setProxy(prx);
        cmds.push_back(cmd);
        mx++;
      }
      if(cmd->get_commandId()==SUBMIT)submitCount--;
    }
    return inQueueCount;
  }

  void putIncomingCommand(const SmscCommand& cmd,int ct)
  {
    if(!CheckValidOutgoingCmd(cmd) || disconnecting)
    {
      debug2(log,"SmppProxy::putIncomingCommand: command for invalid bind state: %d",cmd->get_commandId());
      SmscCommand errresp;
      switch(cmd->get_commandId())
      {
        case DELIVERY:
          errresp=SmscCommand::makeDeliverySmResp("",cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case SUBMIT:
          errresp=SmscCommand::makeSubmitSmResp("",cmd->get_dialogId(),Status::INVBNDSTS,cmd->get_sms()->getIntProperty(Tag::SMPP_DATA_SM));
          break;
        case SUBMIT_MULTI_SM:
          errresp=SmscCommand::makeSubmitMultiResp("",cmd->get_dialogId(),Status::INVBNDSTS);
          break;
        case QUERY:
          errresp=SmscCommand::makeQuerySmResp(cmd->get_dialogId(),Status::INVBNDSTS,0,0,0,0);
          break;
        case UNBIND:
          errresp=SmscCommand::makeUnbindResp(cmd->get_dialogId(),Status::INVBNDSTS,cmd->get_mode());
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
      debug2(log,"SmppProxy::putIncomingCommand: error answer cmdid=%d",errresp->get_commandId());
      {
        MutexGuard g(mutexout);
        if(!opened)return;
        outqueue.Push(errresp,errresp->get_priority());
      }
      volatile SmppSocket * rSck=0;
      volatile SmppSocket * tSck=0;
      {
        MutexGuard g(mutex);
        if(dualChannel)
        {
          if(smppReceiverSocket)rSck=smppReceiverSocket;
          if(smppTransmitterSocket)tSck=smppTransmitterSocket;
        }else
        {
          if(smppReceiverSocket)rSck=smppReceiverSocket;
        }
      }
      if(rSck)rSck->notifyOutThread();
      if(tSck)tSck->notifyOutThread();
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
      if(!opened)return;
      MutexGuard g(mutexout);
      outqueue.Push(cmd,SmscCommandDefaultPriority);
    }
    else
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
          if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=(uint32_t)USSD_PSSR_IND &&
             !(
               sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==(uint32_t)USSD_USSR_REQ &&
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
        if(!ussdSession && shapeLimit>0 && shapeCounterIn.Get()>shapeLimit)
        {
          debug2(log,"Shaping limit exceeded for sme '%s' - %d/%d",id.c_str(),shapeCounterIn.Get(),shapeLimit);
          throw ProxyShapeLimitException(shapeCounterIn.Get(),shapeLimit);
        }
        if(shapeLimit>0)
        {
          shapeCounterIn.Inc(1);
        }

        if(processLimit!=0 && sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
        {
          MutexGuard mg(mutex);
          UssdSessionKey key(sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE),sms.getDestinationAddress());
          debug2(log,"SmppProxy::trying to find ussd session for %s/%d",key.oa.toString().c_str(),key.mr);
          UssdSessionMap::iterator it=ussdSessionMap.find(key);
          if(it!=ussdSessionMap.end())
          {
            if(it->second->cmd.IsOk())
            {
              info2(log,"SmppProxy::session %s/%d found, delayed response sent",key.oa.toString().c_str(),key.mr);
              inqueue.Push(it->second->cmd);
              inQueueCount++;
              limitHash.Delete(it->second->cmd->get_dialogId());
              limitQueue.erase(it->second);
              limitQueueSize--;
              ussdSessionMap.erase(it);
            }else
            {
              it->second->noDelay=true;
            }
          }else
          {
            warn2(log,"SmppProxy::session %s/%d not found!!!",key.oa.toString().c_str(),key.mr);
          }
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
        if(processResponse(cmd))
        {
          debug1(log,"delivery resp on ussd op preserved till corresponding submit");
          //delivery resp on ussd op preserved till corresponding submit
          return;
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
      if(cmd->intData==ctReceiver)
      {
        cmdMode=1;
      }else if(cmd->intData==ctTransmitter)
      {
        cmdMode=2;
      }

    }else if(cmdid==SMPP_PDU)
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
    MutexGuard g(seqMutex);
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
        throw smsc::util::Exception("Transsceiver (%s) cannot be upgraded",id.c_str());
      }
    }else
    {
      if(refcnt>0) throw smsc::util::Exception("Attempt to rebind single channel proxy (%s)",id.c_str());
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

  uint32_t getSmeFlags()const
  {
    return smeFlags;
  }

  void setSmeFlags(uint32_t val)
  {
    smeFlags=val;
  }

  bool isDualChannel()
  {
    return dualChannel;
  }

  void updateSmeInfo(const SmeInfo& info)
  {
    processLimit=info.proclimit;
    processTimeout=info.timeout;
    shapeLimit=info.schedlimit;
    smeFlags=info.flags;
  }

protected:
  smsc::logger::Logger* log;
  mutable Mutex mutex,mutexin,mutexout;
  std::string id;
  SmeIndex smeIndex;
  //smsc::core::buffers::Array<SmscCommand> inqueue;
  smsc::core::buffers::FastMTQueue<SmscCommand> inqueue;
  int inQueueCount;
  smsc::core::buffers::PriorityQueue<SmscCommand,smsc::core::buffers::CyclicQueue<SmscCommand>,0,31> outqueue;
  uint32_t smeFlags;

  struct UssdSessionKey{
    uint16_t mr;
    Address oa;
    UssdSessionKey():mr(0){}
    UssdSessionKey(uint16_t mr,const Address& oa):mr(mr),oa(oa){}
    bool operator<(const UssdSessionKey& rhs)const
    {
      return mr<rhs.mr || (mr==rhs.mr && oa<rhs.oa);
    }
  };

  struct ControlItem{
    time_t submitTime;
    int seqNum;
    bool ussd;
    bool noDelay;
    UssdSessionKey key;
    SmscCommand cmd;
    ControlItem(time_t t,int seq):submitTime(t),seqNum(seq),ussd(false),noDelay(false)
    {
    }
  };


  typedef std::list<ControlItem> LimitQueue;
  LimitQueue limitQueue;
  int limitQueueSize;
  typedef std::map<UssdSessionKey,LimitQueue::iterator> UssdSessionMap;
  UssdSessionMap ussdSessionMap;
  smsc::core::buffers::IntHash<LimitQueue::iterator> limitHash;
  int processLimit;
  int processTimeout;

  void checkProcessLimit(const SmscCommand& cmd, bool ussdSession )
  {
    if(processLimit==0)return;
    MutexGuard g(mutex);
    time_t now=time(NULL);
    while(!limitQueue.empty() && limitQueue.begin()->submitTime+processTimeout<now)
    {
      limitHash.Delete(limitQueue.begin()->seqNum);
      if(limitQueue.begin()->ussd)
      {
        ussdSessionMap.erase(limitQueue.begin()->key);
      }
      limitQueue.erase(limitQueue.begin());
      limitQueueSize--;
    }
    if(!ussdSession && limitQueueSize>=processLimit)throw ProxyQueueLimitException(processLimit,limitQueueSize);
    ControlItem ci(now,cmd->get_dialogId());
    bool ussd=false;
    if(cmd->get_commandId()==DELIVERY && cmd->get_sms()->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
    {
      debug2(log,"SmppProxy:: ussd session detected oa/mr=%s/%d",cmd->get_sms()->getOriginatingAddress().toString().c_str(),cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      ci.ussd=true;
      ci.key=UssdSessionKey(cmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE),cmd->get_sms()->getOriginatingAddress());
      ussd=true;
    }
    limitQueue.push_back(ci);
    limitQueueSize++;
    LimitQueue::iterator it=--limitQueue.end();
    limitHash.Insert(ci.seqNum,it);
    if(ussd)ussdSessionMap.insert(UssdSessionMap::value_type(ci.key,it));
  }

  bool processResponse(const SmscCommand& cmd)
  {
    if(processLimit==0)return false;
    MutexGuard g(mutex);
    LimitQueue::iterator *ptr=limitHash.GetPtr(cmd->get_dialogId());
    if(ptr)
    {
      if(!(*ptr)->ussd)
      {
        limitQueue.erase(*ptr);
        limitQueueSize--;
        limitHash.Delete(cmd->get_dialogId());
        return false;
      }else
      {
        debug2(log,"ussd session found for dlgId=%d, nd=%c, status=%d",cmd->get_dialogId(),(*ptr)->noDelay?'Y':'N',cmd->get_resp()->get_status());
        if((*ptr)->noDelay || cmd->get_resp()->get_status()!=0)
        {
          ussdSessionMap.erase((*ptr)->key);
          limitQueue.erase(*ptr);
          limitQueueSize--;
          limitHash.Delete(cmd->get_dialogId());
          return false;
        }else
        {
          (*ptr)->cmd=cmd;
          return true;
        }
      }
    }else
    {
      debug2(log,"record not found for dlgId=%d",cmd->get_dialogId());
    }
    return false;
  }

  volatile int seq;
  Mutex seqMutex;
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

  smsc::util::TimeSlotCounter<> shapeCounterIn,shapeCounterOut;
  int shapeLimit;

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
    default:break;
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
      break;
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
      break;
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
      break;
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
    default:break;
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
      break;
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
      break;
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
      break;
  }
  return false;
}


}//smppio
}//system
}//smsc


#endif
