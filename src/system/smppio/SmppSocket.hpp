#ifndef __SYSTEM_SMPPIO_SMPPSOCKET_HPP__
#define __SYSTEM_SMPPIO_SMPPSOCKET_HPP__

#include "core/network/Socket.hpp"
#include "smpp/smpp.h"
#include "system/smppio/SmppIOTask.hpp"
#include "logger/Logger.h"
#include <string>
//#include "system/smppio/SmppProxy.hpp"

namespace smsc{

namespace smeman{
  class SmscCommand;
}

namespace system{
namespace smppio{

enum{
  ssModeRead,
  ssModeWrite
};

enum{
  ctUnbound=0,
  ctReceiver=1,    //channel type
  ctTransmitter=2,
  ctTransceiver=3
};

#define SSOCK_PACKET_SIZE 1024
#define SSOCK_INIT_READ_BUFFER 256

using smsc::core::network::Socket;

class SmppProxy;
class SmppIOTask;
class SmppSocketsManager;


class SmppSocket{
public:
  SmppSocket(int sockmode,Socket* sock,int timeOut):
    mode(sockmode),
    socket(sock),
    lastEnquireLink(0),
    timeOut(timeOut),
    channelType(ctUnbound)
  {
    proxy=NULL;
    inThread=NULL;
    outThread=NULL;
    if(sockmode==ssModeRead)
    {
      buffer=new char[SSOCK_INIT_READ_BUFFER];
      bufferSize=SSOCK_INIT_READ_BUFFER;
      bufferOffset=0;
    }else
    {
      buffer=NULL;
      bufferSize=0;
      dataLength=0;
      bufferOffset=0;
    }
    log=0;
    connectTime=time(NULL);
  }

  void setSystemId(const char* smeid)
  {
    std::string s="sd.";
    s+=smeid;
    log=smsc::logger::Logger::getInstance(s.c_str());
  }
  virtual ~SmppSocket()
  {
    if(buffer)
    {
      delete [] buffer;
    }
  }
  void assignTasks(SmppIOTask *in,SmppIOTask *out)
  {
    inThread=in;
    outThread=out;
  }
  void assignProxy(SmppProxy *p)volatile{proxy=p;}
  SmppProxy* getProxy()
  {
    return proxy;
  }
  int receive();
  smsc::smpp::SmppHeader* decode();
  bool hasData(){return bufferOffset!=dataLength;}

  int send();
  void send(int length);

  time_t getLastUpdate();

  bool isConnectionTimedOut()
  {
    return bufferOffset>0 && time(NULL)-getLastUpdate()>timeOut;
  }

  void updateLastEL()
  {
    lastEnquireLink=time(NULL);
  }
  time_t getLastEL(){return lastEnquireLink;}

  char* getBuffer(int length);

  Socket* getSocket()volatile{return socket;}

  void notifyOutThread()volatile;

  void setChannelType(int ct)
  {
    channelType=ct;
  }

  int getChannelType(){return channelType;}

  bool getOutgoingCommand(smsc::smeman::SmscCommand& cmd);

  time_t getConnectTime()
  {
    return connectTime;
  }

protected:
  char* buffer;
  int dataLength;
  int bufferSize;
  int bufferOffset;
  int mode;
  smsc::core::network::Socket* socket;
  time_t lastEnquireLink;
  int timeOut;
  SmppIOTask *inThread,*outThread;
  SmppProxy *proxy;
  int channelType;
  smsc::logger::Logger* log;
  time_t connectTime;
};//smppioSocket

struct SmppSocketData{
  int refCount;
  SmppSocketsManager *socketManager;
  SmppSocket* inSocket;
  SmppSocket* outSocket;
  bool inMulti;
  bool outMulti;
  bool killSocket;
  time_t lastUpdateTime;
  SmppSocketData()
  {
    refCount=0;
    socketManager=0;
    inSocket=0;
    outSocket=0;
    inMulti=false;
    outMulti=false;
    killSocket=false;
    lastUpdateTime=time(NULL);
  }
};


}//smppio
}//system
}//smsc

#endif
