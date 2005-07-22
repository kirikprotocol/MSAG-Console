#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKET_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKET_H__

#include <string>

#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"

#include "SmppCommandQueue.h"
#include "SmppChannelRegistrator.h"
#include "SmppChannel.h"
#include "SmppTypes.h"

#include "SmppSMInterface.h"

#include "logger/Logger.h"

namespace scag{
namespace transport{
namespace smpp{

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


namespace sync=smsc::core::synchronization;
namespace net=smsc::core::network;
namespace buf=smsc::core::buffers;

struct SmppSocket:SmppChannel{

  SmppSocket()
  {
    Init();
  }

  SmppSocket(net::Socket* s)
  {
    Init();
    sock=s;
    connected=true;
    sock->setData(0,this);
  }

  virtual ~SmppSocket()
  {
    delete [] wrBuffer;
    delete [] rdBuffer;
  }

  void acquire()
  {
    MutexGuard mg(mtx);
    refCount++;
    info2(log,"acquire:%p(%s)/%d",this,systemId.c_str(),refCount);
  }

  void release()
  {
    int cnt;
    {
      MutexGuard mg(mtx);
      cnt=--refCount;
      info2(log,"release:%p(%s)/%d",this,systemId.c_str(),refCount);
    }
    if(!cnt)
    {
      info2(log,"Deleting socket for %s",systemId.c_str());
      if(bindType!=btNone)
      {
        info2(log,"unregisterChannel(bt=%d)",bindType);
        chReg->unregisterChannel(this);
        bindType=btNone;
      }
      sm->unregisterSocket(this);
      delete this;
    }
  }

  void setMonitor(sync::EventMonitor* mon)
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
  }

  void processInput();

  bool wantToSend()
  {
    MutexGuard mg(mtx);
    return wrBufUsed>0 || outQueue.Count()>0;
  }
  void sendData();

  net::Socket* getSocket()
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

  void putCommand(const SmppCommand& cmd)
  {
    sync::MutexGuard mg(outMtx);
    outQueue.Push(cmd);
    if(outMon)outMon->notify();
  }

protected:

  sync::EventMonitor* outMon;
  SmppCommandQueue* cmdQueue;
  SmppChannelRegistrator* chReg;

  SmppSMInterface* sm;

  net::Socket* sock;
  sync::Mutex mtx;
  int refCount;
  bool connected;

  std::string systemId;

  SmppBindType bindType;
  SmppEntityType sockType;

  time_t lastActivity;

  char* rdBuffer;
  int   rdToRead;
  int   rdBufSize;
  int   rdBufUsed;

  char* wrBuffer;
  int   wrBufSize;
  int   wrBufUsed;
  int   wrBufSent;

  buf::CyclicQueue<SmppCommand> outQueue;
  sync::Mutex outMtx;

  smsc::logger::Logger* log;

  enum{DefaultBufferSize=4096};

  void Init()
  {
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

  }

  SmppSocket(const SmppSocket&);
  void operator=(const SmppSocket&);
};

SmppSocket* getSmppSocket(net::Socket* sock)
{
  return (SmppSocket*)sock->getData(0);
}

}//smpp
}//transport
}//scag


#endif
