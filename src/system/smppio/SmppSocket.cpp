#ifndef DISABLE_TRACING
//#define DISABLE_TRACING
#endif

#include "system/smppio/SmppSocket.hpp"
#include "system/smppio/SmppProxy.hpp"
#include <string.h>

namespace smsc{
namespace system{
namespace smppio{

char* SmppSocket::getBuffer(int length)
{
  if(length>bufferSize)
  {
    delete buffer;
    buffer=new char[length];
    bufferSize=length;
  }
  return buffer;
}


void SmppSocket::send(int length)
{
  if(log && log->isDebugEnabled())
  {
    std::string s="out:";
    char buf[16];
    for(int i=0;i<bufferOffset;i++)
    {
      sprintf(buf," %02X",(unsigned int)(unsigned char)buffer[i]);
      s+=buf;
    }
    log->log(smsc::logger::Logger::LEVEL_DEBUG,"%s",s.c_str());
  }

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
    __trace__("SmppSocket: read failed");
    return -1;
  }
  bufferOffset+=rd;
  lastUpdate=time(NULL);
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

  if(log && log->isDebugEnabled())
  {
    std::string s="in :";
    char buf[16];
    for(int i=0;i<bufferOffset;i++)
    {
      sprintf(buf," %02X",(unsigned int)(unsigned char)buffer[i]);
      s+=buf;
    }
    log->log(smsc::logger::Logger::LEVEL_DEBUG,"%s",s.c_str());
  }

  smsc::smpp::assignStreamWith(&s,buffer,bufferOffset,true);
  smsc::smpp::SmppHeader* pdu=smsc::smpp::fetchSmppPdu(&s);
  bufferOffset=0;
  return pdu;
}

void SmppSocket::notifyOutThread()
{
  if(outThread)outThread->notify();
}

bool SmppSocket::hasOutput()
{
  return proxy->hasOutput(channelType);
}


}//smppio
}//system
}//smsc
