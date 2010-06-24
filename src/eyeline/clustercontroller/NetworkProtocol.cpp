#include "NetworkProtocol.hpp"
#include "util/config/Manager.h"
#include "util/Exception.hpp"
#include "protocol/ControllerProtocolHandler.hpp"
#include "eyeline/clustercontroller/ConfigLockManager.hpp"

namespace eyeline{
namespace clustercontroller{

namespace cfg=smsc::util::config;

NetworkProtocol* NetworkProtocol::instance=0;

void NetworkProtocol::Init()
{
  if(instance)
  {
    throw Exception("Duplicate NetworkProtocol Init invocation.");
  }
  instance=new NetworkProtocol();
  instance->innerInit();
}

void NetworkProtocol::Deinit()
{
  delete instance;
  instance=0;
}

NetworkProtocol* NetworkProtocol::getInstance()
{
  return instance;
}

void NetworkProtocol::setConnNodeIdx(int connId,int nodeIdx)
{
  sync::MutexGuard mg(clntsMon);
  SocketsMap::iterator it=clnts.find(connId);
  if(it==clnts.end())
  {
    smsc_log_warn(log,"Attempt to set nodeIdx for non-existent connId=%d",connId);
    return;
  }
  it->second->setNodeIdx(nodeIdx);
}

void NetworkProtocol::setConnType(int connId,ConnType ct)
{
  sync::MutexGuard mg(clntsMon);
  SocketsMap::iterator it=clnts.find(connId);
  if(it==clnts.end())
  {
    smsc_log_warn(log,"Attempt to set connType for non-existent connId=%d",connId);
    return;
  }
  it->second->setConnectType(ct);
}

void NetworkProtocol::markConfigAsLoaded(int connId,ConfigType ct)
{
  sync::MutexGuard mg(clntsMon);
  SocketsMap::iterator it=clnts.find(connId);
  if(it==clnts.end())
  {
    smsc_log_warn(log,"Attempt to set connType for non-existent connId=%d",connId);
    return;
  }
  it->second->markConfigAsLoaded(ct);
}


void NetworkProtocol::getConnIdsOfType(ConnType ct,std::vector<int>& ids)
{
  sync::MutexGuard mg(clntsMon);
  for(SocketsMap::iterator it=clnts.begin(),end=clnts.end();it!=end;it++)
  {
    if(it->second->getConnectType()==ct)
    {
      ids.push_back(it->second->getConnId());
    }
  }
}


void NetworkProtocol::innerInit()
{
  cfg::Config& cfg=cfg::Manager::getInstance().getConfig();
  const char* host=cfg.getString("listener.host");
  int port=cfg.getInt("listener.port");
  handlersCount=cfg.getInt("listener.handlersCount");
  try{
    respTimeout=cfg.getInt("listener.respTimeout");
  }catch(...)
  {
    smsc_log_warn(log,"listener.respTimeout not found. Defaulting to 60.");
    respTimeout=60;
  }
  if(handlersCount<1)
  {
    handlersCount=1;
    smsc_log_warn(log,"listener.handlersCount cannot be less than 1");
  }
  if(handlersCount>32)
  {
    handlersCount=32;
    smsc_log_warn(log,"listener.handlersCount cannot be greater than 32");
  }
  if(srvSocket.InitServer(host,port,60,1,true)==-1)
  {
    throw smsc::util::Exception("Failed to init listener at %s:%d",host,port);
  }
  if(srvSocket.StartServer()==-1)
  {
    throw smsc::util::Exception("Failed to start listener at %s:%d",host,port);
  }
  rdmp.add(&srvSocket);
  rdThread.assignProto(this);
  wrThread.assignProto(this);
  tmThread.assignProto(this);
  rdThread.Start();
  wrThread.Start();
  tmThread.Start();
  for(int i=0;i<handlersCount;i++)
  {
    handlers[i].idx=i;
    handlers[i].assignProto(this);
    handlers[i].Start();
  }
}

NetworkProtocol::~NetworkProtocol()
{
  isStopping=true;
  rdThread.WaitFor();
  wrThread.WaitFor();
  tmThread.WaitFor();
  for(int i=0;i<handlersCount;i++)handlers[i].WaitFor();
}


void NetworkProtocol::readPackets()
{
  net::Multiplexer::SockArray rd,err;
  int hIdx=0;
  int cnt=0;
  while(!isStopping)
  {
    if(rdmp.canRead(rd,err,500))
    {
      for(int i=0;i<rd.Count();i++)
      {
        if(rd[i]==&srvSocket)
        {
          net::Socket* clnt=srvSocket.Accept();
          if(clnt)
          {
            char buf[64];
            clnt->GetPeer(buf);
            ProtocolSocket* ps=new ProtocolSocket(clnt,lastId++);
            smsc_log_info(log,"Accepted connect from %s, new connId=%d",buf,ps->getConnId());
            sync::MutexGuard mg(clntsMon);
            clnts.insert(SocketsMap::value_type(ps->getConnId(),ps));
            rdmp.add(clnt);
          }
          continue;
        }
        ProtocolSocket* ps=(ProtocolSocket*)rd[i]->getData(0);
        try{
          if(ps->Read())
          {
            MutexGuard hmg(handlers[hIdx].mon);
            Packet pck=ps->getPacket();
            smsc_log_debug(log,"read packet:%s",pck.getDump().c_str());
            handlers[hIdx].queue.Push(pck);
            handlers[hIdx].mon.notify();
            ps->resetPacket();
            hIdx++;
            hIdx%=handlersCount;
          }
        }catch(std::exception& e)
        {
          smsc_log_warn(log,"Exception during socket read operation, connId=%d:'%s'",ps->getConnId(),e.what());
          ps->markAsDead();
          rdmp.remove(ps->getSocket());
        }
      }
      for(int i=0;i<err.Count();i++)
      {
        ProtocolSocket* ps=(ProtocolSocket*)rd[i]->getData(0);
        ps->markAsDead();
      }
    }
    cnt++;
    if((cnt%10)==0)
    {
      sync::MutexGuard mg(clntsMon);
      std::vector<SocketsMap::iterator> toKill;
      for(SocketsMap::iterator it=clnts.begin(),end=clnts.end();it!=end;it++)
      {
        if(it->second->isDead())
        {
          while(it->second->getInOutMul())
          {
            clntsMon.wait(10);
          }
          toKill.push_back(it);
        }
      }
      for(std::vector<SocketsMap::iterator>::iterator it=toKill.begin(),end=toKill.end();it!=end;it++)
      {
        int connId=(*it)->second->getConnId();
        smsc_log_info(log,"Deleting connId=%d",connId);
        ConfigLockManager::getInstance()->UnlockByConn(connId);
        delete (*it)->second;
        clnts.erase(*it);
      }
    }
  }
}

void NetworkProtocol::writePackets()
{
  net::Multiplexer::SockArray wr,err;
  net::Multiplexer outMp;
  int cnt=0;
  while(!isStopping)
  {
    {
      sync::MutexGuard mg(outQueueMon);
      while(!isStopping && outQueue.Count()==0 && outMp.getSize()==0)
      {
        outQueueMon.wait(200);
      }
      if(outQueue.Count()==0 && outMp.getSize()==0)
      {
        continue;
      }
      if(outQueue.Count()>0)
      {
        sync::MutexGuard mg2(clntsMon);
        while(outQueue.Count()>0)
        {
          Packet p=outQueue.Front();
          SocketsMap::iterator it=clnts.find(p.connId);
          if(it==clnts.end())
          {
            smsc_log_debug(log,"Packet disposed for connId:%d, socket not found",p.connId);
            p.dispose();
            outQueue.Pop();
            continue;
          }
          outQueue.Pop();
          ProtocolSocket& ps=*it->second;
          if(ps.isDead())
          {
            p.dispose();
            continue;
          }

          if(ps.getInOutMul())
          {
            ps.enqueueOutPacket(p);
          }else
          {
            ps.setOutPacket(p);
            outMp.add(ps.getSocket());
            ps.setInOutMul(true);
          }
        }
      }
    }
    if(outMp.getSize() && outMp.canWrite(wr,err,500))
    {
      for(int i=0;i<wr.Count();i++)
      {
        ProtocolSocket* ps=(ProtocolSocket*)wr[i]->getData(0);
        try{
          if(ps->Write())
          {
            outMp.remove(ps->getSocket());
            ps->setInOutMul(false);
          }
        }catch(std::exception& e)
        {
          smsc_log_info(log,"Exception on write in connId=%d:'%s'",ps->getConnId(),e.what());
          outMp.remove(ps->getSocket());
          ps->markAsDead();
          ps->setInOutMul(false);
        }
      }
      for(int i=0;i<err.Count();i++)
      {
        ProtocolSocket* ps=(ProtocolSocket*)err[i]->getData(0);
        outMp.remove(ps->getSocket());
        ps->setInOutMul(false);
        ps->markAsDead();
      }
    }
    cnt++;
    if((cnt%10)==0)
    {
      sync::MutexGuard mg2(clntsMon);
      bool needNotify=false;
      for(SocketsMap::iterator it=clnts.begin(),end=clnts.end();it!=end;it++)
      {
        if(it->second->getInOutMul() && it->second->isDead())
        {
          outMp.remove(it->second->getSocket());
          it->second->setInOutMul(false);
          needNotify=true;
        }
      }
      if(needNotify)
      {
        clntsMon.notify();
      }
    }
  }
}

void NetworkProtocol::handleCommands(int idx)
{
  sync::MutexGuard mg(handlers[idx].mon);
  while(!isStopping)
  {
    while(!isStopping && handlers[idx].queue.Count()==0)
    {
      handlers[idx].mon.wait(200);
    }
    if(handlers[idx].queue.Count()==0)
    {
      continue;
    }
    Packet p;
    handlers[idx].queue.Pop(p);
    protocol::ControllerProtocolHandler cph(p.connId,log);
    protocol::ControllerProtocol cp;
    cp.assignHandler(&cph);
    try{
      cp.decodeAndHandleMessage(p.data,p.dataSize);
    }catch(std::exception& e)
    {
      smsc_log_warn(log,"Exception during command handling:%s",e.what());
    }
    p.dispose();
  }
}

void NetworkProtocol::processTimers()
{
  sync::MutexGuard mg(respMon);
  while(!isStopping)
  {
    if(!respTimers.empty())
    {
      time_t now=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
      while(!respTimers.empty() && respTimers.begin()->first<now)
      {
        RespKey key=respTimers.begin()->second;
        respTimers.erase(respTimers.begin());
        RespMap::iterator it=respMap.find(key);
        if(it==respMap.end())
        {
          continue;
        }
        try{
          if(it->second->isMultiResp())
          {
            MultiRespInfoBase* mr=(MultiRespInfoBase*)it->second;
            respMap.erase(it);
            mr->respIds.push_back(key.nodeIdx);
            mr->statuses.push_back(1);
            if(mr->respIds.size()==mr->reqIds.size())
            {
              mr->send();
              delete mr;
            }
          }else
          {
            NormalRespInfoBase* nr=(NormalRespInfoBase*)it->second;
            respMap.erase(it);
            nr->status=1;
            nr->send();
          }
        }catch(std::exception& e)
        {
          smsc_log_warn(log,"Exception during timers processing:%s",e.what());
        }
      }
    }
    respMon.wait(1000);
  }
}

int NetworkProtocol::ReaderThread::Execute()
{
  proto->readPackets();
  return 0;
}

int NetworkProtocol::WriterThread::Execute()
{
  proto->writePackets();
  return 0;
}

int NetworkProtocol::HandlerThread::Execute()
{
  proto->handleCommands(idx);
  return 0;
}

int NetworkProtocol::TimersThread::Execute()
{
  proto->processTimers();
  return 0;
}

bool ProtocolSocket::Read()
{
  static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("ps.rd");
  if(!havePacketSize)
  {
    int rd=sck->Read(rdBuffer+rdDataSize,4);
    if(rd<=0)
    {
      throw smsc::util::Exception("socket read failed");
    }
    rdDataSize+=rd;
    if(rdDataSize==4)
    {
      uint32_t pckSz;
      memcpy(&pckSz,rdBuffer,4);
      pckSz=ntohl(pckSz);
      smsc_log_debug(log,"received packet with size %d",pckSz);
      rdPacketSize=pckSz;
      if(rdBufferSize<rdPacketSize)
      {
        delete [] rdBuffer;
        rdBufferSize=rdPacketSize;
        rdBuffer=new char [rdBufferSize];
      }
      havePacketSize=true;
      rdDataSize=0;
    }
    return false;
  }
  int rd=sck->Read(rdBuffer+rdDataSize,(int)(rdPacketSize-rdDataSize));
  smsc_log_debug(log,"rd=%d",rd);
  if(rd==0)
  {
    throw smsc::util::Exception("socket closed");
  }
  if(rd<0)
  {
    throw smsc::util::Exception("socket read failed");
  }
  rdDataSize+=rd;
  return rdDataSize==rdPacketSize;
}

bool ProtocolSocket::Write()
{
  int wr=sck->Write(wrBuffer+wrDataWritten,(int)(wrBufferSize-wrDataWritten));
  if(wr==0)
  {
    throw smsc::util::Exception("socket closed");
  }
  if(wr<0)
  {
    throw smsc::util::Exception("socket write failed");
  }
  wrDataWritten+=wr;
  if(wrBufferSize==wrDataWritten)
  {
    delete [] wrBuffer;
    if(outQueue.Count())
    {
      Packet p;
      outQueue.Pop(p);
      wrBuffer=p.data;
      wrBufferSize=p.dataSize;
      wrDataWritten=0;
    }
  }
  return wrBufferSize==wrDataWritten;
}


}
}
