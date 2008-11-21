#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKET2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKET2_H__

#include <string>

#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"

#include "scag/transport/smpp/base/SmppCommandQueue2.h"
#include "scag/transport/smpp/base/SmppChannelRegistrator2.h"
#include "scag/transport/smpp/base/SmppChannel2.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "scag/transport/smpp/base/SmppSMInterface2.h"

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


struct SmppSocket : SmppChannel
{
    SmppSocket( Socket* s = 0 ) :
    outMon(0),
    cmdQueue(0),
    chReg(0),
    sm(0),
    sock(s),
    peer_(0),
    peerAddrFilled_(false),
    bindFailed_(false),
    refCount(1),
    connected(bool(s)),
    bindType(btNone),
    sockType(etUnknown),
    lastActivity(time(0)),
    lastEnquireLink(lastActivity),
    rdBuffer(0),
    rdToRead(0),
    rdBufSize(DefaultBufferSize),
    rdBufUsed(0),
    wrBuffer(0),
    wrBufSize(DefaultBufferSize),
    wrBufUsed(0),
    wrBufSent(0),
    log(0),
    dump(0)
    {
        rdBuffer = new char[rdBufSize];
        wrBuffer = new char[wrBufSize];
        log = smsc::logger::Logger::getInstance("smpp.io");
        dump = smsc::logger::Logger::getInstance("smpp.dmp");
        if ( s ) {
            smsc_log_debug( log, "SmppSocket init: %x", s );
            sock->setData(0,this);
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
    }
    smsc_log_debug(log, "release:%p(%s)/%d",this,systemId.c_str(),cnt);
    if(!cnt)
    {
      smsc_log_debug(log, "Deleting socket for %s(%s)", systemId.c_str(), getPeer());
      if(bindType!=btNone)
      {
        smsc_log_debug(log, "unregisterChannel(bt=%d)",bindType);
        chReg->unregisterChannel(this);
        bindType=btNone;
      }
      if (sm) sm->unregisterSocket(this);
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
      dropPeer();
  }

  /// NOTE: in case this method return false, the connection should be in bound state!
  virtual bool processPdu(PduGuard& pdu) = 0;

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

    bool hasBindFailed() const
    {
        return bindFailed_;
    }

  const char* getSystemId()const
  {
    return systemId.c_str();
  }

  void putCommand(std::auto_ptr<SmppCommand> cmd)
  {
    MutexGuard mg(outMtx);
    outQueue.Push(cmd.release());
    if(outMon)outMon->notify();
  }

  bool checkTimeout(int to)
  {
      return time(NULL) - lastActivity > to;
  }

  void genEnquireLink(int to);

    const char* getPeer() const {
        if ( ! sock ) return "";
        if ( ! peer_ ) {
            MutexGuard mg(const_cast<SmppSocket*>(this)->mtx);
            peer_ = new char[32];
            Socket::PrintAddrPort( getPeerAddress(), peer_ );
        }
        return peer_;
    }

    const sockaddr_in& getPeerAddress() const
    {
        if ( sock && !peerAddrFilled_ ) {
            MutexGuard mg(const_cast<SmppSocket*>(this)->mtx);
            peerAddrFilled_ = true;
            socklen_t len = sizeof(peerAddr_);
            getpeername(sock->getSocket(),(sockaddr*)&peerAddr_,&len); // no error checking
        }
        return peerAddr_;
    }

protected:
  virtual ~SmppSocket()
  {
      MutexGuard mg(mtx);
    smsc_log_debug(log, "SmmpSocket destroying: %x", sock);
    delete [] wrBuffer;
    wrBufUsed = 0;
    delete [] rdBuffer;
    rdBufUsed = 0;
    if(sock)delete sock;
      dropPeer();
    if ( outQueue.Count() > 0 ) {
        smsc_log_warn(log, "destructor: there are %u commands to send", outQueue.Count() );
        SmppCommand* cmd;
        while ( outQueue.Pop(cmd) )
            delete cmd;
    }
  }

    inline void dropPeer() {
        if (peer_) { delete[] peer_; peer_ = 0; }
    }


    SmppSocket(const SmppSocket&);
    void operator=(const SmppSocket&);

protected:
    enum {DefaultBufferSize=4096};

protected:
    EventMonitor* outMon;
    SmppCommandQueue* cmdQueue;
    SmppChannelRegistrator* chReg;
    SmppSMInterface* sm;
    Socket* sock;
    mutable char* peer_;
    mutable sockaddr_in peerAddr_;
    mutable bool        peerAddrFilled_;
    bool                bindFailed_;
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

};

inline SmppSocket* getSmppSocket(Socket* sock)
{
  return (SmppSocket*)sock->getData(0);
}

}//smpp
}//transport
}//scag


#endif
