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
  int towrite=SSOCK_PACKET_SIZE;
  if(bufferOffset+towrite>dataLength)towrite=dataLength-bufferOffset;
  if(towrite==0)return 1;
  int sent=socket->Write(buffer+bufferOffset,towrite);
  if(sent==-1)return -1;
  bufferOffset+=sent;
  if(bufferOffset==dataLength)return 1;
  return 0;
}

int SmppSocket::receive()
{
  int packetsize=-1;
  if(bufferOffset>4)
  {
    packetsize=ntohl(*((int*)buffer));
    if(packetsize>bufferSize)
    {
      bufferSize=packetsize;
      char *newbuf=new char[bufferSize];
      memcpy(newbuf,buffer,bufferOffset);
      delete [] buffer;
      buffer=newbuf;
    }
  }
  int n=bufferSize-bufferOffset;
  n=packetsize>0?(n>packetsize?packetsize:n):4;
  trace2("packetsize:%d, bufferoffset:%d\n",packetsize,bufferOffset);
  int rd=socket->Read(buffer+bufferOffset,n);
  if(rd<=0)return -1;
  bufferOffset+=rd;

  if(bufferOffset==packetsize)return 1;
  return 0;
}

smsc::smpp::SmppHeader* SmppSocket::decode()
{
  smsc::smpp::SmppStream s;
  for(int i=0;i<bufferOffset;i++)printf("%02x ",buffer[i]);
  printf("\n");fflush(stdout);
  smsc::smpp::assignStreamWith(&s,buffer,bufferOffset,true);
  smsc::smpp::SmppHeader* pdu=smsc::smpp::fetchSmppPdu(&s);
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
