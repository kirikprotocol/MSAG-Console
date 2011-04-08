/*
 * NetworkDispatcher.cpp
 *
 *  Created on: Sep 23, 2009
 *      Author: skv
 */

#include "util/Exception.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "ClientBase.hpp"

namespace eyeline{
namespace protogen{

ClientBase::ClientBase(const char* logName)
{
  log=smsc::logger::Logger::getInstance(logName);
  connected=false;
  stopReq=false;
  reader.disp=this;
  writer.disp=this;
  seqNum=0;
}

ClientBase::~ClientBase()
{
  Stop();
  tp.shutdown();
  reader.WaitFor();
  writer.WaitFor();
}

void ClientBase::Stop()
{
  stopReq=true;
  sck.Close();
  tp.shutdown();
  reader.WaitFor();
  writer.WaitFor();
}


void ClientBase::Init(const char* argHost,int argPort)
{
  host=argHost;
  port=argPort;
  reader.Start();
  writer.Start();
}

void ClientBase::Connect()
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

void ClientBase::ReadLoop()
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
      }else
      {
        onConnect();
      }
    }
    if(sck.ReadAll((char*)(&packetLen), 4) != 4)
    {
      smsc_log_warn(log,"failed to read packetLen");
      Disconnect();
      continue;
    }
    packetLen=htonl(packetLen);
    if(packetLen>1024*1024)
    {
      smsc_log_warn(log,"packetLen too big:%u",packetLen);
      Disconnect();
      continue;
    }
    buf.setSize(packetLen);
    if(unsigned(sck.ReadAll(buf.get(),packetLen))!=packetLen)
    {
      smsc_log_warn(log,"failed to read packet with len %u",packetLen);
      Disconnect();
      continue;
    }
    smsc_log_warn(log,"packetLen=%u",packetLen);
    CmdHandler* ch=new CmdHandler;
    ch->buf.assign(buf.get(),packetLen);
    ch->disp=this;
    tp.startTask(ch);
  }
}

void ClientBase::Disconnect()
{
  onDisconnect();
  sck.Close();
  connected=false;
}

void ClientBase::WriteLoop()
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
    smsc_log_debug(log,"sending %d bytes packet",(int)buf.dataSize);
    if(sck.WriteAll(buf.data,(int)buf.dataSize)!=(int)buf.dataSize)
    {
      smsc_log_warn(log,"Failed to send command");
      smsc_log_debug(log,"dump:%s",buf.dump().c_str());
      Disconnect();
    }
    buf.release();
  }
}


int ClientBase::Reader::Execute()
{
  disp->ReadLoop();
  return 0;
}

int ClientBase::Writer::Execute()
{
  disp->WriteLoop();
  return 0;
}


int ClientBase::CmdHandler::Execute()
{
  disp->onHandleCommand(buf);
  buf.release();
  return 0;
}

}
}
