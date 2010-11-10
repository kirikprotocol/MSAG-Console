#ifndef __SMSC_SME_SMPPBASE_HPP__
#define __SMSC_SME_SMPPBASE_HPP__

#include "core/threads/Thread.hpp"
#include "smpp/smpp.h"
#include "core/network/Socket.hpp"
#include "util/Exception.hpp"
#include "util/config/ConfigException.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/buffers/IntHash.hpp"
#include "util/debug.h"
#include <time.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <list>
#include "util/sleep.h"
#include "logger/Logger.h"

namespace smsc{
namespace sme{

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using namespace smsc::smpp;
using smsc::util::Exception;
using std::exception;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

class SmppConnectException:public exception{
int reason;
public:
  struct Reason{
    enum{
      networkResolve,
      networkConnect,
      bindFailed,
      smppError,
      timeout,
      unknown
    };
  };
  explicit SmppConnectException(int reason):reason(reason){}
  int getReason()const{return reason;}
  const char* getTextReason()const
  {
    switch(reason)
    {
      case Reason::networkResolve:return "Unable to resolve service center host name";
      case Reason::networkConnect:return "Unable to connect to service center";
      case Reason::bindFailed:return "Failed to bind sme";
      case Reason::smppError:return "Generic smpp error";
      case Reason::timeout:return "Bind attempt timed out";
      case Reason::unknown:return "Unknown error";
    }
    return "INVALID REASON";
  }
  virtual const char* what()const throw(){return getTextReason();}
};

class SmppInvalidBindState:public exception{
public:
  virtual const char* what()const throw(){return "Sent pdu in invalid bind state";}
};


struct Buffer{
  char* buffer;
  int size;
  int offset;

  Buffer(){buffer=0;size=0;offset=0;}
  ~Buffer(){if(buffer)delete [] buffer;}

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
  smppExiting
};

class SmppPduEventListener{
public:
  virtual void handleEvent(SmppHeader *pdu)=0;
  virtual void handleError(int errorCode)=0;
  virtual bool handleIdle(){return false;}
  virtual void handleTimeout(int seqNum){}
};

class SmppThread:public Thread{
public:
  SmppThread():stopped(0)
  {
  }
  using Thread::Start;
  void Start()
  {
    stopped=0;
    Thread::Start();
  }
  virtual void Stop()
  {
    stopped=1;
  }
protected:
  int stopped;
};


static inline void DumpPduBuffer(smsc::logger::Logger* log,const char* prefix,const char* buf)
{
  if(!log)return;
  if(!log->isDebugEnabled())return;
  int32_t sz,id,st,sq;
  memcpy(&sz,buf,4);
  memcpy(&id,buf+4,4);
  memcpy(&st,buf+8,4);
  memcpy(&sq,buf+12,4);
  sz=ntohl(sz);
  id=ntohl(id);
  st=ntohl(st);
  sq=ntohl(sq);
  string res=prefix;
  char tmp[128];
  sprintf(tmp,"sz=%d,id=%x,st=%d,sq=%d:",sz,id,st,sq);
  res+=tmp;
  for(int i=16;i<sz;i++)
  {
    sprintf(tmp," %02X",(unsigned int)(unsigned char)buf[i]);
    res+=tmp;
  }
  log->log(smsc::logger::Logger::LEVEL_DEBUG,"%s",res.c_str());
}

extern "C" typedef void (*SigFunc)(int);


class SmppReader:public SmppThread{
public:
  SmppReader(SmppPduEventListener *lst,Socket *sock,int it,int dt):
    listener(lst),
    socket(sock),
    idleTimeout(it),disconnectTimeout(dt),
    lastUpdate(0), lastTOCheck(0)
  {
    log=smsc::logger::Logger::getInstance("smppdump");
    logrd=smsc::logger::Logger::getInstance("smpp.rd");
    running=false;
  }
#ifndef _WIN32
  static void sigdisp(int sig)
  {
    if(StaticHolder<0>::olddisp && StaticHolder<0>::olddisp!=SIG_HOLD && StaticHolder<0>::olddisp!=SIG_ERR &&
       StaticHolder<0>::olddisp!=(SigFunc)sigdisp)
    {
      StaticHolder<0>::olddisp(sig);
    }
  }
#endif

  int Execute()
  {
    smsc_log_info(logrd,"smpp reader started");

#ifndef _WIN32
    if(StaticHolder<0>::olddisp==0)
    {
      StaticHolder<0>::olddisp=sigset(16,(SigFunc)sigdisp);
    }
#endif
    SmppHeader *pdu;
    running=true;
    while(!stopped)
    {
      try{
        pdu=receivePdu();
      }catch(...)//exception& e)
      {
        //__trace2__("ex:%s",e.what());
        smsc_log_debug(logrd,"Reader: About to call handleError, stopped=%d",stopped);
        if(!stopped)listener->handleError(smppErrorNetwork);
        break;
      }
      if(pdu)
      {
        smsc_log_debug(logrd,"SmppReader: received pdu, cmdid=%x, seqnum=%d",pdu->get_commandId(),pdu->get_sequenceNumber());
        listener->handleEvent(pdu);
        smsc_log_debug(logrd,"SmppReader: handle event ok");
        pdu=NULL;
      }else
      {
        if(!stopped)listener->handleError(smppErrorNetwork);
        break;
      }
    }
    running=false;
    smsc_log_info(logrd,"smpp reader exited");
    return 0;
  }

  bool isRunning()
  {
    return running;
  }

protected:
  SmppPduEventListener *listener;
  Socket *socket;
  Buffer buf;
  int idleTimeout;
  int disconnectTimeout;
  smsc::logger::Logger* log;
  smsc::logger::Logger* logrd;
  volatile bool running;

#ifndef _WIN32
  template <int n>
  struct StaticHolder
  {
    static SigFunc olddisp;
  };
#endif

  time_t lastUpdate;
  time_t lastTOCheck;

  bool IdleCheck()
  {
    time_t now=time(NULL);
    if(lastTOCheck!=now)
    {
      listener->handleTimeout(0);
      lastTOCheck=now;
    }
    if(idleTimeout)
    {
      if(!socket->canRead(1))
      {
        if(time(NULL)-lastUpdate>disconnectTimeout)
        {
          socket->Close();
          return true;
        }
        if(time(NULL)-lastUpdate>idleTimeout)
        {
          listener->handleIdle();
        }
        return false;
      }else
      {
        return true;
      }
    }
    return true;
  }

  SmppHeader* receivePdu()
  {
    smsc_log_debug(logrd,"SmppReader: receive pdu");
    lastUpdate=time(NULL);
    buf.offset=0;
    buf.setSize(64);
    while(buf.offset<4)
    {
      if(!IdleCheck())continue;
      smsc_log_debug(logrd,"SmppReader: read socket %p",socket);
      int rd=socket->Read(buf.buffer+buf.offset,4-buf.offset);
      if(rd<=0)
      {
        smsc_log_debug(logrd,"SmppReader: Socket error %s",strerror(errno));
        return NULL;
      }
      buf.offset+=rd;
    }
    lastUpdate=time(NULL);
    int sz=ntohl(*((int*)buf.buffer));
    if(sz>70000)
    {
      smsc_log_warn(logrd,"Invalid pdu size=%d",sz);
      return NULL;
    }
    buf.setSize(sz);
    while(buf.offset<sz)
    {
      if(!IdleCheck())continue;
      int rd=socket->Read(buf.current(),sz-buf.offset);
      if(rd<=0)return NULL;
      buf.offset+=rd;
    }

    DumpPduBuffer(log,"in:",buf.buffer);
    SmppStream s;
    assignStreamWith(&s,buf.buffer,sz,true);
    return fetchSmppPdu(&s);
  }
};

#ifndef _WIN32
template <int n>
SigFunc SmppReader::StaticHolder<n>::olddisp=0;
#endif

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
    running=false;
    shutdown=false;
    log=smsc::logger::Logger::getInstance("smppdump");
    logwr=smsc::logger::Logger::getInstance("smpp.wr");
    stopped=false;
  }
  int Execute()
  {
    smsc_log_info(logwr,"Starting smppwriter");
    PduBuffer pb;
    mon.Lock();
    running=true;
    shutdown=false;
    while(!stopped)
    {
      while(queue.Count()==0 && !stopped)
      {
        if(shutdown)
        {
          stopped=true;
          break;
        }
        mon.wait(2000);
      }
      if(stopped)break;
      queue.Shift(pb);
      try{
        sendPdu(pb);
      }catch(...)
      {
        if(!stopped)listener->handleError(smppErrorNetwork);
        break;
      }
      delete pb.buf;
    }
    while(queue.Count())
    {
      queue.Pop(pb);
      delete pb.buf;
    }
    smsc_log_info(logwr,"Exiting smppwriter");
    mon.Unlock();
    running=false;
    stopped=false;
    return 0;
  }
  void Stop()
  {
    MutexGuard lock(mon);
    stopped=true;
    mon.notify();
  }
  void Shutdown()
  {
    {
      MutexGuard lock(mon);
      shutdown=true;
      mon.notify();
    }
    WaitFor();
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

  bool isRunning()
  {
    return running;
  }

protected:
  Array<PduBuffer> queue;
  EventMonitor mon;
  SmppPduEventListener *listener;
  Socket *socket;
  bool running;
  bool shutdown;

  smsc::logger::Logger* log;
  smsc::logger::Logger* logwr;


  void sendPdu(PduBuffer& pb)
  {
    int count=0,wr;
    smsc_log_debug(logwr,"writer:sending buffer %p, %d",pb.buf,pb.size);
    DumpPduBuffer(log,"out:",pb.buf);
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
  SmeConfig()
  {
    port=-1;
    timeOut=10;          // connect timeout
    smppTimeOut=120;
    idleTimeout=60;
    disconnectTimeout=300;
    interfaceVersion = 0x34;
  }

    void setSystemType( const std::string& st ) {
        if ( st.size() > 12 ) { throw util::config::ConfigException("systemType is too long: %s", st.c_str()); }
        else systemType = st;
    }
    void setAddressRange( const std::string& st ) {
        addressRange = st;
        uint8_t ton, npi;
        const char* ar = 0;
        parseAddressRange(ton,npi,ar);
    }

    /// throws ConfigException
    void parseAddressRange(uint8_t& ton, uint8_t& npi, const char*& arstr) const {
        arstr = "";
        const std::string& ar = addressRange;
        if ( ar.empty() ) {
            ton = npi = 0;
        } else if ( ar[0] == '+' ) {
            ton = npi = 1;
            arstr = ar.c_str()+1;
        } else if ( ar[0] == '.' ) {
            int iton, inpi, shift = 0;
            sscanf(ar.c_str(),".%u.%u.%n",&iton,&inpi,&shift);
            if ( shift == 0 ) {
                throw util::config::ConfigException("problems reading TON NPI from addressRange %s",ar.c_str());
            }
            ton = uint8_t(iton);
            npi = uint8_t(inpi);
            arstr = ar.c_str() + shift;
        } else {
            ton = 0;
            npi = 1;
            arstr = ar.c_str();
        }
        const size_t arlen = strlen(arstr);
        if ( ton != 0 && npi != 0 ) {
            if ( arlen == 0 ) {
                throw util::config::ConfigException("length of addressRange value is 0: %s", ar.c_str());
            } else if ( arlen > 40 ) {
                throw util::config::ConfigException("length of addressRange value > 40: %s", ar.c_str());
            }
        }
    }

  std::string host;
  int port;
  std::string sid;
  int timeOut;        // connect timeout, seconds
  int smppTimeOut;
  std::string password;
  std::string systemType;
  std::string origAddr;
  int interfaceVersion;
  std::string addressRange;

  int idleTimeout;
  int disconnectTimeout;
};

namespace BindType{
  const int Receiver = 1;
  const int Transmitter = 2;
  const int Transceiver = 3;
}

class SmppSession{
protected:
  struct TOItem{
    time_t to;
    int seq;
  };
  typedef std::list<TOItem> TOList;
  struct Lock{
    Event* event;
    SmppHeader *pdu;
    int error;
    TOList::iterator toit;
  };
  class InnerListener;
  friend class InnerListener;
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
      if(errorCode==smppErrorNetwork)
      {
        session.abortWaits();
      }
    }
    void handleEvent(SmppHeader* pdu)
    {
      session.processIncoming(pdu);
    }
    bool handleIdle()
    {
      if(session.listener)
      {
        if(!session.listener->handleIdle())
        {
          PduEnquireLink el;
          el.get_header().set_commandId(SmppCommandSet::ENQUIRE_LINK);
          el.get_header().set_sequenceNumber(session.getNextSeq());
          el.get_header().set_commandStatus(0);
          session.getAsyncTransmitter()->sendPdu((SmppHeader*)&el);
        }
      }
      return true;
    }
    void handleTimeout(int seq)
    {
      session.processTimeouts();
    }
  };
  class InnerSyncTransmitter;
  friend class InnerSyncTransmitter;
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
      if(!session.checkOutgoingValidity(pdu))
      {
        throw SmppInvalidBindState();
      }
      Event event;
      int seq=pdu->get_sequenceNumber();
      session.registerPdu(seq,&event);
      session.writer.enqueue(pdu);
      event.Wait(session.cfg.smppTimeOut*1000);
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
  class InnerAsyncTransmitter;
  friend class InnerAsyncTransmitter;
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
      if(!session.checkOutgoingValidity(pdu))
      {
        throw SmppInvalidBindState();
      }
      //int seq=pdu->get_sequenceNumber();
      session.registerPdu(pdu->get_sequenceNumber(),NULL);
      session.writer.enqueue(pdu);
      return NULL;
    };
  };

#ifdef _WIN32
  /*
  friend smsc::sme::SmppSession::InnerAsyncTransmitter;
  friend smsc::sme::SmppSession::InnerSyncTransmitter;
  friend smsc::sme::SmppSession::InnerListener;
  */
#endif

public:
  SmppSession(const SmeConfig& config,SmppPduEventListener* lst):
    cfg(config),
    listener(lst),
    innerListener(*this),
    reader(&innerListener,&socket,config.idleTimeout,config.disconnectTimeout),
    writer(&innerListener,&socket),
    seqCounter(0),
    strans(*this),
    atrans(*this),
    closed(true)
#ifdef INTHASH_USAGE_CHECKING
    ,lock(SMSCFILELINE)
#endif
  {
    log=smsc::logger::Logger::getInstance("smpp.ses");
    smsc_log_info(log,"SmppSession: create session %s:%p", config.sid.c_str(),this);
  }
  ~SmppSession()
  {
    close();
  }
  void setConfig(const SmeConfig& config)
  {
    cfg=config;
  }
  void connect(int bindtype=BindType::Transceiver)//throw(SmppConnectException)
  {
    smsc_log_info(log,"SmppSession: connect %p, %s:%d to=%d",this, cfg.host.c_str(), cfg.port, cfg.timeOut);
    if(!closed)return;
    if(socket.Init(cfg.host.c_str(),cfg.port,cfg.timeOut)==-1)
      throw SmppConnectException(SmppConnectException::Reason::networkResolve);

    socket.setConnectTimeout(cfg.timeOut);
    if(socket.Connect()==-1)
      throw SmppConnectException(SmppConnectException::Reason::networkConnect);
    reader.Start();
    writer.Start();
    PduBindTRX pdu;
    unsigned int expectedbindresp;
    switch(bindtype)
    {
      case BindType::Transceiver:
        pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
        expectedbindresp=SmppCommandSet::BIND_TRANCIEVER_RESP;
        break;
      case BindType::Transmitter:
        pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANSMITTER);
        expectedbindresp=SmppCommandSet::BIND_TRANSMITTER_RESP;
        break;
      case BindType::Receiver:
        pdu.get_header().set_commandId(SmppCommandSet::BIND_RECIEVER);
        expectedbindresp=SmppCommandSet::BIND_RECIEVER_RESP;
        break;
    }
    bindType=bindtype;
    //pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
    pdu.set_systemId(cfg.sid.c_str());
    pdu.set_password(cfg.password.c_str());
    pdu.set_systemType(cfg.systemType.c_str());
    pdu.set_interfaceVersion(cfg.interfaceVersion);
    if( ! cfg.addressRange.empty())
    {
        uint8_t ton, npi;
        const char* ar;
        try {
            cfg.parseAddressRange(ton,npi,ar);
        } catch ( util::config::ConfigException& e ) {
            smsc_log_error(log,"SmppSession: bad address range: %s",e.what());
            throw SmppConnectException(SmppConnectException::Reason::bindFailed);
        }
        pdu.get_addressRange().set_typeOfNumber(ton);
        pdu.get_addressRange().set_numberingPlan(npi);
        pdu.get_addressRange().set_value(ar);
    }
    int seq=getNextSeq();
    pdu.get_header().set_sequenceNumber(seq);

    PduBindTRXResp *resp=(PduBindTRXResp*)strans.sendPdu((SmppHeader*)&pdu);

    if(!resp || resp->get_header().get_commandId()!=expectedbindresp ||
       resp->get_header().get_sequenceNumber()!=pdu.get_header().get_sequenceNumber() ||
       resp->get_header().get_commandStatus()!=SmppStatusSet::ESME_ROK)
    {
      int reason=!resp?SmppConnectException::Reason::timeout :
               resp->get_header().get_commandId()!=expectedbindresp ||
               resp->get_header().get_sequenceNumber()!=pdu.get_header().get_sequenceNumber() ?
               SmppConnectException::Reason::smppError:
               resp->get_header().get_commandStatus()==SmppStatusSet::ESME_RBINDFAIL?
               SmppConnectException::Reason::bindFailed:
               SmppConnectException::Reason::unknown;
      if(resp)
      {
        smsc_log_warn(log,"Unexpected bind response code:%04X",resp->get_header().get_commandStatus());
        disposePdu((SmppHeader*)resp);
      }
      reader.Stop();
      writer.Stop();
      socket.Close();
      reader.WaitFor();
      writer.WaitFor();
      throw SmppConnectException(reason);
    }
    disposePdu((SmppHeader*)resp);
    smsc_log_info(log,"SmppSession: connected %p",this);
    closed=false;
  }
  void close()
  {
    if(closed)return;
    smsc_log_info(log,"SmppSession: closing %p",this);
    reader.Stop();
    writer.Stop();
    socket.Close();
#ifndef _WIN32
    reader.Kill(16);
#endif
    if(reader.isRunning())reader.WaitFor();
    if(reader.isRunning())writer.WaitFor();
    closed=true;
    abortWaits();
    lock.Empty();
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

  void getPeer(char* in)
  {
      socket.GetPeer(in);
  }

    bool isClosed() const {
        return closed;
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
  int bindType;

  smsc::logger::Logger* log;

  IntHash<Lock> lock;
  TOList tolist;

  void processTimeouts()
  {
    MutexGuard g(lockMutex);
    if(tolist.empty())
    {
      return;
    }
    time_t now=time(NULL);
    while(! tolist.empty() && tolist.front().to < now )
    {
      int key=tolist.front().seq;
      Lock* l = lock.GetPtr(key);
      if (!l) {
          // lock was already processed
          tolist.erase(tolist.begin());
          continue;
      }
      if(!l->pdu) // in synchronous case if l.pdu assigned than it was already received
      {
        listener->handleTimeout(key);
        tolist.erase(tolist.begin());
        lock.Delete(key);
      }
      if(l->event)
      {
        l->event->Signal();
      }
    }
    /*
    IntHash<Lock>::Iterator i=lock.First();
    Lock l;
    int key;
    time_t now=time(NULL);
    while(i.Next(key,l))
    {
      smsc_log_debug(log,"to check:%d<%d",l.timeOut,now);
      if(l.timeOut<now)
      {
        if(!l.pdu)
        {
          listener->handleTimeout(key);
          lock.Delete(key);
        }
        if(l.event)
        {
          l.event->Signal();
        }
      }
    }
    */
  }

  void registerPdu(int seq,Event* event)
  {
    processTimeouts();
    MutexGuard g(lockMutex);
    smsc_log_debug(log,"registerPdu seq=%d",seq);
    if(lock.Exist(seq))
    {
      throw smsc::util::Exception("Attempt to register pdu with already registered seq=%d",seq);
    }
    TOItem ti;
    ti.to=time(NULL)+cfg.smppTimeOut;
    ti.seq=seq;
    Lock l;
    l.toit=tolist.insert(tolist.end(),ti);
    //l.timeOut=ti.to;
    l.event=event;
    l.pdu=NULL;
    l.error=0;
    lock.Insert(seq,l);
  }
  SmppHeader* getPduResponse(int seq)
  {
    MutexGuard g(lockMutex);
    if(!lock.Exist(seq))return NULL;
    const Lock &l=lock.Get(seq);
    if(l.error)
    {
      tolist.erase(l.toit);
      lock.Delete(seq);
      throw Exception("Unknown error");
    }
    SmppHeader *retval=l.pdu;
    tolist.erase(l.toit);
    lock.Delete(seq);
    return retval;
  }

  void abortWaits()
  {
    MutexGuard g(lockMutex);
    Lock l;
    int key;
    IntHash<Lock>::Iterator i=lock.First();
    while(i.Next(key,l))
    {
      if(l.event)
      {
        l.event->Signal();
      }
    }
  }

  /*bool checkIncomingValidity(SmppHeader* pdu)
  {
    using namespace SmppCommandSet;
    switch(pdu->get_commandId())
    {
      case BIND_TRANSMITTER_RESP:
      case BIND_RECIEVER_RESP:
      case BIND_TRANCIEVER_RESP:
        return true;
    }
    switch(bindType)
    {
      case BindType::Receiver:
        switch(pdu->get_commandId())
        {
          case DELIVERY_SM:
          case GENERIC_NACK:
          case UNBIND_RESP:
            return true;
          case SUBMIT_SM_RESP:
          case DELIVERY_SM_RESP:
          case SUBMIT_SM:
          case QUERY_SM:
          default:
            return false;
        }
      case BindType::Transmitter:
        switch(pdu->get_commandId())
        {
          case GENERIC_NACK:
          case SUBMIT_SM_RESP:
          case UNBIND_RESP:
            return true;
          case DELIVERY_SM:
          case DELIVERY_SM_RESP:
          case SUBMIT_SM:
          case QUERY_SM:
          default:
            return false;
        }
      case BindType::Transceiver:return true;
    }
    return false;
  }*/
  bool checkIncomingValidity(SmppHeader* pdu)
  {
    using namespace SmppCommandSet;
    switch(pdu->get_commandId())
    {
      case BIND_TRANSMITTER_RESP:
      case BIND_RECIEVER_RESP:
      case BIND_TRANCIEVER_RESP:
      case ENQUIRE_LINK:
      case ENQUIRE_LINK_RESP:
      case UNBIND:
      case UNBIND_RESP:
      case ALERT_NOTIFICATION:
        return true;
    }
    switch(bindType)
    {
      case BindType::Receiver:
        switch(pdu->get_commandId())
        {
          case DATA_SM:
          case DELIVERY_SM:
          case GENERIC_NACK:
            return true;
          default:
            return false;
        }
      case BindType::Transmitter:
        switch(pdu->get_commandId())
        {
          case DATA_SM:
          case DATA_SM_RESP:
          case GENERIC_NACK:
          case SUBMIT_SM_RESP:
          case SUBMIT_MULTI_RESP:
          case UNBIND_RESP:
          case CANCEL_SM_RESP:
          case QUERY_SM_RESP:
          case REPLACE_SM_RESP:
            return true;
          default:
            return false;
        }
      case BindType::Transceiver:
        switch(pdu->get_commandId())
        {
          case DATA_SM:
          case DATA_SM_RESP:
          case GENERIC_NACK:
          case SUBMIT_SM_RESP:
          case SUBMIT_MULTI_RESP:
          case CANCEL_SM_RESP:
          case QUERY_SM_RESP:
          case REPLACE_SM_RESP:
          case DELIVERY_SM:
            return true;
          default:
            return false;
        }
    }
    return false;
  }

  bool checkOutgoingValidity(SmppHeader* pdu)
  {
    using namespace SmppCommandSet;
    switch(pdu->get_commandId())
    {
      case BIND_TRANSMITTER:
      case BIND_RECIEVER:
      case BIND_TRANCIEVER:
      case ENQUIRE_LINK:
      case ENQUIRE_LINK_RESP:
      case UNBIND:
      case UNBIND_RESP:
        return true;
    }
    switch(bindType)
    {
      case BindType::Receiver:
        switch(pdu->get_commandId())
        {
          case DELIVERY_SM_RESP:
          case GENERIC_NACK:
            return true;
          default:
            return false;
        }
      case BindType::Transmitter:
        switch(pdu->get_commandId())
        {
          case DATA_SM:
          case DATA_SM_RESP:
          case SUBMIT_SM:
          case SUBMIT_MULTI:
          case GENERIC_NACK:
          case CANCEL_SM:
          case REPLACE_SM:
          case QUERY_SM:
            return true;
          default:
            return false;
        }
      case BindType::Transceiver:
        switch(pdu->get_commandId())
        {
          case DATA_SM:
          case DATA_SM_RESP:
          case SUBMIT_SM:
          case SUBMIT_MULTI:
          case GENERIC_NACK:
          case CANCEL_SM:
          case REPLACE_SM:
          case QUERY_SM:
          case DELIVERY_SM_RESP:
            return true;
          default:
            return false;
        }
    }
    return false;
  }


  void processIncoming(SmppHeader* pdu)
  {
    using namespace smsc::smpp::SmppCommandSet;
    if(!checkIncomingValidity(pdu))
    {
      smsc_log_warn(log,"processIncoming: received pdu in invalid bind state (%x,%d)",pdu->get_commandId(),bindType);
      PduGenericNack gnack;
      gnack.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      atrans.sendGenericNack(gnack);
      return;
    }
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
            lockMutex.Unlock();
            l.error=1;
            l.event->Signal();
            disposePdu(pdu);
          }else
          {
            tolist.erase(l.toit);
            lock.Delete(seq);
            lockMutex.Unlock();
            listener->handleEvent(pdu);
          }
        }else
        {
          disposePdu(pdu);
          lockMutex.Unlock();
        }
      }break;
      /*case ENQUIRE_LINK_RESP:
      {
        lockMutex.Unlock();
        disposePdu(pdu);
      }break;*/
      case ENQUIRE_LINK:
      {
        lockMutex.Unlock();
        PduEnquireLinkResp resp;
        resp.get_header().set_commandId(ENQUIRE_LINK_RESP);
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        resp.get_header().set_commandStatus(0);
        writer.enqueue((SmppHeader*)&resp);
        disposePdu(pdu);
      }break;
      case SUBMIT_SM_RESP:
      case SUBMIT_MULTI_RESP:
      case DATA_SM_RESP:
      case QUERY_SM_RESP:
      case CANCEL_SM_RESP:
      case REPLACE_SM_RESP:
      case BIND_TRANCIEVER_RESP:
      case BIND_TRANSMITTER_RESP:
      case BIND_RECIEVER_RESP:
      case UNBIND_RESP:
      case ENQUIRE_LINK_RESP:
      {
        if(lock.Exist(seq))
        {
          smsc_log_debug(log,"processIncoming: lock for %d found",seq);
          Lock &l=lock.Get(seq);
          if(l.event)
          {
            l.pdu=pdu;
            lockMutex.Unlock();
            l.event->Signal();
          }else
          {
            tolist.erase(l.toit);
            lock.Delete(seq);
            lockMutex.Unlock();
            listener->handleEvent(pdu);
          }
        }else
        {
          //__trace2__("processIncoming: lock for %d not found!!!",seq);
          lockMutex.Unlock();
          //disposePdu(pdu);
          listener->handleEvent(pdu);
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
      case UNBIND:
      {
        lockMutex.Unlock();
        listener->handleEvent(pdu);
        PduUnbindResp resp;
        resp.get_header().set_commandId(UNBIND_RESP);
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        resp.get_header().set_commandStatus(0);
        writer.enqueue((SmppHeader*)&resp);
        //disposePdu(pdu);
        writer.Shutdown();
        close();
        listener->handleError(smppErrorNetwork);
      }break;
      default:
      {
        lockMutex.Unlock();
      }
    }
  }
};

}//sme
}//smsc


#endif
