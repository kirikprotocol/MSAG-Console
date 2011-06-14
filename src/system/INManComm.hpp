#ifndef __SMSC_SYSTEM_INMANCOMM_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_SYSTEM_INMANCOMM_HPP__

#include "smeman/smeproxy.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/threads/Thread.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include <algorithm>
#include "sms/sms.h"
#include "smeman/smereg.h"
#include "logger/Logger.h"
#include <map>


namespace smsc{
namespace system{

namespace sync=smsc::core::synchronization;
namespace net=smsc::core::network;
namespace thr=smsc::core::threads;
namespace buf=smsc::core::buffers;

using namespace smsc::sms;

class INManComm:public smsc::smeman::SmeProxy,public smsc::core::threads::ThreadedTask{
public:
  INManComm(smsc::smeman::SmeRegistrar* argSmeReg);

  void Init(const char* host,int port);

  ~INManComm()
  {
    debug1(log,"Calling stop on packetWriter");
    packetWriter.Stop();
    smeReg->unregisterSmeProxy(this);
  }

  //////////

  void ChargeSms(SMSId id,const SMS& sms,smsc::smeman::INSmsChargeResponse::SubmitContext& ctx);
  void ChargeSms(SMSId id,const SMS& sms,smsc::smeman::INFwdSmsChargeResponse::ForwardContext& contex);
  void Report(SMSId id,int dlgId,const SMS& sms,bool final);
  void FullReport(SMSId id,const SMS& sms);

  //////////


  int Execute();

  virtual void close()
  {

  }

  virtual void putCommand(const smsc::smeman::SmscCommand& command)
  {
    //shouldn't be called
  }

  virtual bool getCommand(smsc::smeman::SmscCommand& cmd)
  {
    sync::MutexGuard mg(queueMtx);
    debug1(log,"getCommand");
    return queue.Pop(cmd);
  }

  virtual int getCommandEx(std::vector<smsc::smeman::SmscCommand>& cmds,int& mx,smsc::smeman::SmeProxy* prx)
  {
    sync::MutexGuard mg(queueMtx);
    int n=std::min(mx,queue.Count());
    smsc::smeman::SmscCommand cmd;
    for(int i=0;i<n;i++)
    {
      queue.Pop(cmd);
      cmd.setProxy(prx);
      cmds.push_back(cmd);
    }
    mx=n;
    return queue.Count();
  }

  virtual smsc::smeman::SmeProxyState getState() const
  {
    return smsc::smeman::VALID;
  }

  virtual void init()
  {
  }

  virtual bool hasInput() const
  {
    return false;
  }

  virtual void attachMonitor(smsc::smeman::ProxyMonitor* monitor)
  {
    smeManMon=monitor;
  }

  virtual bool attached()
  {
    return smeManMon!=0;
  }

  virtual uint32_t getNextSequenceNumber()
  {
    return 0;
  }

  const char* getSystemId()const
  {
    return systemId.c_str();
  }

  const char* taskName()
  {
    return "INManComm";
  }

  void stop()
  {
    sync::MutexGuard mg(mon);
    mon.notify();
    isStopping=true;
  }

  int getNewDlgId()
  {
    sync::MutexGuard mg(dlgIdMtx);
    return ++dlgIdSeq;
  }

  static Address scAddr;

protected:
  buf::CyclicQueue<smsc::smeman::SmscCommand> queue;
  sync::Mutex queueMtx;
  smsc::smeman::ProxyMonitor* smeManMon;

  int reqTimeOut;

  int dlgIdSeq;
  sync::Mutex dlgIdMtx;

  smsc::logger::Logger* log;
  smsc::smeman::SmeRegistrar* smeReg;

  struct ReqData;
  friend struct ReqData;
  typedef std::map<int,ReqData*> ReqDataMap;
  typedef std::multimap<time_t,ReqDataMap::iterator> TimeMap;
  struct ReqData{
    SMSId id;
    SMS sms;
    enum{ctSubmit,ctForward};
    TimeMap::iterator tmIt;
    int chargeType;
    union{
      smsc::smeman::INSmsChargeResponse::SubmitContext *sbmCtx;
      smsc::smeman::INFwdSmsChargeResponse::ForwardContext *fwdCtx;
    };
  };

  sync::Mutex reqMtx;
  ReqDataMap reqDataMap;
  TimeMap    timeMap;

  std::string systemId;

  sync::EventMonitor mon;

  struct PacketWriter:public thr::Thread{
    net::Socket* socket;
    bool inUse;
    sync::Mutex mtx;
    net::Socket* newSocket;


    smsc::logger::Logger* log;

    volatile bool isStopping;

    struct Packet{
      char* buf;
      int   len;
    };
    sync::EventMonitor  mon;
    buf::CyclicQueue<Packet> queue;

    void enqueue(const char* buf,int len)
    {
      Packet pkt;
      pkt.buf=new char[len];
      memcpy(pkt.buf,buf,len);
      pkt.len=len;
      sync::MutexGuard mg(mon);
      queue.Push(pkt);
      if(queue.Count()==1)
      {
        mon.notify();
      }
      debug2(log,"queue.Count()=%d",queue.Count());
    }

    PacketWriter()
    {
      log=smsc::logger::Logger::getInstance("pktwrt");
      socket=0;
      newSocket=0;
      inUse=false;
      isStopping=false;
    }

    void assignSocket(net::Socket* argSock)
    {
      sync::MutexGuard mg(mtx);
      info2(log,"assignSocket %p/%p",socket,argSock);
      if(inUse)
      {
        newSocket=argSock;
      }else
      {
        if(socket)
        {
          info2(log,"delete socket %p",socket);
          delete socket;
        }
        socket=argSock;
      }
    }

    void acquireSocket()
    {
      sync::MutexGuard mg(mtx);
      inUse=true;
    }

    void releaseSocket()
    {
      sync::MutexGuard mg(mtx);
      inUse=false;
      if(newSocket)
      {
        if(socket)delete socket;
        socket=newSocket;
        newSocket=0;
      }
    }

    void Stop()
    {
      sync::MutexGuard mg(mon);
      isStopping=true;
      mon.notify();
    }

    int Execute()
    {
      Packet pkt;
      while(!isStopping)
      {
        {
          sync::MutexGuard mg(mon);
          while(!isStopping && queue.Count()==0)
          {
            mon.wait(1000);
          }
          if(!queue.Pop(pkt))continue;
        }
        acquireSocket();
        uint32_t nlen=htonl(pkt.len);
        socket->WriteAll((char*)&nlen,4);
        socket->WriteAll(pkt.buf,pkt.len);
        delete [] pkt.buf;
        releaseSocket();
      }
      return 0;
    }
  };


  std::string host;
  int port;
  net::Socket* socket;
  bool socketOk;
  PacketWriter packetWriter;

  void ProcessExpiration();
};

}//system
}//smsc

#endif
