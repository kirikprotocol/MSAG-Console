#include "system/smppio/SmppSocket.hpp"
#include <string.h>

namespace smsc{
namespace system{
namespace smppio{

int SmppSocket::send(char *newbuffer,int newbuffersize)
{
  buffer=newbuffer;
  bufferSize=newbuffersize;
  bufferOffset=0;
  return send();
}

int SmppSocket::send()
{
  int towrite=SSOCK_PACKET_SIZE;
  if(bufferOffset+towrite>bufferSize)towrite=bufferSize-bufferOffset;
  if(towrite==0)return 1;
  int sent=socket->Write(buffer+bufferOffset,towrite);
  if(sent==-1)return -1;
  bufferOffset+=sent;
  if(bufferOffset==bufferSize)return 1;
  return 0;
}

int SmppSocket::receive()
{
  int *packetsize=(int*)buffer;
  if(bufferOffset>4 && *packetsize>bufferSize)
  {
    bufferSize=*packetsize;
    char *newbuf=new char[bufferSize];
    memcpy(newbuf,buffer,bufferOffset);
    delete [] buffer;
    buffer=newbuf;
  }
  int rd=socket->Read(buffer+bufferOffset,bufferSize-bufferOffset);
  if(rd==-1)return -1;
  bufferOffset+=rd;
  if(bufferOffset==*packetsize)return 1;
  return 0;
}

smsc::smpp::SmppHeader* SmppSocket::decode()
{
  smsc::smpp::SmppStream s;
  smsc::smpp::assignStreamWith(&s,buffer,bufferSize,true);
  smsc::smpp::SmppHeader* pdu=smsc::smpp::fetchSmppPdu(&s);
  return pdu;
}

void SmppSocket::notifyOutThread()
{
  outThread->notify();
}


};//smppio
};//system
};//smsc
