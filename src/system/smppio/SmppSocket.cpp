#ifndef DISABLE_TRACING
//#define DISABLE_TRACING
#endif

#include "system/smppio/SmppSocket.hpp"
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
  dataLength=length;
  bufferOffset=0;
}

int SmppSocket::send()
{
  int towrite=dataLength-bufferOffset;
  if(towrite==0)return 1;
  __trace2__("send:(%d,%d,%d,%d)%d(%d)",
    (int)buffer[0],(int)buffer[1],(int)buffer[2],(int)buffer[3],towrite,dataLength);
  int sent=socket->Write(buffer+bufferOffset,towrite);
  if(sent==-1)return -1;
  bufferOffset+=sent;
  __trace2__("sent:%d,%d/%d,(%d,%d,%d,%d)",sent,bufferOffset,dataLength,
    (int)buffer[0],(int)buffer[1],(int)buffer[2],(int)buffer[3]);
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
  trace2("packetsize:%d, bufferoffset:%d, toread:%d\n",packetsize,bufferOffset,n);
  int rd=socket->Read(buffer+bufferOffset,n);
  if(rd<=0)
  {
    trace2("SmppSocket: read failed");
    return -1;
  }
  bufferOffset+=rd;

  if(bufferOffset==packetsize)return 1;
  return 0;
}

smsc::smpp::SmppHeader* SmppSocket::decode()
{
  smsc::smpp::SmppStream s;
  //for(int i=0;i<bufferOffset;i++)printf("%02x ",buffer[i]);
  //printf("\n");fflush(stdout);
  trace2("decode: %p, %d\n",buffer,bufferOffset);
  smsc::smpp::assignStreamWith(&s,buffer,bufferOffset,true);
  smsc::smpp::SmppHeader* pdu=smsc::smpp::fetchSmppPdu(&s);
  if(!pdu)
  {
    trace2("failed to decode buffer: %p",buffer);
#ifndef DISABLE_TRACING
    for(int i=0;i<bufferOffset;i++)
    {
      fprintf(stderr,"%02X ",(int)buffer[i]);
    }
    fprintf(stderr,"\n");
#endif
  }
  bufferOffset=0;
  return pdu;
}

void SmppSocket::notifyOutThread()
{
  outThread->notify();
}


};//smppio
};//system
};//smsc
