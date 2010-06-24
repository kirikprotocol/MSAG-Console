/*
 * NetworkDispatcher.h
 *
 *  Created on: Sep 23, 2009
 *      Author: skv
 */

#ifndef __SMSC_CLUSTER_CONTROLLER_NETWORKDISPATCHER_H__
#define __SMSC_CLUSTER_CONTROLLER_NETWORKDISPATCHER_H__

#include <string>
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "core/buffers/CyclicQueue.hpp"
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include <memory.h>
#include "protocol/ControllerSmscProtocolClnt.hpp"
#include "protocol/ControllerHandler.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Condition.hpp"

namespace smsc{
namespace cluster{
namespace controller{

namespace net=smsc::core::network;
namespace sync=smsc::core::synchronization;
namespace thr=smsc::core::threads;
namespace buf=smsc::core::buffers;

class NetworkDispatcher{
protected:
  smsc::logger::Logger* log;
  std::string host;
  int port;
  int nodeIndex;
  net::Socket sck;
  bool connected;
  bool stopReq;

  struct SentMsgInfo{
    sync::Condition cnd;
    int result;
    SentMsgInfo():result(-1){}
  };

  sync::Mutex sendMsgMtx;
  int seqNum;
  buf::IntHash<SentMsgInfo*> sentMsgs;

  smsc::cluster::controller::protocol::ControllerSmscProtocol smscProto;
  smsc::cluster::controller::protocol::ControllerHandler ctrlHandler;


  static NetworkDispatcher* instance;

  struct Buffer{
    Buffer():data(0),dataSize(0){}
    const char* data;
    size_t dataSize;
    void assign(const char* argData,size_t argDataSize)
    {
      if(data)
      {
        delete [] data;
      }
      data=new char[argDataSize];
      memcpy((char*)data,argData,argDataSize);
      dataSize=argDataSize;
    }
    std::string dump()
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
    void release()
    {
      if(data)
      {
        delete [] data;
      }
    }
  };

  buf::CyclicQueue<Buffer> outQueue;
  sync::EventMonitor outQueueMon;

  void enqueueReg();

public:
  NetworkDispatcher();
  ~NetworkDispatcher();
  static NetworkDispatcher& getInstance()
  {
    return *instance;
  }
  static void Init(int argNodeIndex,const char* host,int port);
  static void Shutdown();
  void Stop();
  template <class MSG_T>
  void enqueueMessage(const MSG_T& msg)
  {
    eyeline::protogen::framework::SerializerBuffer buf(1024);
    smscProto.encodeMessage(msg,&buf);
    Buffer b;
    b.data=buf.detachBuffer();
    b.dataSize=buf.getDataWritten();
    smsc_log_debug(log,"enqueue:%s",b.dump().c_str());
    sync::MutexGuard mg(outQueueMon);
    outQueue.Push(b);
    outQueueMon.notify();
  }

  int getNextSeq()
  {
    sync::MutexGuard mg(sendMsgMtx);
    return seqNum++;
  }

  template <class MSG_T>
  int sendMessage(MSG_T& msg)
  {
    sync::MutexGuard mg(sendMsgMtx);
    int seq=seqNum++;
    smsc_log_debug(log,"send msg with seq=%d",seq);
    msg.setSeqNum(seq);
    enqueueMessage(msg);
    SentMsgInfo smi;
    sentMsgs.Insert(seq,&smi);
    sync::TimeSlice ts(30,sync::TimeSlice::tuSecs);
    int rv=smi.cnd.WaitOn(sendMsgMtx,ts);
    sentMsgs.Delete(seq);
    if(rv==ETIMEDOUT)
    {
      return -1;
    }
    return smi.result;
  }

  void notifyOnMessageResp(int seq,int result)
  {
    sync::MutexGuard mg(sendMsgMtx);
    smsc_log_debug(log,"notify msg with seq=%d, result=%d",seq,result);
    SentMsgInfo** smiPtr=sentMsgs.GetPtr(seq);
    if(!smiPtr)
    {
      return;
    }
    (*smiPtr)->result=result;
    (*smiPtr)->cnd.Signal();
  }

  int getNodeIndex()
  {
    return nodeIndex;
  }

protected:

  void Connect();
  void Disconnect();
  void ReadLoop();
  void WriteLoop();

  void HandleCommand(Buffer& buf);


  class Reader:public thr::Thread{
  public:
    int Execute();
    NetworkDispatcher* disp;
  };
  friend class Reader;

  Reader reader;

  class Writer:public thr::Thread{
  public:
    int Execute();
    NetworkDispatcher* disp;
  };
  friend class Writer;

  Writer writer;

  class CmdHandler:public thr::ThreadedTask{
  public:
    int Execute();
    const char* taskName()
    {
      return "cmdhandler";
    }
    Buffer buf;
    NetworkDispatcher* disp;
  };
  friend class CmdHandler;
  thr::ThreadPool tp;
};

}
}
}

#endif /* NETWORKDISPATCHER_H_ */
