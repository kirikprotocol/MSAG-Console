#ifndef __EYELINE_CLUSTERCONTROLLER_ServerBase_HPP__
#define __EYELINE_CLUSTERCONTROLLER_ServerBase_HPP__

#include <map>
#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/64bitcompat.h"
#include "ProtocolSocketBase.hpp"
#include "logger/Logger.h"
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
//#include "util/TimeSource.h"

namespace eyeline{
namespace protogen{

namespace thr=smsc::core::threads;
namespace net=smsc::core::network;
namespace sync=smsc::core::synchronization;
namespace buf=smsc::core::buffers;



class ServerBase{
public:
  ServerBase(const char* logName)
  {
    lastId=0;
    log=smsc::logger::Logger::getInstance(logName);
    outSeqId=0;
  }
  ~ServerBase();
  void Init(const char* host,int port,int hndCnt);
  void Stop();

  template <class CommandType,class ProtocolType>
  void enqueueCommand(int connId,CommandType& cmd,ProtocolType& proto,bool fillSeq=true)
  {
    ::protogen::framework::SerializerBuffer sb(128);
    sync::MutexGuard mg(outQueueMon);
    if(fillSeq)
    {
      cmd.setSeqNum(outSeqId++);
    }
    proto.encodeMessage(cmd,&sb);
    ProtocolSocketBase::Packet p(sb.detachBuffer(),sb.getDataWritten(),connId);
    outQueue.Push(p);
  }

protected:


  virtual ProtocolSocketBase* onConnect(net::Socket* clnt,int connId)=0;
  virtual void onHandleCommand(ProtocolSocketBase::Packet& pkt)=0;
  virtual void onDisconnect(ProtocolSocketBase* sck)=0;

  void readPackets();
  void writePackets();
  void handleCommands(int idx);

  friend class ReaderThread;
  friend class WriterThread;
  friend class HandlerThread;

  class MethodRunnerThread:public thr::Thread{
  protected:
    ServerBase* proto;
  public:
    void assignProto(ServerBase* argProto)
    {
      proto=argProto;
    }
  };

  class ReaderThread:public MethodRunnerThread{
  public:
    int Execute();
  }rdThread;
  class WriterThread:public MethodRunnerThread{
  public:
    int Execute();
  }wrThread;
  class HandlerThread:public MethodRunnerThread{
  public:
    int Execute();
    buf::CyclicQueue<ProtocolSocketBase::Packet> queue;
    sync::EventMonitor mon;
    int idx;
  };
  HandlerThread handlers[32];
  int handlersCount;

  net::Socket srvSocket;
  net::Multiplexer rdmp,wrmp;
  typedef std::map<int,ProtocolSocketBase*> SocketsMap;
  sync::EventMonitor clntsMon;
  SocketsMap clnts;
  bool isStopping;
  int outSeqId;

  smsc::logger::Logger* log;

  int lastId;

  sync::EventMonitor outQueueMon;
  buf::CyclicQueue<ProtocolSocketBase::Packet> outQueue;

};

}
}

#endif
