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
    friend class eyeline::informer::EmbedRefPtr< SmppSocket >;

    SmppSocket( Socket* s = 0 ) :
    outMon(0),
    cmdQueue(0),
    chReg(0),
    sm(0),
    sock(s),
    bindFailed_(false),
    // refCount(1),
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
    dump(0)
    {
        rdBuffer = new char[rdBufSize];
        wrBuffer = new char[wrBufSize];
        dump = smsc::logger::Logger::getInstance("smpp.dmp");
        if ( s ) {
            smsc_log_debug( log_, "SmppSocket @%p ctor(%x)", this, s );
            sock->setData(0,this);
        } else {
            smsc_log_debug( log_, "SmppSocket @%p ctor()", this);
        }
        fillPeerData();
    }

  /*
  void acquire()
  {
    MutexGuard mg(mtx);
    refCount++;
    smsc_log_debug(log_, "acquire:%p(%s)/%d",this,systemId.c_str(),refCount);
  }
   */

    /*
  void release()
  {
    int cnt;
    {
      MutexGuard mg(mtx);
      cnt=--refCount;
    }
    smsc_log_debug(log_, "release:%p(%s)/%d",this,systemId.c_str(),cnt);
    if(!cnt)
    {
      smsc_log_debug(log_, "Deleting socket for %s(%s)", systemId.c_str(), getPeer());
      if(bindType!=btNone)
      {
        smsc_log_debug(log_, "unregisterChannel(bt=%d)",bindType);
        chReg->unregisterChannel(this);
        bindType=btNone;
      }
      if (sm) sm->unregisterSocket(this);
      delete this;
    }
  }
     */

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
      smsc_log_debug(log_,"SmppSocket @%p disconnect()",this);
      connected=false;
      sock->Close();
      // peername_ = "";
      // memset(&peeraddr_,0,sizeof(peeraddr_));
      if (bindType!=btNone) {
          chReg->unregisterChannel(this);
          bindType = btNone;
      }
      if (sm) {
          sm->unregisterSocket(this);
      }
      fillPeerData();
  }

  /// NOTE: in case this method return false, the connection should be in bound state!
  virtual bool processPdu(PduGuard& pdu) = 0;

  void processInput();

  bool wantToSend()
  {
      bool res;
      {
          MutexGuard mg(mtx);
          res = (wrBufUsed>0 || outQueue.Count()>0);
      }
      if (res) {
          smsc_log_debug(log_,"SmppSocket @%p wantToSend()",this);
      }
      return res;
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

    const sockaddr_in& getPeerAddress() const {
        return peerAddr_;
    }

protected:
    void fillPeerData() {
        MutexGuard mg(mtx);
        if ( sock && connected ) {
            socklen_t len = sizeof(peerAddr_);
            getpeername(sock->getSocket(),(sockaddr*)&peerAddr_,&len); // no error checking
            Socket::PrintAddrPort( getPeerAddress(), peername_.str );
        } else {
            memset(&peerAddr_,0,sizeof(peerAddr_));
            peername_ = "";
        }
    }


  virtual ~SmppSocket()
  {
      MutexGuard mg(mtx);
    smsc_log_debug(log_, "SmppSocket @%p destroying: %x", this, sock);
    delete [] wrBuffer;
    wrBufUsed = 0;
    delete [] rdBuffer;
    rdBufUsed = 0;
    if (sock) { delete sock; sock = 0; }
    // dropPeer();
    if ( outQueue.Count() > 0 ) {
        smsc_log_warn(log_, "destructor: there are %u commands to send", outQueue.Count() );
        SmppCommand* cmd;
        while ( outQueue.Pop(cmd) )
            delete cmd;
    }
  }


    SmppSocket(const SmppSocket&);
    void operator=(const SmppSocket&);

    void setSystemId( const char* regSysId );

protected:
    enum {DefaultBufferSize=4096};

protected:
    EventMonitor* outMon;
    SmppCommandQueue* cmdQueue;    // where to put incoming commands
    SmppChannelRegistrator* chReg; // where to register socket
    SmppSMInterface* sm;
    Socket* sock;
    sockaddr_in peerAddr_;
    bool                bindFailed_;
    Mutex mtx;
    // int refCount;
    bool connected;
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

    smsc::logger::Logger* dump;

private:
    /// we make it private to have controlled access
    std::string systemId;
};

inline SmppSocket* getSmppSocket(Socket* sock)
{
  return (SmppSocket*)sock->getData(0);
}

typedef eyeline::informer::EmbedRefPtr< SmppSocket >  SmppSocketPtr;

}//smpp
}//transport
}//scag


#endif
