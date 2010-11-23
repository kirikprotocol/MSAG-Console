/*
 * NetworkDispatcher.cpp
 *
 *  Created on: Sep 23, 2009
 *      Author: skv
 */

#include "util/config/Manager.h"
#include "util/Exception.hpp"
#include "eyeline/clustercontroller/protocol/Magics.hpp"
#include "protocol/messages/RegisterAsSmsc.hpp"
#include "core/buffers/TmpBuf.hpp"

#include "NetworkDispatcher.hpp"
#include "smsc/configregistry/ConfigRegistry.hpp"

namespace smsc{
namespace cluster{
namespace controller{

NetworkDispatcher* NetworkDispatcher::instance=0;



namespace cfg=smsc::util::config;
namespace proto=smsc::cluster::controller::protocol::messages;

NetworkDispatcher::NetworkDispatcher()
{
  seqNum=1;
  connected=false;
  log=smsc::logger::Logger::getInstance("cc.net");
  logDump=smsc::logger::Logger::getInstance("cp.dump.out");

  stopReq=false;
  smscProto.assignHandler(&ctrlHandler);
  reader.disp=this;
  writer.disp=this;
}

NetworkDispatcher::~NetworkDispatcher()
{
  tp.shutdown();
  reader.WaitFor();
  writer.WaitFor();
}

void NetworkDispatcher::Stop()
{
  stopReq=true;
  sck.Close();
}

void NetworkDispatcher::Shutdown()
{
  delete instance;
}


void NetworkDispatcher::Init(int argNodeIndex,const char* host,int port)
{
  if(instance)
  {
    throw smsc::util::Exception("Duplicate NetworkDispatcher initialization");
  }
  instance=new NetworkDispatcher;
  instance->host=host;
  instance->port=port;
  instance->nodeIndex=argNodeIndex;
  instance->ctrlHandler.Init();
  instance->reader.Start();
  instance->writer.Start();
  instance->enqueueReg();
}

void NetworkDispatcher::enqueueReg()
{
  proto::RegisterAsSmsc msg;
  msg.messageSetSeqNum(getNextSeq());
  msg.setNodeIndex(nodeIndex);
  msg.setMagic(eyeline::clustercontroller::protocol::pmSmsc);
  smsc::configregistry::ConfigRegistry::getInstance()->get(msg.getConfigUpdateTimesRef());
  enqueueMessage(msg);
}

void NetworkDispatcher::Connect()
{
  connected=false;
  if(sck.Init(host.c_str(),port,60)==-1)
  {
    smsc_log_warn(log,"Failed to init socket %s:%d",host.c_str(),port);
    return;
  }
  if(sck.Connect()==-1)
  {
    smsc_log_warn(log,"Failed to connect to %s:%d",host.c_str(),port);
    return;
  }
  connected=true;
}

void NetworkDispatcher::ReadLoop()
{
  uint32_t packetLen;
  buf::TmpBuf<char,2048> buf(0);
  while(!stopReq)
  {
    while(!connected && !stopReq)
    {
      Connect();
      if(!connected)
      {
        sleep(5);
        continue;
      }
    }
    if(sck.ReadAll((char*)(&packetLen), 4) != 4)
    {
      smsc_log_warn(log,"failed to read packetLen");
      Disconnect();
      continue;
    }
    packetLen=htonl(packetLen);
    if(packetLen>16*1024*1024)
    {
      smsc_log_warn(log,"packetLen too big:%u",packetLen);
      Disconnect();
      continue;
    }
    buf.setSize(packetLen);
    if(sck.ReadAll(buf.get(),packetLen)!=packetLen)
    {
      smsc_log_warn(log,"failed to read packet with len %u",packetLen);
      Disconnect();
      continue;
    }
    smsc_log_debug(log,"read packetLen=%u",packetLen);
    CmdHandler* ch=new CmdHandler;
    ch->buf.assign(buf.get(),packetLen);
    ch->disp=this;
    tp.startTask(ch);
  }
}

inline void NetworkDispatcher::Disconnect()
{
  sck.Close();
  connected=false;
  enqueueReg();
}

void NetworkDispatcher::WriteLoop()
{
  while(!stopReq)
  {
    while(!connected && !stopReq)
    {
      sleep(1);
    }
    sync::MutexGuard mg(outQueueMon);
    while(outQueue.Count()==0 && connected && !stopReq)
    {
      outQueueMon.wait(500);
    }
    if(!connected || stopReq || outQueue.Count()==0)
    {
      continue;
    }
    Buffer buf;
    outQueue.Pop(buf);
    if(sck.WriteAll(buf.data,(int)buf.dataSize)!=(int)buf.dataSize)
    {
      smsc_log_warn(log,"Failed to send command");
      smsc_log_debug(log,"dump:%s",buf.dump().c_str());
      Disconnect();
    }
    buf.release();
  }
}

void NetworkDispatcher::HandleCommand(Buffer& buf)
{
  try{
    smsc_log_debug(log,"handlePacket:%s",buf.dump().c_str());
    smscProto.decodeAndHandleMessage(buf.data,buf.dataSize);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"command handling exception:%s",e.what());
  }
}


int NetworkDispatcher::Reader::Execute()
{
  disp->ReadLoop();
  return 0;
}

int NetworkDispatcher::Writer::Execute()
{
  disp->WriteLoop();
  return 0;
}


int NetworkDispatcher::CmdHandler::Execute()
{
  disp->HandleCommand(buf);
  buf.release();
  return 0;
}

}
}
}
