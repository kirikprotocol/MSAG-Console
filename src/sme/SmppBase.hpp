#ifndef __SMSC_SME_SMPPBASE_HPP__
#define __SMSC_SME_SMPPBASE_HPP__

#include "core/threads/Thread.hpp"
#include "smpp/smpp.h"
#include "core/network/Socket.hpp"
#include "util/Exception.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/buffers/IntHash.hpp"
#include "util/debug.h"
#include <time.h>
#include <string>

namespace smsc{
namespace sme{

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using namespace smsc::smpp;
using smsc::util::Exception;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;


struct Buffer{
  char* buffer;
  int size;
  int offset;

  Buffer(){buffer=0;size=0;offset=0;}

  void setSize(int newsize)
  {
    if(newsize<size)return;
    char *newbuf=new char[newsize];
    if(offset)memcpy(newbuf,buffer,offset);
    if(buffer)delete [] buffer;
    buffer=newbuf;
    size=newsize;
  }
  void append(char *mem,int count)
  {
    if(offset+count>size)setSize((offset+count)+(offset+count)/2);
    memcpy(buffer+offset,mem,count);
    offset+=count;
  }
  char* current(){return buffer+offset;}
  int freeSpace(){return size-offset;}
};

enum SmppError{
  smppErrorNetwork=0x1000,
  smppErrorInvalidPdu,
};

class SmppPduEventListener{
public:
  virtual void handleEvent(SmppHeader *pdu)=0;
  virtual void handleError(int errorCode)=0;
};

class SmppThread:public Thread{
public:
  SmppThread():stopped(0)
  {
  }
  virtual void Stop()
  {
    stopped=1;
  }
protected:
  int stopped;
};

class SmppReader:public SmppThread{
public:
  SmppReader(SmppPduEventListener *lst,Socket *sock):
    listener(lst),
    socket(sock)
  {
  }
  int Execute()
  {
    SmppHeader *pdu;
    while(!stopped)
    {
      try{
        pdu=receivePdu();
      }catch(std::exception& e)
      {
        __trace2__("ex:%s",e.what());
        listener->handleError(smppErrorNetwork);
        break;
      }
      if(pdu)
      {
        listener->handleEvent(pdu);
        pdu=NULL;
      }
    }
    __trace__("smpp reader exited");
    return 0;
  }
protected:
  SmppPduEventListener *listener;
  Socket *socket;
  Buffer buf;
  SmppHeader* receivePdu()
  {
    buf.offset=0;
    buf.setSize(32);
    while(buf.offset<4)
    {
      int rd=socket->Read(buf.buffer+buf.offset,4-buf.offset);
      if(rd<=0)throw Exception("SMPP transport network error (receiving header)");
      buf.offset+=rd;
    }
    int sz=ntohl(*((int*)buf.buffer));
    buf.setSize(sz);
    while(buf.offset<sz)
    {
      int rd=socket->Read(buf.current(),sz-buf.offset);
      if(rd<=0)throw Exception("SMPP transport network error (receiving body)");
      buf.offset+=rd;
    }
    SmppStream s;
    assignStreamWith(&s,buf.buffer,sz,true);
    return fetchSmppPdu(&s);
  }
};

class SmppWriter:public SmppThread{
protected:
  struct PduBuffer{
    char *buf;
    int size;
  };
public:
  SmppWriter(SmppPduEventListener *lst,Socket* sock):
    listener(lst),
    socket(sock)
  {
  }
  int Execute()
  {
    PduBuffer pb;
    mon.Lock();
    while(!stopped)
    {
      while(queue.Count()==0 && !stopped)
      {
        mon.wait();
      }
      if(stopped)break;
      queue.Shift(pb);
      try{
        sendPdu(pb);
      }catch(...)
      {
        listener->handleError(smppErrorNetwork);
        break;
      }
      delete pb.buf;
    }
    __trace2__("Exiting smppwriter\n");
    mon.Unlock();
    return 0;
  }
  void Stop()
  {
    stopped=true;
    mon.notify();
  }
  void enqueue(SmppHeader* pdu)
  {
    MutexGuard lock(mon);
    int sz=calcSmppPacketLength(pdu);
    PduBuffer pb;
    pb.buf=new char[sz];
    pb.size=sz;
    SmppStream s;
    assignStreamWith(&s,pb.buf,sz,false);
    if(!fillSmppPdu(&s,pdu))throw Exception("Failed to fill smpp packet");
    queue.Push(pb);
    mon.notify();
  }
protected:
  Array<PduBuffer> queue;
  EventMonitor mon;
  SmppPduEventListener *listener;
  Socket *socket;

  void sendPdu(PduBuffer& pb)
  {
    int count=0,wr;
    __trace2__("writer:sending buffer %p, %d",pb.buf,pb.size);
    do{
      wr=socket->Write(pb.buf+count,pb.size-count);
      if(wr<=0)throw Exception("Failed to send smpp packet");
      count+=wr;
    }while(count!=pb.size);
  }

};

class SmppTransmitter{
public:
  virtual SmppHeader* sendPdu(SmppHeader* pdu)=0;
  virtual void sendGenericNack(PduGenericNack& pdu)=0;
  virtual void sendDeliverySmResp(PduDeliverySmResp& pdu)=0;
  virtual void sendDataSmResp(PduDataSmResp& pdu)=0;
  virtual PduSubmitSmResp* submit(PduSubmitSm& pdu)=0;
  virtual PduMultiSmResp* submitm(PduMultiSm& pdu)=0;
  virtual PduDataSmResp* data(PduDataSm& pdu)=0;
  virtual PduQuerySmResp* query(PduQuerySm& pdu)=0;
  virtual PduCancelSmResp* cancel(PduCancelSm& pdu)=0;
  virtual PduReplaceSmResp* replace(PduReplaceSm& pdu)=0;
};

class SmppBaseReceiver: public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    using namespace smsc::smpp::SmppCommandSet;
    switch(pdu->get_commandId())
    {
      case GENERIC_NACK:      processGenericNack(*(PduGenericNack*)pdu);break;
      case SUBMIT_SM_RESP:    processSubmitSmResp(*(PduSubmitSmResp*)pdu);break;
      case SUBMIT_MULTI_RESP: processMultiResp(*(PduMultiSmResp*)pdu);break;
      case DELIVERY_SM:       processDeliverySm(*(PduDeliverySm*)pdu);break;
      case DATA_SM:           processDataSm(*(PduDataSm*)pdu);break;
      case DATA_SM_RESP:      processDataSmResp(*(PduDataSmResp*)pdu);break;
      case QUERY_SM_RESP:     processQuerySmResp(*(PduQuerySmResp*)pdu);break;
      case CANCEL_SM_RESP:    processCancelSmResp(*(PduCancelSmResp*)pdu);break;
      case REPLACE_SM_RESP:   processReplaceSmResp(*(PduReplaceSmResp*)pdu);break;
      case ALERT_NOTIFICATION:processAlertNotification(*(PduAlertNotification*)pdu);break;
      default: handleError(smppErrorInvalidPdu);break;
    }
    disposePdu(pdu);
  }
  virtual void processGenericNack(PduGenericNack& pdu){};
  virtual void processSubmitSmResp(PduSubmitSmResp& pdu){};
  virtual void processMultiResp(PduMultiSmResp& pdu){};
  virtual void processDeliverySm(PduDeliverySm& pdu){};
  virtual void processDataSm(PduDataSm& pdu){};
  virtual void processDataSmResp(PduDataSmResp& pdu){};
  virtual void processQuerySmResp(PduQuerySmResp& pdu){};
  virtual void processCancelSmResp(PduCancelSmResp& pdu){};
  virtual void processReplaceSmResp(PduReplaceSmResp& pdu){};
  virtual void processAlertNotification(PduAlertNotification& pdu){};
};

struct SmeConfig{
  std::string host;
  int port;
  std::string sid;
  int timeOut;
  std::string password;
  std::string systemType;
  std::string origAddr;
};

class SmppSession{
protected:
  struct Lock{
    time_t timeOut;
    Event* event;
    SmppHeader *pdu;
    int error;
  };
  class InnerListener:public SmppPduEventListener{
  public:
    SmppSession& session;
    InnerListener(SmppSession& s):session(s)
    {
    }
    void handleError(int errorCode)
    {
      if(session.listener)
      {
        session.listener->handleError(errorCode);
      }
    }
    void handleEvent(SmppHeader* pdu)
    {
      session.processIncoming(pdu);
    }
  };
  class InnerSyncTransmitter:public SmppTransmitter{
  public:

    InnerSyncTransmitter(SmppSession& s):session(s)
    {
    }
    virtual ~InnerSyncTransmitter()
    {
    }
    SmppHeader* sendPdu(SmppHeader* pdu)
    {
      Event event;
      int seq=(const_cast<SmppHeader*>(pdu))->get_sequenceNumber();
      session.registerPdu(seq,&event);
      session.writer.enqueue(pdu);
      event.Wait(10000);
      return session.getPduResponse(seq);
    };

    void sendGenericNack(PduGenericNack& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::GENERIC_NACK);
      //pdu.get_header().set_sequenceNumber(session.getNextSeq());
      session.writer.enqueue((SmppHeader*)&pdu);
    };
    void sendDeliverySmResp(PduDeliverySmResp& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      //pdu.get_header().set_sequenceNumber(session.getNextSeq());
      session.writer.enqueue((SmppHeader*)&pdu);
    };
    void sendDataSmResp(PduDataSmResp& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
      //pdu.get_header().set_sequenceNumber(session.getNextSeq());
      session.writer.enqueue((SmppHeader*)&pdu);
    };
    PduSubmitSmResp* submit(PduSubmitSm& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
      pdu.get_header().set_sequenceNumber(session.getNextSeq());
      return (PduSubmitSmResp*)sendPdu((SmppHeader*)&pdu);
    };
    PduMultiSmResp* submitm(PduMultiSm& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_MULTI);
      pdu.get_header().set_sequenceNumber(session.getNextSeq());
      return (PduMultiSmResp*)sendPdu((SmppHeader*)&pdu);
    };
    PduDataSmResp* data(PduDataSm& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::DATA_SM);
      pdu.get_header().set_sequenceNumber(session.getNextSeq());
      return (PduDataSmResp*)sendPdu((SmppHeader*)&pdu);
    };
    PduQuerySmResp* query(PduQuerySm& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::QUERY_SM);
      pdu.get_header().set_sequenceNumber(session.getNextSeq());
      return (PduQuerySmResp*)sendPdu((SmppHeader*)&pdu);
    };
    PduCancelSmResp* cancel(PduCancelSm& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::CANCEL_SM);
      pdu.get_header().set_sequenceNumber(session.getNextSeq());
      return (PduCancelSmResp*)sendPdu((SmppHeader*)&pdu);
    };
    PduReplaceSmResp* replace(PduReplaceSm& pdu)
    {
      pdu.get_header().set_commandId(SmppCommandSet::REPLACE_SM);
      pdu.get_header().set_sequenceNumber(session.getNextSeq());
      return (PduReplaceSmResp*)sendPdu((SmppHeader*)&pdu);
    };
  protected:
    SmppSession& session;
  };
  class InnerAsyncTransmitter:public InnerSyncTransmitter{
  public:
    InnerAsyncTransmitter(SmppSession& s):InnerSyncTransmitter(s)
    {
    }
    virtual ~InnerAsyncTransmitter()
    {
    }

    SmppHeader* sendPdu(SmppHeader* pdu)
    {
      int seq=pdu->get_sequenceNumber();
      session.registerPdu(seq,NULL);
      session.writer.enqueue(pdu);
      return NULL;
    };
  };

public:
  SmppSession(const SmeConfig& config,SmppPduEventListener* lst):
    cfg(config),
    listener(lst),
    innerListener(*this),
    reader(&innerListener,&socket),
    writer(&innerListener,&socket),
    seqCounter(0),
    strans(*this),
    atrans(*this),
    closed(false)
  {
  }
  ~SmppSession()
  {
    close();
  }
  void connect()
  {
    if(socket.Init(cfg.host.c_str(),cfg.port,cfg.timeOut)==-1)
      throw Exception("Failed to resolve smsc host");
    if(socket.Connect()==-1)
      throw Exception("Failed to connect to smsc host");
    reader.Start();
    writer.Start();
    PduBindTRX pdu;
    pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
    pdu.set_systemId(cfg.sid.c_str());
    pdu.set_password(cfg.password.c_str());
    pdu.set_systemType(cfg.systemType.c_str());
    int seq=getNextSeq();
    pdu.get_header().set_sequenceNumber(seq);

    PduBindTRXResp *resp=(PduBindTRXResp*)strans.sendPdu((SmppHeader*)&pdu);

    if(!resp || resp->get_header().get_commandId()!=SmppCommandSet::BIND_TRANCIEVER_RESP ||
       resp->get_header().get_sequenceNumber()!=pdu.get_header().get_sequenceNumber())
    {
      disposePdu((SmppHeader*)resp);
      throw Exception("Unable to bind sme");
    }
  }
  void close()
  {
    if(closed)return;
    reader.Stop();
    writer.Stop();
    socket.Close();
    reader.WaitFor();
    writer.WaitFor();
    closed=true;
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
  SmeConfig cfg;
  Socket socket;
  SmppPduEventListener *listener;
  InnerListener innerListener;
  SmppReader reader;
  SmppWriter writer;
  int seqCounter;
  InnerSyncTransmitter strans;
  InnerAsyncTransmitter atrans;
  Mutex cntMutex,lockMutex;
  bool closed;

  IntHash<Lock> lock;

  void registerPdu(int seq,Event* event)
  {
    MutexGuard g(lockMutex);
    IntHash<Lock>::Iterator i=lock.First();
    Lock l;
    int key;
    time_t now=time(NULL);
    while(i.Next(key,l))
    {
      if(l.timeOut<now)
      {
        if(!l.pdu)lock.Delete(key);
        if(l.event)
        {
          l.event->Signal();
        }
      }
    }
    l.timeOut=time(NULL)+10;
    l.event=event;
    l.pdu=NULL;
    l.error=0;
    lock.Insert(seq,l);
  }
  SmppHeader* getPduResponse(int seq)
  {
    MutexGuard g(lockMutex);
    if(!lock.Exist(seq))return NULL;
    Lock &l=lock.Get(seq);
    if(l.error)throw Exception("Unknown error");
    SmppHeader *retval=l.pdu;
    lock.Delete(seq);
    return retval;
  }
  void processIncoming(SmppHeader* pdu)
  {
    using namespace smsc::smpp::SmppCommandSet;
    lockMutex.Lock();
    int seq=pdu->get_sequenceNumber();
    switch(pdu->get_commandId())
    {
      //отдельно отработаем всякие разные респонсы.
      case GENERIC_NACK://облом однако.
      {
        if(lock.Exist(seq))
        {
          Lock &l=lock.Get(seq);
          if(l.event)
          {
            l.error=1;
            l.event->Signal();
            disposePdu(pdu);
          }else
          {
            lock.Delete(seq);
            lockMutex.Unlock();
            listener->handleEvent(pdu);
          }
        }
      }break;
      case SUBMIT_SM_RESP:
      case DATA_SM_RESP:
      case QUERY_SM_RESP:
      case CANCEL_SM_RESP:
      case REPLACE_SM_RESP:
      case SUBMIT_MULTI_RESP:
      case BIND_TRANCIEVER_RESP:
      {
        if(lock.Exist(seq))
        {
          Lock &l=lock.Get(seq);
          if(l.event)
          {
            l.pdu=pdu;
            l.event->Signal();
          }else
          {
            lock.Delete(seq);
            lockMutex.Unlock();
            listener->handleEvent(pdu);
          }
        }else
        {
          disposePdu(pdu);
        }
      }break;
      // и отдельно собственно пришедшие smpp-шки.
      case DELIVERY_SM:
      case DATA_SM:
      case ALERT_NOTIFICATION:
      {
        lockMutex.Unlock();
        listener->handleEvent(pdu);
      }break;
      default:
      {
        lockMutex.Unlock();
      }
    }
  }
};

};//sme
};//smsc


#endif
