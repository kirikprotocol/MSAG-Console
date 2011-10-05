#ifndef DISABLE_TRACING
//#define DISABLE_TRACING
#endif

#include "system/smppio/SmppSocket.hpp"
#include "system/smppio/SmppProxy.hpp"
#include <string.h>
#include "smeman/smsccmd.h"

namespace smsc{
namespace system{
namespace smppio{


using smsc::logger::Logger;

static inline void DumpPduBuffer(Logger* log,const char* prefix,const char* buf)
{
  if(!log)return;
  if(!log->isDebugEnabled())return;
  int32_t sz,id,st,sq;
  memcpy(&sz,buf,4);
  memcpy(&id,buf+4,4);
  memcpy(&st,buf+8,4);
  memcpy(&sq,buf+12,4);
  sz=ntohl(sz);
  id=ntohl(id);
  st=ntohl(st);
  sq=ntohl(sq);
  string res=prefix;
  char tmp[128];
  sprintf(tmp,"sz=%d,id=%x,st=%d,sq=%d:",sz,id,st,sq);
  res+=tmp;
  for(int i=16;i<sz;i++)
  {
    sprintf(tmp," %02X",(unsigned int)(unsigned char)buf[i]);
    res+=tmp;
  }
  log->log(Logger::LEVEL_DEBUG,"%s",res.c_str());
}

char* SmppSocket::getBuffer(int length)
{
  if(length>bufferSize)
  {
    delete [] buffer;
    buffer=new char[length];
    bufferSize=length;
  }
  return buffer;
}

time_t SmppSocket::getLastUpdate()
{
  return ((SmppSocketData*)socket->getData(0))->lastUpdateTime;
}


void SmppSocket::send(int length)
{
  DumpPduBuffer(log,"out:",buffer);

  dataLength=length;
  bufferOffset=0;
}

int SmppSocket::send()
{
  int towrite=dataLength-bufferOffset;
  if(towrite==0)return 1;
  __trace2__("send:%d/%d",towrite,dataLength);
  int sent=socket->Write(buffer+bufferOffset,towrite);
  if(sent==-1)return -1;
  bufferOffset+=sent;
  __trace2__("sent:%d,%d/%d",sent,bufferOffset,dataLength);
  if(bufferOffset==dataLength)return 1;
  return 0;
}

int SmppSocket::receive()
{
  int packetsize=-1;
  if(bufferOffset>=4)
  {
    packetsize=ntohl(*((int*)buffer));
    if(packetsize>70000 || packetsize<16)
    {
      __trace2__("Invalid packet length:%d",packetsize);
      socket->Close();
      return -1;
    }
    if(packetsize>bufferSize)
    {
      bufferSize=packetsize;
      char *newbuf=new char[bufferSize];
      memcpy(newbuf,buffer,bufferOffset);
      delete [] buffer;
      buffer=newbuf;
    }
  }
  int n;
  if(packetsize>0)
  {
    n=packetsize-bufferOffset;
  }else
  {
    n=4;
  }
  trace2("packetsize:%d, bufferoffset:%d, toread:%d",packetsize,bufferOffset,n);
  int rd=socket->Read(buffer+bufferOffset,n);
  if(rd<=0)
  {
    if(log && proxy)
    {
      smsc_log_warn(log,"SmppSocket(%s): read failed %s",proxy->getSystemId(),strerror(errno));
    }else
    {
      __warning2__("SmppSocket: read failed %s",strerror(errno));
    }
    return -1;
  }
  bufferOffset+=rd;
  ((SmppSocketData*)socket->getData(0))->lastUpdateTime=time(NULL);
  if(bufferOffset==packetsize)
  {
    return 1;
  }
  return 0;
}

smsc::smpp::SmppHeader* SmppSocket::decode()
{
  smsc::smpp::SmppStream s;
  //for(int i=0;i<bufferOffset;i++)printf("%02x ",buffer[i]);
  //printf("\n");fflush(stdout);
  trace2("decode: %p, %d",buffer,bufferOffset);

  DumpPduBuffer(log,"in :",buffer);

  smsc::smpp::assignStreamWith(&s,buffer,bufferOffset,true);
  smsc::smpp::SmppHeader* pdu=smsc::smpp::fetchSmppPdu(&s);
  bufferOffset=0;
  return pdu;
}

void SmppSocket::notifyOutThread()volatile
{
  if(outThread)outThread->notify();
}

bool SmppSocket::getOutgoingCommand(smsc::smeman::SmscCommand& cmd)
{
  return proxy->getOutgoingCommand(channelType,cmd);
}


}//smppio
}//system
}//smsc
