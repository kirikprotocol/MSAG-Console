#ifndef __SME_SME_HPP__
#define __SME_SME_HPP__

#include "core/network/Socket.hpp"
#include "smpp/smpp.h"
#include "core/buffers/Array.hpp"
#include <string>

namespace smsc{
namespace sme{

using namespace smsc::smpp;

struct Buffer{
  char* buffer;
  int size;
  int offset;

  Buffer(){buffer=0;size=0;offset=0;}

  void setSize(int newsize)
  {
    if(newsize<size)return;
    char *newbuf=new char[newsize];
    if(offset)memcpy(newbuf,buffer,offset);
    if(buffer)delete [] buffer;
    buffer=newbuf;
    size=newsize;
  }
  void append(char *mem,int count)
  {
    if(offset+count>size)setSize((offset+count)+(offset+count)/2);
    memcpy(buffer+offset,mem,count);
    offset+=count;
  }
  char* current(){return buffer+offset;}
  int freeSpace(){return size-offset;}
};


class BaseSme{
public:
  BaseSme(const char* host,int port,const char* systemId):
    smscHost(host),smscPort(port),smeSystemId(systemId),
    smppCount(0)
  {
  }
  virtual ~BaseSme(){}
  bool init();
  void bindsme();
  SmppHeader* receiveSmpp(int);

  int getNextSeq(){return smppCount++;}

  bool sendSms(smsc::sme::SMS* sms,int seq);
  bool sendSmpp(SmppHeader* pdu);
  virtual bool processPdu(SmppHeader* pdu){return false;}
  virtual bool processSms(smsc::sms::SMS* sms)=0;

protected:
  std::string smscHost;
  int smscPort;
  std::string smeSystemId;
  smsc::core::network::Socket socket;
  int smppCount;
  Buffer wrbuf;
  Buffer rdbuf;
  bool sendBuffer(const char* buffer,int size);
};

};//sme
};//smsc

#endif
