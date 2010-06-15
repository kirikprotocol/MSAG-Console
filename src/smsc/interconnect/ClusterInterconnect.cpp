/*
 * ClusterInterconnect.cpp
 *
 *  Created on: Feb 25, 2010
 *      Author: skv
 */

#include "smsc/interconnect/ClusterInterconnect.hpp"
#include "core/network/Multiplexer.hpp"
#include "sms/sms_serializer.h"

namespace smsc{
namespace interconnect{

ClusterInterconnect* ClusterInterconnect::instance=0;

ClusterInterconnect::ClusterInterconnect(int argOwnNodeIndex):
    ownNodeIndex(argOwnNodeIndex-1),nodesCount(0),remoteSocketsCount(0),
    monitor(0),seqCount(0),shutdown(false),connThr(this),readThr(this),writeThr(this)
{
  for(int i=0;i<maxNodesCount;i++)
  {
    connectedClnt[i]=0;
  }
  log=smsc::logger::Logger::getInstance("clst.icon");
}

ClusterInterconnect::~ClusterInterconnect()
{
  for(int i=0;i<remoteSocketsCount;i++)
  {
    delete remoteSockets[i];
  }
}

void ClusterInterconnect::Init(int argOwnIndex)
{
  if(instance)
  {
    throw smsc::util::Exception("Duplicate ClusterInterconnect init");
  }
  instance=new ClusterInterconnect(argOwnIndex);
}

ClusterInterconnect* ClusterInterconnect::getInstance()
{
  return instance;
}

void ClusterInterconnect::Start()
{
  if(srvSocket.InitServer(hosts[ownNodeIndex].c_str(),ports[ownNodeIndex],60,true,true)==-1)
  {
    throw smsc::util::Exception("Failed to init server at %s:%d",hosts[ownNodeIndex].c_str(),ports[ownNodeIndex]);
  }
  if(srvSocket.StartServer()==-1)
  {
    throw smsc::util::Exception("Failed to start server at %s:%d",hosts[ownNodeIndex].c_str(),ports[ownNodeIndex]);
  }
  for(int i=0;i<nodesCount;i++)
  {
    if(i==ownNodeIndex)continue;
    if(clntSockets[i].Init(hosts[i].c_str(),ports[i],60)==-1)
    {
      throw smsc::util::Exception("Failed to init socket for %s:%d",hosts[i].c_str(),ports[i]);
    }
  }


  instance->connThr.Start();
  instance->readThr.Start();
  instance->writeThr.Start();
}


void ClusterInterconnect::Deinit()
{
  instance->Shutdown();
  delete instance;
  instance=0;
}

void ClusterInterconnect::Shutdown()
{
  srvSocket.Abort();
  for(int i=0;i<maxNodesCount;i++)
  {
    if(connectedClnt[i])
    {
      clntSockets[i].Abort();
    }
  }
  for(int i=0;i<remoteSocketsCount;i++)
  {
    remoteSockets[i]->Abort();
  }
  shutdown=true;
  connThr.WaitFor();
  readThr.WaitFor();
  writeThr.WaitFor();
}



void ClusterInterconnect::putCommand(const smsc::smeman::SmscCommand& command)
{
  sync::MutexGuard mg(outMon);
  outQueue.Push(command);
  if(outQueue.Count()==1)
  {
    outMon.notify();
  }
}

bool ClusterInterconnect::getCommand(smsc::smeman::SmscCommand& cmd)
{
  sync::MutexGuard mg(inMon);
  return inQueue.Pop(cmd);
}

void ClusterInterconnect::attachMonitor(smsc::smeman::ProxyMonitor* argMonitor)
{
  monitor=argMonitor;
}

bool ClusterInterconnect::attached()
{
  return monitor!=0;
}

uint32_t ClusterInterconnect::getNextSequenceNumber()
{
  sync::MutexGuard mg(seqMtx);
  return ++seqCount;
}

void ClusterInterconnect::connectThread()
{
  while(!shutdown)
  {
    for(int i=0;i<nodesCount;i++)
    {
      if(i!=ownNodeIndex && !connectedClnt[i])
      {
        if(clntSockets[i].Connect()!=-1)
        {
          using namespace smsc::smeman;
          SmscCommand cmd=SmscCommand::makeCommand(CLUSTERICONIDENT,ownNodeIndex,0,0);
          cmd->dstNodeIdx=i;
          sync::MutexGuard mg(outMon);
          outQueue.Push(cmd);
          if(outQueue.Count()==1)
          {
            outMon.notify();
          }
          connectedClnt[i]=true;
          smsc_log_debug(log,"connected to node %d",i);
        }else
        {
          smsc_log_debug(log,"failed to connected to node %d",i);
        }
      }
    }
    smsc::util::millisleep(500);
  }
}


int ClusterInterconnect::getClntSocketIndex(net::Socket* sck)
{
  return (int)(sck-&clntSockets[0]);
}

int ClusterInterconnect::getRemSocketIndex(net::Socket* sck)
{
  for(int i=0;i<remoteSocketsCount;i++)
  {
    if(remoteSockets[i]==sck)
    {
      return i;
    }
  }
  return -1;
}



void ClusterInterconnect::readThread()
{
  net::Multiplexer mul;
  net::Multiplexer::SockArray rd,err;
  mul.add(&srvSocket);
  while(!shutdown)
  {
    if(!mul.canRead(rd,err,200))continue;
    for (int i=0;i< err.Count();++i)
    {
      int idx=getRemSocketIndex(err[i]);
      mul.remove(err[i]);
      remoteSockets[idx]->Abort();
      delete remoteSockets[idx];
      remoteSockets[idx]=0;
    }
    for(int i=0;i<rd.Count();i++)
    {
      if(rd[i]==&srvSocket)
      {
        net::Socket* newClnt=srvSocket.Accept(60);
        char peerBuf[127];
        newClnt->GetPeer(peerBuf);
        smsc_log_info(log,"accepted connection from %s, remoteSocketsCount=%d",peerBuf,remoteSocketsCount);
        if(remoteSocketsCount==maxNodesCount)
        {
          //wtf?
          newClnt->Abort();
          delete newClnt;
          continue;
        }
        remoteSockets[remoteSocketsCount]=newClnt;
        mul.add(newClnt);
        remoteSocketsIdx[remoteSocketsCount]=-1;
        remoteSocketsCount++;
        continue;
      }
      int idx=getRemSocketIndex(rd[i]);
      if(idx==-1)continue;
      ReadBuffer::ReadResult rr=rdBuf[idx].read(rd[i]);
      if(rr==ReadBuffer::rrClosed)
      {
        remoteSockets[idx]->Abort();
        mul.remove(err[i]);
        delete remoteSockets[idx];
        remoteSockets[idx]=0;
        continue;
      }
      if(rr==ReadBuffer::rrCompleted)
      {
        processIncomingCommand(idx);
      }
    }
    int didx=0;
    for(int i=0;i<remoteSocketsCount;i++)
    {
      if(didx!=i)
      {
        remoteSockets[didx]=remoteSockets[i];
      }
      if(remoteSockets[i])didx++;
    }
    remoteSocketsCount=didx;
  }
}

void ClusterInterconnect::processIncomingCommand(int idx)
{
  using namespace smsc::sms::BufOps;
  SmsBuffer buf(rdBuf[idx].buf,rdBuf[idx].dataSize);
  uint32_t cmdId;
  buf>>cmdId>>cmdId;
  if(cmdId==cciIdent)
  {
    uint8_t remIdx;
    buf>>remIdx;
    remoteSocketsIdx[idx]=remIdx;
    smsc_log_debug(log,"received node ident command for node %d",remIdx);
  }else if(cmdId==cciSubmit)
  {
    smsc::sms::SMS sms;
    uint32_t dlgId;
    buf>>dlgId;
    char smeIdBuf[64];
    SmallCharBuf scb(smeIdBuf,sizeof(smeIdBuf));
    buf>>scb;
    smsc::sms::Deserialize(buf,sms,0x10001);
    smsc::smeman::SmscCommand cmd=smsc::smeman::SmscCommand::makeSumbmitSm(sms,dlgId);
    cmd->sourceId=(char*)smeIdBuf;
    cmd->dstNodeIdx=remoteSocketsIdx[idx];
    smsc_log_debug(log,"received submit command from node %d sourceId=%s",cmd->dstNodeIdx,cmd->sourceId.c_str());
    sync::MutexGuard mg(inMon);
    inQueue.Push(cmd);
    if(inQueue.Count()==1)
    {
      inMon.notify();
    }
  }else if(cmdId==cciSubmitResp)
  {
    char msgIdBuf[64];
    SmallCharBuf scb(msgIdBuf,sizeof(msgIdBuf));
    char smeIdBuf[64];
    uint32_t dlgId;
    uint32_t status;
    uint8_t dataSm;
    buf>>scb;
    scb.buf=smeIdBuf;
    scb.len=sizeof(smeIdBuf);
    buf>>scb>>dlgId>>status>>dataSm;
    smsc_log_debug(log,"received submit_resp(%d,%d) command from node %d for sme %s",dlgId,status,remoteSocketsIdx[idx],smeIdBuf);

    smsc::smeman::SmscCommand cmd=smsc::smeman::SmscCommand::makeSubmitSmResp(msgIdBuf,dlgId,status,dataSm);
    cmd->sourceId=(const char*)smeIdBuf;

    sync::MutexGuard mg(inMon);
    inQueue.Push(cmd);
    if(inQueue.Count()==1)
    {
      inMon.notify();
    }
  }
  rdBuf[idx].reset();
}

void ClusterInterconnect::prepareOutCommand(smsc::smeman::SmscCommand& cmd)
{
  using namespace smsc::sms::BufOps;
  SmsBuffer buf;
  uint32_t len=0;
  buf<<len;
  if(cmd->get_commandId()==smsc::smeman::CLUSTERICONIDENT)
  {
    buf<<(uint32_t)cciIdent;
    buf<<(uint8_t)cmd->get_dialogId();
    smsc_log_debug(log,"send ident to node %d",cmd->dstNodeIdx);
  }else if(cmd->get_commandId()==smsc::smeman::SUBMIT)
  {
    smsc_log_debug(log,"send submit command from '%s' to node %d",cmd->sourceId.c_str(),cmd->dstNodeIdx);
    buf<<(uint32_t)cciSubmit;
    buf<<cmd->get_dialogId();
    buf<<cmd->sourceId.c_str();
    smsc::sms::Serialize(*cmd->get_sms(),buf);
  }else if(cmd->get_commandId()==smsc::smeman::SUBMIT_RESP)
  {
    smsc_log_debug(log,"send submit_resp(%d,%d) command for '%s' to node %d",cmd->get_dialogId(),cmd->get_resp()->get_status(),cmd->sourceId.c_str(),cmd->dstNodeIdx);
    buf<<(uint32_t)cciSubmitResp;
    buf<<cmd->get_resp()->get_messageId();
    buf<<cmd->sourceId.c_str();
    buf<<cmd->get_dialogId();
    buf<<cmd->get_resp()->get_status();
    buf<<(uint8_t)cmd->get_resp()->get_dataSm();
  }
  len=htonl((uint32_t)buf.GetPos());
  memcpy(buf.get(),&len,4);
  wrBuf[cmd->dstNodeIdx].assign(buf.get(),buf.GetPos());
}

void ClusterInterconnect::writeThread()
{
  net::Multiplexer mul;
  net::Multiplexer::SockArray wr,err;
  while(!shutdown)
  {
    {
      sync::MutexGuard mg(outMon);
      while(!shutdown && mul.getSize()==0 && outQueue.Count()==0)
      {
        outMon.wait(200);
      }
      if(outQueue.Count()>0)
      {
        smsc::smeman::SmscCommand& cmd=outQueue.Front();
        if(!connectedClnt[cmd->dstNodeIdx])
        {
          smsc_log_debug(log,"node %d isn't connected. skipping command.",cmd->dstNodeIdx);
          if(cmd->get_commandId()==smsc::smeman::SUBMIT)
          {
            smsc::smeman::SmscCommand resp=smsc::smeman::SmscCommand::makeSubmitSmResp("",cmd->get_dialogId(),smsc::Status::RX_T_APPN,cmd->get_sms_and_forget()->getIntProperty(smsc::sms::Tag::SMPP_DATA_SM));
            sync::MutexGuard mg2(inMon);
            inQueue.Push(resp);
            if(inQueue.Count()==1)
            {
              inMon.notify();
            }
          }
          outQueue.Pop();
          continue;
        }
        if(!mul.exists(&clntSockets[cmd->dstNodeIdx]))
        {
          prepareOutCommand(cmd);
          mul.add(&clntSockets[cmd->dstNodeIdx]);
          outQueue.Pop();
        }
      }
      /*
      for(int i=0;i<nodesCount;i++)
      {
        if(i==ownNodeIndex)continue;
        if(wrBuf[i].haveData() && !mul.exists(&clntSockets[i]))
        {
          smsc_log_debug(log,"node %d have data.",i);
          mul.add(&clntSockets[i]);
        }
      }
      */
      if(mul.getSize()==0)
      {
        continue;
      }
    }
    if(!mul.canWrite(wr,err,200))continue;
    for(int i=0;i<err.Count();i++)
    {
      int idx=getClntSocketIndex(err[i]);
      clntSockets[idx].Abort();
      connectedClnt[idx]=false;
      mul.remove(&clntSockets[idx]);
    }
    for(int i=0;i<wr.Count();i++)
    {
      int idx=getClntSocketIndex(wr[i]);
      smsc_log_debug(log,"send data to %d",idx);
      WriteBuffer::WriteResult wres=wrBuf[idx].write(wr[i]);
      if(wres==WriteBuffer::wrError)
      {
        mul.remove(&clntSockets[idx]);
        clntSockets[i].Abort();
        connectedClnt[i]=false;
      }else if(wres==WriteBuffer::wrComplete)
      {
        mul.remove(&clntSockets[idx]);
      }
    }
  }
}


}
}
