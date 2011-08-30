#ifndef __EYELINE_SME_CLUSTERSMPPSESSION_HPP__
#define __EYELINE_SME_CLUSTERSMPPSESSION_HPP__

#include "SmppBase.hpp"
#include <vector>

namespace smsc{
namespace sme{

enum{
  maxNumberOfNodes=16
};

class ClusterSmppSession{
public:
  ClusterSmppSession(const std::vector<SmeConfig>& configs,SmppPduEventListener* argLst):lst(argLst),strans(this),atrans(this),connThr(this),connCount(0)
  {
    reconnectDelay=5;
    seqCounter=0;
    for(std::vector<SmeConfig>::const_iterator it=configs.begin(),end=configs.end();it!=end;++it)
    {
      sessions.push_back(new SessionInfo(this,*it));
    }
  }
  void connect()
  {
    if(connCount>0)return;
    MutexGuard mg(connMon);
    if(!connThr.started)
    {
      connThr.Start();
    }
    while(connCount==0)
    {
      connMon.notify();
      connMon.wait();
    }
  }
  void close()
  {
    connThr.stop();
    connThr.WaitFor();
    MutexGuard mg(connMon);
    for(SVector::iterator it=sessions.begin(),end=sessions.end();it!=end;++it)
    {
      (*it)->ss.close();
    }

  }
  int getNextSeq()
  {
    MutexGuard g(cntMutex);
    return ++seqCounter;
  }

  SmppTransmitter* getSyncTransmitter()
  {
    return &strans;
  }
  SmppTransmitter* getAsyncTransmitter()
  {
    return &atrans;
  }

protected:
  SmppPduEventListener* lst;
  struct InnerListener:SmppPduEventListener{
    ClusterSmppSession* css;
    InnerListener(ClusterSmppSession* argCss):css(argCss){}
    void handleEvent(SmppHeader* pdu)
    {
      css->lst->handleEvent(pdu);
    }
    void handleError(int code);
  };
  friend struct InnerListener;
  struct ConnectThread:smsc::core::threads::Thread{
    ClusterSmppSession* css;
    volatile bool started;
    volatile bool stopRequest;
    ConnectThread(ClusterSmppSession* argCss):css(argCss),started(false),stopRequest(false){}
    int Execute();
    void stop()
    {
      MutexGuard mg(css->connMon);
      stopRequest=true;
      css->connMon.notifyAll();
    }
  };
  friend struct ConnectThread;

  class SyncTransmitterImpl:public SmppTransmitter{
  public:
    ClusterSmppSession* css;
    SyncTransmitterImpl(ClusterSmppSession* argCss):css(argCss){}
    virtual SmppHeader* sendPdu(SmppHeader* pdu);
    virtual void sendGenericNack(PduGenericNack& pdu);
    virtual void sendDeliverySmResp(PduDeliverySmResp& pdu);
    virtual void sendDataSmResp(PduDataSmResp& pdu);
    virtual PduSubmitSmResp* submit(PduSubmitSm& pdu);
    virtual PduMultiSmResp* submitm(PduMultiSm& pdu);
    virtual PduDataSmResp* data(PduDataSm& pdu);
    virtual PduQuerySmResp* query(PduQuerySm& pdu);
    virtual PduCancelSmResp* cancel(PduCancelSm& pdu);
    virtual PduReplaceSmResp* replace(PduReplaceSm& pdu);
  };
  SyncTransmitterImpl strans;
  friend class SyncTransmitterImpl;
  class ASyncTransmitterImpl:public SmppTransmitter{
  public:
    ClusterSmppSession* css;
    ASyncTransmitterImpl(ClusterSmppSession* argCss):css(argCss){}
    virtual SmppHeader* sendPdu(SmppHeader* pdu);
    virtual void sendGenericNack(PduGenericNack& pdu);
    virtual void sendDeliverySmResp(PduDeliverySmResp& pdu);
    virtual void sendDataSmResp(PduDataSmResp& pdu);
    virtual PduSubmitSmResp* submit(PduSubmitSm& pdu);
    virtual PduMultiSmResp* submitm(PduMultiSm& pdu);
    virtual PduDataSmResp* data(PduDataSm& pdu);
    virtual PduQuerySmResp* query(PduQuerySm& pdu);
    virtual PduCancelSmResp* cancel(PduCancelSm& pdu);
    virtual PduReplaceSmResp* replace(PduReplaceSm& pdu);
  };
  friend class ASyncTransmitterImpl;
  ASyncTransmitterImpl atrans;

  struct SessionInfo{
    SessionInfo(ClusterSmppSession* argCss,const SmeConfig& cfg):lst(argCss),ss(cfg,&lst),lastReconnect(0),isConnected(false){}
    InnerListener lst;
    SmppSession ss;
    time_t lastReconnect;
    bool isConnected;
  };
  ConnectThread connThr;
  typedef std::vector<SessionInfo*> SVector;
  SVector sessions;
  int connCount;
  EventMonitor connMon;
  int reconnectDelay;

  size_t sendIdx;

  Mutex cntMutex;
  int seqCounter;


  void connectSession(size_t idx)
  {
    try{
      sessions[idx]->ss.connect();
    }catch(...)
    {
      sessions[idx]->lastReconnect=time(0);
      return;
    }
    MutexGuard mg(connMon);
    sessions[idx]->isConnected=true;
    connCount++;
    connMon.notify();
  }

  SmppTransmitter* selectTransmitter(bool isSync)
  {
    size_t idx;
    do{
      MutexGuard mg(connMon);
      if(connCount==0)connMon.wait();
      sendIdx=idx=(sendIdx+1)%sessions.size();
    }while(!sessions[idx]->isConnected);
    return isSync?sessions[idx]->ss.getSyncTransmitter():sessions[idx]->ss.getAsyncTransmitter();
  }
  SmppTransmitter* getTransmitter(size_t idx,bool isSync)
  {
    if(idx>=sessions.size())
    {
      throw std::runtime_error("Invalid session index");
    }
    return isSync?sessions[idx]->ss.getSyncTransmitter():sessions[idx]->ss.getAsyncTransmitter();
  }
};

void ClusterSmppSession::InnerListener::handleError(int code)
{
  MutexGuard mg(css->connMon);
  if(css->connCount==0)
  {
    css->lst->handleError(code);
  }else
  {
    css->connMon.notify();
  }
}

int ClusterSmppSession::ConnectThread::Execute()
{
  std::vector<size_t> toConnect;
  while(!stopRequest)
  {
    {
      MutexGuard mg(css->connMon);
      toConnect.clear();
      time_t now=time(0);
      int toSleep=-1;
      for(size_t i=0;i<css->sessions.size();++i)
      {
        ClusterSmppSession::SessionInfo& si=*css->sessions[i];
        if(!si.isConnected)
        {
          if(now-si.lastReconnect>css->reconnectDelay)
          {
            toConnect.push_back(i);
          }else
          {
            int delay=(int)(now-si.lastReconnect-css->reconnectDelay);
            if(toSleep==-1 || toSleep>delay)
            {
              toSleep=delay;
            }
          }
        }
      }
      if(toConnect.empty())
      {
        if(toSleep!=-1)
        {
          css->connMon.wait(toSleep*1000);
        }else
        {
          css->connMon.wait();
        }
        continue;
      }
    }
    for(size_t i=0;i<toConnect.size();i++)
    {
      css->connectSession(toConnect[i]);
    }
  }
  started=false;
  return 0;
}

PduSubmitSmResp* ClusterSmppSession::SyncTransmitterImpl::submit(PduSubmitSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(true);
  return tr->submit(pdu);
}

SmppHeader* ClusterSmppSession::SyncTransmitterImpl::sendPdu(SmppHeader* pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(true);
  return tr->sendPdu(pdu);
}
void ClusterSmppSession::SyncTransmitterImpl::sendGenericNack(PduGenericNack& pdu)
{
  SmppTransmitter* tr=css->getTransmitter(pdu.get_header().get_sequenceNumber()%maxNumberOfNodes,true);
  return tr->sendGenericNack(pdu);
}
void ClusterSmppSession::SyncTransmitterImpl::sendDeliverySmResp(PduDeliverySmResp& pdu)
{
  SmppTransmitter* tr=css->getTransmitter(pdu.get_header().get_sequenceNumber()%maxNumberOfNodes,true);
  return tr->sendDeliverySmResp(pdu);
}

void ClusterSmppSession::SyncTransmitterImpl::sendDataSmResp(PduDataSmResp& pdu)
{
  SmppTransmitter* tr=css->getTransmitter(pdu.get_header().get_sequenceNumber()%maxNumberOfNodes,true);
  return tr->sendDataSmResp(pdu);
}
PduMultiSmResp* ClusterSmppSession::SyncTransmitterImpl::submitm(PduMultiSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(true);
  return tr->submitm(pdu);
}

PduDataSmResp* ClusterSmppSession::SyncTransmitterImpl::data(PduDataSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(true);
  return tr->data(pdu);
}

PduQuerySmResp* ClusterSmppSession::SyncTransmitterImpl::query(PduQuerySm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(true);
  return tr->query(pdu);
}

PduCancelSmResp* ClusterSmppSession::SyncTransmitterImpl::cancel(PduCancelSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(true);
  return tr->cancel(pdu);
}

PduReplaceSmResp* ClusterSmppSession::SyncTransmitterImpl::replace(PduReplaceSm& pdu)
{
  uint64_t msgId;
  sscanf(pdu.messageId.cstr(),"%lld",&msgId);
  int idx=(int)(msgId%maxNumberOfNodes);
  SmppTransmitter* tr=css->getTransmitter(idx,true);
  return tr->replace(pdu);
}

/*
 *
 * async
 *
 */

PduSubmitSmResp* ClusterSmppSession::ASyncTransmitterImpl::submit(PduSubmitSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(false);
  return tr->submit(pdu);
}

SmppHeader* ClusterSmppSession::ASyncTransmitterImpl::sendPdu(SmppHeader* pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(false);
  return tr->sendPdu(pdu);
}
void ClusterSmppSession::ASyncTransmitterImpl::sendGenericNack(PduGenericNack& pdu)
{
  SmppTransmitter* tr=css->getTransmitter(pdu.get_header().get_sequenceNumber()%maxNumberOfNodes,false);
  return tr->sendGenericNack(pdu);
}
void ClusterSmppSession::ASyncTransmitterImpl::sendDeliverySmResp(PduDeliverySmResp& pdu)
{
  SmppTransmitter* tr=css->getTransmitter(pdu.get_header().get_sequenceNumber()%maxNumberOfNodes,false);
  return tr->sendDeliverySmResp(pdu);
}

void ClusterSmppSession::ASyncTransmitterImpl::sendDataSmResp(PduDataSmResp& pdu)
{
  SmppTransmitter* tr=css->getTransmitter(pdu.get_header().get_sequenceNumber()%maxNumberOfNodes,false);
  return tr->sendDataSmResp(pdu);
}
PduMultiSmResp* ClusterSmppSession::ASyncTransmitterImpl::submitm(PduMultiSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(false);
  return tr->submitm(pdu);
}

PduDataSmResp* ClusterSmppSession::ASyncTransmitterImpl::data(PduDataSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(false);
  return tr->data(pdu);
}

PduQuerySmResp* ClusterSmppSession::ASyncTransmitterImpl::query(PduQuerySm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(false);
  return tr->query(pdu);
}

PduCancelSmResp* ClusterSmppSession::ASyncTransmitterImpl::cancel(PduCancelSm& pdu)
{
  SmppTransmitter* tr=css->selectTransmitter(false);
  return tr->cancel(pdu);
}

PduReplaceSmResp* ClusterSmppSession::ASyncTransmitterImpl::replace(PduReplaceSm& pdu)
{
  uint64_t msgId;
  sscanf(pdu.messageId.cstr(),"%lld",&msgId);
  int idx=(int)(msgId%maxNumberOfNodes);
  SmppTransmitter* tr=css->getTransmitter(idx,false);
  return tr->replace(pdu);
}


}
}

#endif
