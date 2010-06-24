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
#include "util/TimeSource.h"
#include "NetworkProtocol.hpp"

namespace eyeline{
namespace clustercontroller{

namespace thr=smsc::core::threads;
namespace net=smsc::core::network;
namespace sync=smsc::core::synchronization;
namespace buf=smsc::core::buffers;



class NetworkProtocol{
public:
  NetworkProtocol()
  {
    lastId=0;
    log=smsc::logger::Logger::getInstance("net.proto");
    outSeqId=0;
    isStopping=false;
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
  bool enqueueCommandAnyOfType(ConnType ct,int srcConnId,CommandType& cmd)
  {
    std::vector<int> ids;
    getConnIdsOfType(ct,ids);
    if(!ids.empty())
    {
      enqueueCommand(ids.front(),cmd);
      time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
      sync::MutexGuard mg(respMon);
      RespInfo<RespType>* respInfo=new RespInfo<RespType>();
      respInfo->connId=srcConnId;
      respInfo->seq=cmd.getSeqNum();
      RespKey key(srcConnId,cmd.getSeqNum());
      respMap.insert(RespMap::value_type(key,respInfo));
      respTimers.insert(RespTimerMap::value_type(now+respTimeout,key));
    }
    return !ids.empty();
  }


  template <class CommandType,class RespType>
  size_t enqueueMultirespCommand(int srcConnId,CommandType& cmd,ConfigType ct)
  {
    sync::MutexGuard mg(respMon);
    smsc_log_debug(log,"sending multiresp command from connId=%d",srcConnId);
    std::vector<int> ids;
    getConnIdsOfType(ctSmsc,ids);
    MultiRespInfo<RespType>* respInfo=new MultiRespInfo<RespType>();
    respInfo->connId=srcConnId;
    respInfo->seq=cmd.getSeqNum();
    time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
    sync::MutexGuard mg2(clntsMon);
    for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;it++)
    {
      SocketsMap::iterator sit=clnts.find(*it);
      if(sit==clnts.end())continue;
      ProtocolSocket* ps=sit->second;
      if(ps->isConfigLoaded(ct))
      {
        enqueueCommand(*it,cmd);
        RespKey key(*it,cmd.getSeqNum(),ps->getNodeIdx());
        smsc_log_debug(log,"enqueued command to connId=%d",*it);
        respInfo->reqIds.push_back(key.nodeIdx);
        respMap.insert(RespMap::value_type(key,respInfo));
        respTimers.insert(RespTimerMap::value_type(now+respTimeout,key));
      }
    }
    return respInfo->reqIds.size();
  }

  template <class CommandType,class RespType>
  size_t enqueueMultirespCommandEx(int srcConnId,CommandType& cmd,RespType& resp,ConfigType ct)
  {
    MultiRespInfoEx<RespType>* respInfo=new MultiRespInfoEx<RespType>(resp);
    sync::MutexGuard mg(respMon);
    smsc_log_debug(log,"sending multiresp command from connId=%d",srcConnId);
    std::vector<int> ids;
    getConnIdsOfType(ctSmsc,ids);
    respInfo->connId=srcConnId;
    respInfo->seq=cmd.getSeqNum();
    time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
    sync::MutexGuard mg2(clntsMon);
    for(std::vector<int>::iterator it=ids.begin(),end=ids.end();it!=end;it++)
    {
      SocketsMap::iterator sit=clnts.find(*it);
      if(sit==clnts.end())continue;
      ProtocolSocket* ps=sit->second;
      if(ps->isConfigLoaded(ct))
      {
        enqueueCommand(*it,cmd);
        RespKey key(*it,cmd.getSeqNum(),ps->getNodeIdx());
        smsc_log_debug(log,"enqueued command to connId=%d",*it);
        respInfo->reqIds.push_back(key.nodeIdx);
        respMap.insert(RespMap::value_type(key,respInfo));
        respTimers.insert(RespTimerMap::value_type(now+respTimeout,key));
      }
    }
    return respInfo->reqIds.size();
  }

  template <class RespType>
  void createGatherReq(int srcConnId,int srcSeqNum,const RespType& respMsg)
  {
    GatherRespInfo<RespType>* respInfo=new GatherRespInfo<RespType>(respMsg);
    time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
    sync::MutexGuard mg(respMon);
    respInfo->connId=srcConnId;
    respInfo->seq=srcSeqNum;
    RespKey key(srcConnId,srcSeqNum);
    respMap.insert(RespMap::value_type(key,respInfo));
    respTimers.insert(RespTimerMap::value_type(now+respTimeout,key));
  }

  void addToGatherReq(int srcConnId,int srcSeqNum,int dstConnId,int dstSeqNum)
  {
    sync::MutexGuard mg(respMon);
    RespKey key(srcConnId,srcSeqNum);
    RespMap::iterator it=respMap.find(key);
    if(it==respMap.end())
    {
      smsc_log_warn(log,"Gather resp not found for connId=%d, seqNum=%d",srcConnId,srcSeqNum);
      return;
    }
    if(!it->second->isGatherResp())
    {
      smsc_log_warn(log,"Resp is not gather resp for connId=%d, seqNum=%d",srcConnId,srcSeqNum);
      return;
    }
    GatherRespInfoBase* respInfo=(GatherRespInfoBase*)it->second;

    respInfo->reqInfo.push_back(GatherRespInfoBase::GatherMsgInfo(dstConnId,dstSeqNum));
    RespKey key2(dstConnId,dstSeqNum);
    respMap.insert(RespMap::value_type(key2,respInfo));
  }

  void finishGatherReq(int srcConnId,int srcSeqNum)
  {
    sync::MutexGuard mg(respMon);
    RespKey key(srcConnId,srcSeqNum);
    RespMap::iterator it=respMap.find(key);
    if(it==respMap.end())
    {
      smsc_log_warn(log,"Gather resp not found for connId=%d, seqNum=%d",srcConnId,srcSeqNum);
      return;
    }
    if(!it->second->isGatherResp())
    {
      smsc_log_warn(log,"Resp is not gather resp for connId=%d, seqNum=%d",srcConnId,srcSeqNum);
      return;
    }
    GatherRespInfoBase* respInfo=(GatherRespInfoBase*)it->second;
    respInfo->allRequestsMade=true;
    bool allDone=true;
    for(size_t i=0;i<respInfo->reqInfo.size();i++)
    {
      if(!respInfo->reqInfo[i].completed)
      {
        allDone=false;
        break;
      }
    }
    if(allDone)
    {
      respInfo->send();
      delete respInfo;
      respMap.erase(it);
    }
  }

  template <class UpdateOp>
  void updateGatherResp(int dstConnId,int dstSeqNum,UpdateOp& op)
  {
    sync::MutexGuard mg(respMon);
    RespKey key(dstConnId,dstSeqNum);
    RespMap::iterator it=respMap.find(key);
    if(it==respMap.end())
    {
      smsc_log_warn(log,"Gather resp not found for connId=%d, seqNum=%d",dstConnId,dstSeqNum);
      return;
    }
    if(!it->second->isGatherResp())
    {
      smsc_log_warn(log,"Resp is not gather resp for connId=%d, seqNum=%d",dstConnId,dstSeqNum);
      return;
    }
    GatherRespInfoBase::GatherMsgInfo info(dstConnId,dstSeqNum);
    GatherRespInfo<typename UpdateOp::RespType>* respInfo=((GatherRespInfo<typename UpdateOp::RespType>*)it->second);
    op(respInfo->respMsg);
    for(size_t i=0;i<respInfo->reqInfo.size();i++)
    {
      if(respInfo->reqInfo[i]==info)
      {
        respInfo->reqInfo[i].completed=true;
        break;
      }
    }
    bool allDone=true;
    for(size_t i=0;i<respInfo->reqInfo.size();i++)
    {
      if(!respInfo->reqInfo[i].completed)
      {
        allDone=false;
        break;
      }
    }
    if(allDone)
    {
      RespMap::iterator it2=respMap.find(RespKey(respInfo->connId,respInfo->seq));
      if(it2!=respMap.end())
      {
        respMap.erase(it2);
      }
      respInfo->send();
      delete respInfo;
      respMap.erase(it);
    }
  }

  template <class MultiResp>
  void registerMultiResp(int connId,const MultiResp& msg)
  {
    sync::MutexGuard mg(respMon);
    const protocol::messages::MultiResponse& resp=msg.getResp();
    RespMap::iterator it=respMap.find(RespKey(connId,msg.getSeqNum()));
    if(it==respMap.end())
    {
      smsc_log_warn(log,"Received mutliresp, but no record found from connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    if(!it->second->isMultiResp())
    {
      smsc_log_warn(log,"Received mutliresp, but original request wasn't multireq connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    if(resp.getStatus().empty() || resp.getIds().empty())
    {
      smsc_log_warn(log,"Received invalid mutliresp from connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    MultiRespInfoBase* mr=(MultiRespInfoBase*)it->second;
    mr->statuses.push_back(resp.getStatus().front());
    mr->respIds.push_back(resp.getIds().front());
    if(mr->statuses.size()==mr->reqIds.size())
    {
      RespMap::iterator it2=respMap.find(RespKey(mr->connId,mr->seq));
      if(it2!=respMap.end())
      {
        respMap.erase(it2);
      }
      it->second->send();
      delete it->second;
    }
    respMap.erase(it);
  }

  template <class RespType>
  void registerResp(int connId,const RespType& msg)
  {
    sync::MutexGuard mg(respMon);
    RespMap::iterator it=respMap.find(RespKey(connId,msg.getSeqNum()));
    if(it==respMap.end())
    {
      smsc_log_warn(log,"Received resp, but no record found from connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    if(it->second->isMultiResp())
    {
      smsc_log_warn(log,"Received resp, but original request was multireq connId=%d, seqNum=%d",connId,msg.getSeqNum());
      return;
    }
    NormalRespInfoBase* nr=(NormalRespInfoBase*)it->second;
    nr->status=msg.getResp().getStatus();
    nr->send();
    delete it->second;
    respMap.erase(it);
  }

protected:

  static NetworkProtocol* instance;

  void innerInit();

  void readPackets();
  void writePackets();
  void handleCommands(int idx);
  void processTimers();

  friend class ReaderThread;
  friend class WriterThread;
  friend class HandlerThread;

  class MethodRunnerThread:public thr::Thread{
  protected:
    NetworkProtocol* proto;
  public:
    void assignProto(NetworkProtocol* argProto)
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
    buf::CyclicQueue<Packet> queue;
    sync::EventMonitor mon;
    int idx;
  };
  HandlerThread handlers[32];
  int handlersCount;

  class TimersThread:public MethodRunnerThread{
  public:
    int Execute();
  }tmThread;

  net::Socket srvSocket;
  net::Multiplexer rdmp,wrmp;
  typedef std::map<int,ProtocolSocket*> SocketsMap;
  sync::EventMonitor clntsMon;
  SocketsMap clnts;
  bool isStopping;
  int outSeqId;

  smsc::logger::Logger* log;

  int lastId;

  time_t respTimeout;

  sync::EventMonitor outQueueMon;
  buf::CyclicQueue<Packet> outQueue;

  struct RespInfoBase{
    int connId;
    uint32_t seq;
    virtual ~RespInfoBase(){}
    virtual void send()=0;
    virtual bool isMultiResp()const
    {
      return false;
    }
    virtual bool isGatherResp()const
    {
      return false;
    }
  };
  struct MultiRespInfoBase:RespInfoBase{
    std::vector<int8_t> reqIds;
    std::vector<int32_t> statuses;
    std::vector<int8_t> respIds;
    virtual ~MultiRespInfoBase(){}
  };

  template <class T>
  struct MultiRespInfo:MultiRespInfoBase{
    void send()
    {
      T msg;
      protocol::messages::MultiResponse resp;
      resp.setIds(respIds);
      resp.setStatus(statuses);
      msg.setResp(resp);
      msg.setSeqNum(seq);
      NetworkProtocol::getInstance()->enqueueCommand(connId,msg);
    }
    bool isMultiResp()const
    {
      return true;
    }
  };

  template <class T>
  struct MultiRespInfoEx:MultiRespInfoBase{
    T msg;
    MultiRespInfoEx(const T& argMsg):msg(argMsg)
    {
    }
    void send()
    {
      protocol::messages::MultiResponse resp;
      resp.setIds(respIds);
      resp.setStatus(statuses);
      msg.setResp(resp);
      msg.setSeqNum(seq);
      NetworkProtocol::getInstance()->enqueueCommand(connId,msg);
    }
    bool isMultiResp()const
    {
      return true;
    }
  };


  struct NormalRespInfoBase:RespInfoBase{
    int status;
  };

  template <class T>
  struct RespInfo:NormalRespInfoBase{
    void send()
    {
      T msg;
      protocol::messages::Response resp;
      resp.setStatus(status);
      msg.setResp(resp);
      msg.setSeqNum(seq);
      NetworkProtocol::getInstance()->enqueueCommand(connId,msg);
    }
  };

  struct GatherRespInfoBase:RespInfoBase{
    struct GatherMsgInfo{
      int connId;
      int seqNum;
      bool completed;
      GatherMsgInfo(int argConnId,int argSeqNum):connId(argConnId),seqNum(argSeqNum),completed(false)
      {
      }
      bool operator==(const GatherMsgInfo& rhs)const
      {
        return connId==rhs.connId && seqNum==rhs.seqNum;
      }
    };
    std::vector<GatherMsgInfo> reqInfo;
    bool allRequestsMade;
    GatherRespInfoBase():allRequestsMade(false)
    {
    }
  };

  template <class T>
  struct GatherRespInfo:GatherRespInfoBase{
    T respMsg;
    GatherRespInfo(const T& argRespMsg):respMsg(argRespMsg)
    {
    }
    bool isGatherResp()const
    {
      return true;
    }
    void send()
    {
      protocol::messages::Response resp;
      resp.setStatus(0);
      respMsg.setResp(resp);
      NetworkProtocol::getInstance()->enqueueCommand(connId,respMsg);
    }
  };

  struct RespKey{
    int connId;
    int seqNum;
    int nodeIdx;
    RespKey(int argConnId,int argSeqNum,int argNodeIdx=0):connId(argConnId),seqNum(argSeqNum),nodeIdx(argNodeIdx)
    {

    }
    bool operator<(const RespKey& argOther)const
    {
      return connId<argOther.connId || (connId==argOther.connId && seqNum<argOther.seqNum);
    }
  };
  typedef std::map<RespKey,RespInfoBase*> RespMap;
  sync::EventMonitor respMon;
  RespMap respMap;
  typedef std::multimap<time_t,RespKey> RespTimerMap;
  RespTimerMap respTimers;
};

}
}

#endif
