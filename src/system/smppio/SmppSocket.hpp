#ifndef __SYSTEM_SMPPIO_SMPPSOCKET_HPP__
#define __SYSTEM_SMPPIO_SMPPSOCKET_HPP__

#include "core/network/Socket.hpp"
#include "smpp/smpp.h"
#include "system/smppio/SmppIOTask.hpp"
//#include "system/smppio/SmppProxy.hpp"

namespace smsc{
namespace system{
namespace smppio{

enum{
  ssModeRead,
  ssModeWrite,
};

#define SSOCK_PACKET_SIZE 1024
#define SSOCK_INIT_READ_BUFFER 256

using smsc::core::network::Socket;

class SmppProxy;
class SmppIOTask;

class SmppSocket{
public:
  SmppSocket(int sockmode,Socket* sock):
    mode(sockmode),
    socket(sock)
  {
    if(sockmode==ssModeRead)
    {
      buffer=new char[SSOCK_INIT_READ_BUFFER];
      bufferSize=SSOCK_INIT_READ_BUFFER;
      bufferOffset=0;
    }else
    {
      buffer=NULL;
      bufferSize=0;
      bufferOffset=0;
    }
  }
  void assignTasks(SmppIOTask *in,SmppIOTask *out)
  {
    inThread=in;
    outThread=out;
  }
  void assignProxy(SmppProxy *p){proxy=p;}
  SmppProxy* getProxy(){return proxy;}
  int receive();
  smsc::smpp::SmppHeader* decode();
  int send();
  int send(char *newbuffer,int newbuffersize);
  Socket* getSocket(){return socket;}

  void notifyOutThread();

protected:
  char* buffer;
  int bufferSize;
  int bufferOffset;
  int mode;
  smsc::core::network::Socket* socket;
  SmppIOTask *inThread,*outThread;
  SmppProxy *proxy;
};//smppioSocket

};//smppio
};//system
};//smsc

#endif
