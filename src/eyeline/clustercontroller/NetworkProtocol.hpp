#ifndef __EYELINE_CLUSTERCONTROLLER_NETWORKPROTOCOL_HPP__
#define __EYELINE_CLUSTERCONTROLLER_NETWORKPROTOCOL_HPP__

#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/64bitcompat.h"
#include <map>
#include "ProtocolSocket.hpp"
#include "logger/Logger.h"
#include "protocol/ControllerProtocol.hpp"

namespace eyeline{
namespace clustercontroller{

namespace thr=smsc::core::threads;
namespace net=smsc::core::network;
namespace sync=smsc::core::synchronization;
namespace buf=smsc::core::buffers;



class NetworkProtocol{
public:
  NetworkProtocol():rdThread(this),wrThread(this)
  {
    lastId=0;
    log=smsc::logger::Logger::getInstance("net.proto");
    outSeqId=0;
  }
  ~NetworkProtocol();
  static void Init();
  static void Deinit();
  static NetworkProtocol* getInstance();

  void setConnType(int connId,ConnType ct);

  void getConnIdsOfType(ConnType ct,std::vector<int>& ids);

  template <class CommandType>
  void enqueueCommand(int connId,CommandType& cmd)
  {
    protocol::ControllerProtocol cp;
    protogen::framework::SerializerBuffer sb(128);
    cp.encodeMessage(cmd,&sb);
    sync::MutexGuard mg(outQueueMon);
    cmd.setSeqNum(outSeqId++);
    Packet p(sb.detachBuffer(),sb.getDataWritten(),connId);
    outQueue.Push(p);
  }

  template <class CommandType>
  size_t enqueueCommandToType(ConnType ct,CommandType& cmd)
  {
    std::vector<int> ids;
    getConnIdsOfType(ct,ids);
    for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;it++)
    {
      enqueueCommand(*it,cmd);
    }
    return ids.size();
  }

protected:

  static NetworkProtocol* instance;

  void innerInit();

  void readPackets();
  void writePackets();
  void handleCommands(int idx);

  friend class ReaderThread;
  friend class WriterThread;
  friend class HandlerThread;
  class ReaderThread:public thr::Thread{
  public:
    ReaderThread(NetworkProtocol* argProto):proto(argProto)
    {

    }
    int Execute();
    NetworkProtocol* proto;
  }rdThread;
  class WriterThread:public thr::Thread{
  public:
    WriterThread(NetworkProtocol* argProto):proto(argProto)
    {

    }
    int Execute();
    NetworkProtocol* proto;
  }wrThread;
  class HandlerThread:public thr::Thread{
  public:
    void assignProto(NetworkProtocol* argProto)
    {
      proto=argProto;
    }
    int Execute();
    NetworkProtocol* proto;
    buf::CyclicQueue<Packet> queue;
    sync::EventMonitor mon;
    int idx;
  };
  HandlerThread handlers[32];
  int handlersCount;

  net::Socket srvSocket;
  net::Multiplexer rdmp,wrmp;
  typedef std::map<int,ProtocolSocket*> SocketsMap;
  sync::EventMonitor clntsMon;
  SocketsMap clnts;
  bool isStopping;
  int outSeqId;

  smsc::logger::Logger* log;

  int lastId;

  sync::EventMonitor outQueueMon;
  buf::CyclicQueue<Packet> outQueue;
};

}
}

#endif
