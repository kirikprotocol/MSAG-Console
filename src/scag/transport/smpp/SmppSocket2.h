#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKET2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKET2_H__

#include <string>

#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"

#include "SmppCommandQueue2.h"
#include "SmppChannelRegistrator2.h"
#include "SmppChannel2.h"
#include "SmppTypes2.h"

#include "SmppSMInterface2.h"

#include "logger/Logger.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace smsc::core::synchronization;
using namespace smsc::core::network;
using namespace smsc::core::buffers;

struct PduGuard{
  SmppHeader* pdu;
  PduGuard():pdu(0){}
  PduGuard(SmppHeader* argPdu):pdu(argPdu){}
  ~PduGuard()
  {
    if(pdu)disposePdu(pdu);
  }
  PduGuard& operator=(SmppHeader* argPdu)
  {
    if(pdu)disposePdu(pdu);
    pdu=argPdu;
    return *this;
  }
  SmppHeader* operator->(){return pdu;}
  operator SmppHeader*(){return pdu;}
  template <class T>
  T& as()
  {
    return *(T*)pdu;
  }
  bool isNull(){return pdu==0;}
protected:
  PduGuard(const PduGuard&);
};


struct SmppSocket:SmppChannel{

  SmppSocket()
  {
    Init();
  }

  SmppSocket(Socket* s)
  {
    Init();
    smsc_log_debug(log, "SmmpSocket init: %x", s);      
    sock=s;
    connected=true;
    sock->setData(0,this);
  }

  virtual ~SmppSocket()
  {
    smsc_log_debug(log, "SmmpSocket destroying: %x", sock);
    delete [] wrBuffer;
    delete [] rdBuffer;
    if(sock)delete sock;
    if ( outQueue.Count() > 0 ) {
        smsc_log_warn(log, "destructor: there are %u commands to send", outQueue.Count() );
        SmppCommand* cmd;
        while ( outQueue.Pop(cmd) )
            delete cmd;
    }
  }

  void acquire()
  {
    MutexGuard mg(mtx);
    refCount++;
    smsc_log_debug(log, "acquire:%p(%s)/%d",this,systemId.c_str(),refCount);
  }

  void release()
  {
    int cnt;
    {
      MutexGuard mg(mtx);
      cnt=--refCount;
      smsc_log_debug(log, "release:%p(%s)/%d",this,systemId.c_str(),refCount);
    }
    if(!cnt)
    {
      smsc_log_debug(log, "Deleting socket for %s",systemId.c_str());
      if(bindType!=btNone)
      {
        smsc_log_debug(log, "unregisterChannel(bt=%d)",bindType);
        chReg->unregisterChannel(this);
        bindType=btNone;
      }
      sm->unregisterSocket(this);
      delete this;
    }
  }

  void setMonitor(EventMonitor* mon)
  {
    outMon=mon;
  }

  void setInterfaces(SmppCommandQueue* q,SmppChannelRegistrator* r)
  {
    cmdQueue=q;
    chReg=r;
  }

  void setSocketManager(SmppSMInterface* argSm)
  {
    sm=argSm;
  }

  bool isConnected()const
  {
    return connected;
  }

  void disconnect()
  {
    connected=false;
    sock->Close();
  }

  void processInput();

  bool wantToSend()
  {
    MutexGuard mg(mtx);
    return wrBufUsed>0 || outQueue.Count()>0;
  }
  void sendData();

  Socket* getSocket()
  {
    return sock;
  }

  SmppEntityType getType()const
  {
    return sockType;
  }

  SmppBindType getBindType()const
  {
    return bindType;
  }

  const char* getSystemId()const
  {
    return systemId.c_str();
  }

  virtual bool processPdu(PduGuard& pdu)=0;

  void putCommand(std::auto_ptr<SmppCommand> cmd)
  {
    MutexGuard mg(outMtx);
    outQueue.Push(cmd.release());
    if(outMon)outMon->notify();
  }

  bool checkTimeout(int to)
  {
    return time(NULL)-lastActivity>to;
  }

  void genEnquireLink(int to);

  virtual std::string getPeer()
  {
      if (!sock) return "";

      char buff[32];
      sock->GetPeer(buff);

      std::string str = buff;
      return str;
  }

protected:

  EventMonitor* outMon;
  SmppCommandQueue* cmdQueue;
  SmppChannelRegistrator* chReg;

  SmppSMInterface* sm;

  Socket* sock;
  Mutex mtx;
  int refCount;
  bool connected;

  std::string systemId;

  SmppBindType bindType;
  SmppEntityType sockType;

  time_t lastActivity;
  time_t lastEnquireLink;

  char* rdBuffer;
  int   rdToRead;
  int   rdBufSize;
  int   rdBufUsed;

  char* wrBuffer;
  int   wrBufSize;
  int   wrBufUsed;
  int   wrBufSent;

  CyclicQueue<SmppCommand*> outQueue;
  Mutex outMtx;

  smsc::logger::Logger* log;
  smsc::logger::Logger* dump;

  enum{DefaultBufferSize=4096};

  void Init()
  {
    lastActivity=time(NULL);
    outMon=0;
    sock=0;
    refCount=1;
    connected=false;
    sockType=etUnknown;
    bindType=btNone;
    rdBuffer=new char[DefaultBufferSize];
    rdBufSize=DefaultBufferSize;
    rdBufUsed=0;
    rdToRead=0;

    wrBuffer=new char[DefaultBufferSize];
    wrBufSize=DefaultBufferSize;
    wrBufUsed=0;
    wrBufSent=0;

    cmdQueue=0;

    log=smsc::logger::Logger::getInstance("smpp.io");
    dump=smsc::logger::Logger::getInstance("smpp.dmp");
  }

  SmppSocket(const SmppSocket&);
  void operator=(const SmppSocket&);
};

inline SmppSocket* getSmppSocket(Socket* sock)
{
  return (SmppSocket*)sock->getData(0);
}

}//smpp
}//transport
}//scag


#endif
