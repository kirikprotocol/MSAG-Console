#ifndef __EYELINE_CLUSTERCONTROLLER_PROTOCOLSOCKET_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROTOCOLSOCKET_HPP__

#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "ConfigLocks.hpp"
#include <string>

namespace eyeline{
namespace clustercontroller{

namespace net=smsc::core::network;
namespace buf=smsc::core::buffers;

enum ConnType{
  ctNone,
  ctWebApp,
  ctSmsc,
  ctLoadBalancer
};

struct Packet{
  const char* data;
  size_t dataSize;
  int connId;
  Packet():data(0)
  {
  }
  Packet(const char* argData,size_t argDataSize,int argConnId):
    data(argData),dataSize(argDataSize),connId(argConnId)
  {

  }

  std::string getDump()
  {
    char buf[32];
    std::string rv;
    sprintf(buf,"[%lu]:",dataSize);
    rv=buf;
    for(size_t i=0;i<dataSize;i++)
    {
      sprintf(buf,"%02x ",(unsigned int)(unsigned char)data[i]);
      rv+=buf;
    }
    return rv;
  }
  void dispose()
  {
    if(data)
    {
      delete [] data;
    }
  }
};


class ProtocolSocket{
public:
  ProtocolSocket(net::Socket* argSck,int argConnId):
  sck(argSck),connId(argConnId),ct(ctNone),havePacketSize(false)
  {
    sck->setData(0,this);
    rdBuffer=new char[128];
    rdBufferSize=128;
    rdDataSize=0;
    wrBuffer=0;
    wrDataWritten=0;
    wrBufferSize=0;
    inOutMul=false;
    dead=false;
    for(int i=0;i<ctConfigsCount;i++)loadedConfigs[i]=false;
  }

  ~ProtocolSocket()
  {
    delete [] rdBuffer;
    delete sck;
  }

  void markConfigAsLoaded(ConfigType argCt)
  {
    loadedConfigs[argCt]=true;
  }

  bool isConfigLoaded(ConfigType argCt)const
  {
    return loadedConfigs[argCt];
  }

  void setConnectType(ConnType argCt)
  {
    ct=argCt;
  }
  ConnType getConnectType()const
  {
    return ct;
  }

  void setNodeIdx(int argNodeIdx)
  {
    nodeIdx=argNodeIdx;
  }
  int getNodeIdx()const
  {
    return nodeIdx;
  }

  net::Socket* getSocket()const
  {
    return sck;
  }
  int getConnId()const
  {
    return connId;
  }

  bool Read();
  bool Write();

  Packet getPacket()const
  {
    char* buf=new char[rdPacketSize];
    memcpy(buf,rdBuffer,rdPacketSize);
    return Packet(buf,rdPacketSize,connId);
  }

  void resetPacket()
  {
    havePacketSize=false;
    rdDataSize=0;
  }

  void setInOutMul(bool value)
  {
    inOutMul=value;
  }

  bool getInOutMul()const
  {
    return inOutMul;
  }

  void markAsDead()
  {
    dead=true;
  }

  bool isDead()const
  {
    return dead;
  }

  void enqueueOutPacket(const Packet& p)
  {
    outQueue.Push(p);
  }

  void setOutPacket(const Packet& p)
  {
    wrBuffer=p.data;
    wrBufferSize=p.dataSize;
    wrDataWritten=0;
  }

protected:
  net::Socket* sck;
  int connId;
  int nodeIdx;
  ConnType ct;
  char* rdBuffer;
  const char* wrBuffer;
  size_t rdBufferSize;
  size_t rdDataSize;
  size_t rdPacketSize;
  size_t wrBufferSize;
  size_t wrDataWritten;
  buf::CyclicQueue<Packet> outQueue;
  bool havePacketSize;
  bool inOutMul;
  bool dead;
  bool loadedConfigs[ctConfigsCount];
};


}
}

#endif
