/*
 * ClientBase.hpp
 *
 *  Created on: Sep 23, 2009
 *      Author: skv
 */

#ifndef __EYELINE_PROTOGEN_CLIENTBASE_HPP__
#define __EYELINE_PROTOGEN_CLIENTBASE_HPP__

#include <string>
#include <memory.h>
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "core/buffers/CyclicQueue.hpp"
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "core/buffers/IntHash.hpp"

namespace eyeline{
namespace protogen{

namespace net=smsc::core::network;
namespace sync=smsc::core::synchronization;
namespace thr=smsc::core::threads;
namespace buf=smsc::core::buffers;

class ClientBase{
protected:
  smsc::logger::Logger* log;
  std::string host;
  int port;
  net::Socket sck;
  bool connected;
  bool stopReq;
  sync::Mutex seqMtx;
  int seqNum;

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

public:
  ClientBase(const char* logName);
  virtual ~ClientBase();
  void Init(const char* host,int port);
  void Stop();

  template <class MSG_T,class PROTO_T>
  void enqueueMessage(const MSG_T& msg,PROTO_T& proto)
  {
    eyeline::protogen::framework::SerializerBuffer buf(1024);
    proto.encodeMessage(msg,&buf);
    Buffer b;
    b.data=buf.detachBuffer();
    b.dataSize=buf.getDataWritten();
    sync::MutexGuard mg(outQueueMon);
    outQueue.Push(b);
    outQueueMon.notify();
  }

  int getNextSeq()
  {
    sync::MutexGuard mg(seqMtx);
    return seqNum++;
  }

  bool isConnected()const
  {
    return connected;
  }

protected:

  void Connect();
  void Disconnect();
  void ReadLoop();
  void WriteLoop();

  virtual void onHandleCommand(Buffer& buf)=0;
  virtual void onConnect()=0;
  virtual void onDisconnect()=0;


  class Reader:public thr::Thread{
  public:
    int Execute();
    ClientBase* disp;
  };
  friend class Reader;

  Reader reader;

  class Writer:public thr::Thread{
  public:
    int Execute();
    ClientBase* disp;
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
    ClientBase* disp;
  };
  friend class CmdHandler;
  thr::ThreadPool tp;
};

}
}

#endif /* ClientBase_H_ */
