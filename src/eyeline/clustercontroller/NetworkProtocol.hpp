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
  void setConnNodeIdx(int connId,int nodeIdx);
  void markConfigAsLoaded(int connId,ConfigType ct);

  void getConnIdsOfType(ConnType ct,std::vector<int>& ids);

  template <class CommandType>
  void enqueueCommand(int connId,CommandType& cmd,bool fillSeq=true)
  {
    protocol::ControllerProtocol cp;
    protogen::framework::SerializerBuffer sb(128);
    sync::MutexGuard mg(outQueueMon);
    if(fillSeq)
    {
      cmd.setSeqNum(outSeqId++);
    }
    cp.encodeMessage(cmd,&sb);
    Packet p(sb.detachBuffer(),sb.getDataWritten(),connId);
    outQueue.Push(p);
  }

  template <class CommandType>
  size_t enqueueCommandToType(ConnType ct,CommandType& cmd,int exceptId=-1)
  {
    std::vector<int> ids;
    getConnIdsOfType(ct,ids);
    for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;it++)
    {
      if(*it!=exceptId)
      {
        enqueueCommand(*it,cmd);
      }
    }
    return ids.size();
  }

  template <class CommandType,class RespType>
  size_t enqueueMultirespCommand(int srcConnId,CommandType& cmd,ConfigType ct)
  {
    sync::MutexGuard mg(mrMon);
    smsc_log_debug(log,"sending multiresp command from connId=%d",srcConnId);
    std::vector<int> ids;
    getConnIdsOfType(ctSmsc,ids);
    MultiRespInfo<RespType>* respInfo=new MultiRespInfo<RespType>();
    respInfo->connId=srcConnId;
    respInfo->seq=cmd.getSeqNum();
    respInfo->count=0;
    for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;it++)
    {
      SocketsMap::iterator sit=clnts.find(*it);
      if(sit==clnts.end())continue;
      ProtocolSocket* ps=sit->second;
      if(ps->isConfigLoaded(ct))
      {
        enqueueCommand(*it,cmd);
        RespKey key(*it,cmd.getSeqNum());
        smsc_log_debug(log,"enqueued command to connId=%d",*it);
        multiResps.insert(MultiRespMap::value_type(key,respInfo));
        respInfo->count++;
      }
    }
    return respInfo->count++;
  }

  template <class MultiResp>
  void registerMultiResp(int connId,const MultiResp& msg)
  {
    sync::MutexGuard mg(mrMon);
    const protocol::messages::MultiResponse& resp=msg.getResp();
    MultiRespMap::iterator it=multiResps.find(RespKey(connId,msg.getSeqNum()));
    if(it==multiResps.end())
    {
      smsc_log_warn(log,"Received mutliresp, but no record found from connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    if(resp.getStatus().empty() || resp.getIds().empty())
    {
      smsc_log_warn(log,"Received invalid mutliresp from connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    it->second->statuses.push_back(resp.getStatus().front());
    it->second->ids.push_back(resp.getIds().front());
    if(it->second->statuses.size()==it->second->count)
    {
      it->second->send();
      delete it->second;
      multiResps.erase(it);
    }
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

  struct MultiRespInfoBase{
    int connId;
    uint32_t seq;
    size_t count;
    std::vector<uint32_t> statuses;
    std::vector<uint8_t> ids;
    virtual ~MultiRespInfoBase(){}
    virtual void send()=0;
  };

  template <class T>
  struct MultiRespInfo:MultiRespInfoBase{
    void send()
    {
      T msg;
      protocol::messages::MultiResponse resp;
      resp.setIds(ids);
      resp.setStatus(statuses);
      msg.setResp(resp);
      msg.setSeqNum(seq);
      NetworkProtocol::getInstance()->enqueueCommand(connId,msg);
    }
  };

  struct RespKey{
    int connId;
    int seqNum;
    RespKey(int argConnId,int argSeqNum):connId(argConnId),seqNum(argSeqNum)
    {

    }
    bool operator<(const RespKey& argOther)const
    {
      return connId<argOther.connId || (connId==argOther.connId && seqNum<argOther.seqNum);
    }
  };
  typedef std::map<RespKey,MultiRespInfoBase*> MultiRespMap;
  sync::EventMonitor mrMon;
  MultiRespMap multiResps;
};

}
}

#endif
