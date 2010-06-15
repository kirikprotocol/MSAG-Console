/*
 * ClusterInterconnect.hpp
 *
 *  Created on: Feb 25, 2010
 *      Author: skv
 */

#ifndef __SMSC_INTERCONNECT_CLUSTERINTERCONNECT_HPP__
#define __SMSC_INTERCONNECT_CLUSTERINTERCONNECT_HPP__

#include "smsc/smeman/smeproxy.h"
#include "core/network/Socket.hpp"
#include "util/Exception.hpp"
#include <string>
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace interconnect{

namespace net=smsc::core::network;
namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;
namespace thr=smsc::core::threads;

enum ClusterCommandId{
  cciIdent=1,
  cciSubmit,
  cciSubmitResp
};

class ClusterInterconnect: public smsc::smeman::SmeProxy
{
protected:
  enum{maxNodesCount=16};
  std::string hosts[maxNodesCount];
  int ports[maxNodesCount];
  int nodesCount;
  int ownNodeIndex;
  net::Socket clntSockets[maxNodesCount];
  bool connectedClnt[maxNodesCount];
  net::Socket* remoteSockets[maxNodesCount];
  int remoteSocketsCount;
  int remoteSocketsIdx[maxNodesCount];
  net::Socket srvSocket;
  buf::CyclicQueue<smsc::smeman::SmscCommand> outQueue;
  sync::EventMonitor outMon;
  buf::CyclicQueue<smsc::smeman::SmscCommand> inQueue;
  mutable sync::EventMonitor inMon;

  struct ReadBuffer{
    ReadBuffer()
    {
      bufSize=1024;
      buf=new char[bufSize];
      dataSize=0;
      packetSize=0;
    }
    ~ReadBuffer()
    {
      delete [] buf;
    }
    char* buf;
    size_t bufSize;
    size_t dataSize;
    size_t packetSize;
    void reset()
    {
      dataSize=0;
      packetSize=0;
    }
    enum ReadResult{
      rrIncomplete,
      rrCompleted,
      rrClosed
    };
    ReadResult read(net::Socket* sck)
    {
      if(packetSize==0)
      {
        dataSize+=sck->Read(buf+dataSize,(int)(4-dataSize));
        if(dataSize<4)
        {
          return rrIncomplete;
        }
        uint32_t sz;
        memcpy(&sz,buf,4);
        packetSize=ntohl(sz);
        if(packetSize>bufSize)
        {
          bufSize=packetSize;
          char* newBuf=new char[bufSize];
          memcpy(newBuf,buf,4);
          delete [] buf;
          buf=newBuf;
        }
        return rrIncomplete;
      }
      int rd=sck->Read(buf+dataSize,(int)(packetSize-dataSize));
      if(rd<=0)
      {
        return rrClosed;
      }
      dataSize+=rd;
      return dataSize==packetSize?rrCompleted:rrIncomplete;
    }
  };

  struct WriteBuffer{
    char* buf;
    size_t bufSize;
    size_t dataSize;
    size_t written;
    WriteBuffer()
    {
      bufSize=1024;
      buf=new char[bufSize];
      dataSize=0;
      written=0;
    }
    enum WriteResult {
      wrIncomplete,
      wrComplete,
      wrError
    };
    WriteResult write(net::Socket* sck)
    {
      int wr=sck->Write(buf+written,(int)(dataSize-written));
      if(wr<0)
      {
        return wrError;
      }
      written+=wr;
      return written==dataSize?wrComplete:wrIncomplete;
    }

    bool haveData()const
    {
      return written==dataSize;
    }

    void reset()
    {
      written=0;
      dataSize=0;
    }

    void assign(const char* argBuf,size_t argSize)
    {
      if(argSize>bufSize)
      {
        delete [] buf;
        bufSize=argSize;
        buf=new char[bufSize];
      }
      memcpy(buf,argBuf,argSize);
      dataSize=argSize;
      written=0;
    }
  };

  ReadBuffer rdBuf[maxNodesCount];
  WriteBuffer wrBuf[maxNodesCount];

  smsc::smeman::ProxyMonitor* monitor;

  smsc::logger::Logger* log;

  sync::Mutex seqMtx;
  int seqCount;

  int getClntSocketIndex(net::Socket* sck);
  int getRemSocketIndex(net::Socket* sck);

  void connectThread();
  void readThread();
  void writeThread();

  void processIncomingCommand(int idx);
  void prepareOutCommand(smsc::smeman::SmscCommand& cmd);

  template <void (ClusterInterconnect::*threadMethod)()>
  class ThreadRunner:public thr::Thread{
  protected:
    ClusterInterconnect* that;
  public:
    ThreadRunner(ClusterInterconnect* argThat):that(argThat)
    {

    }
    int Execute()
    {
      (that->*threadMethod)();
      return 0;
    }
  };


  ThreadRunner<&ClusterInterconnect::connectThread> connThr;
  ThreadRunner<&ClusterInterconnect::readThread> readThr;
  ThreadRunner<&ClusterInterconnect::writeThread> writeThr;

  bool shutdown;

  static ClusterInterconnect* instance;

  void Shutdown();

  ClusterInterconnect(int argOwnNodeIndex);
public:

  virtual ~ClusterInterconnect();

  static void Init(int argOwnIndex);
  static ClusterInterconnect* getInstance();
  static void Deinit();

  void Start();

  void addNode(const std::string& argHost,int argPort)
  {
    if(nodesCount==maxNodesCount)
    {
      throw smsc::util::Exception("Max nodes count(%d) reached",maxNodesCount);
    }
    hosts[nodesCount]=argHost;
    ports[nodesCount]=argPort;
    nodesCount++;
  }

  virtual void close(){}

  virtual void putCommand(const smsc::smeman::SmscCommand& command);
  virtual bool getCommand(smsc::smeman::SmscCommand& cmd);

  virtual int getCommandEx(std::vector<smsc::smeman::SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    smsc::smeman::SmscCommand cmd;
    if(getCommand(cmd))
    {
      cmd.setProxy(prx);
      cmds.push_back(cmd);
      mx=1;
    }else
    {
      mx=0;
    }
    return 0;
  }
  virtual smsc::smeman::SmeProxyState getState() const
  {
    return smsc::smeman::VALID;
  }

  virtual void init() {}
  virtual bool hasInput() const
  {
    sync::MutexGuard mg(inMon);
    return inQueue.Count()!=0;
  }
  virtual void attachMonitor(smsc::smeman::ProxyMonitor* argMonitor);
  virtual bool attached();
  virtual uint32_t getNextSequenceNumber();
  virtual const char * getSystemId() const
  {
    return "CLSTRICON";
  }

};

}
}

#endif /* CLUSTERINTERCONNECT_HPP_ */
