#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER_H__

#include "SmeAcceptor.h"
#include "SmscConnector.h"
#include "SmppReader.h"
#include "SmppWriter.h"
#include "core/threads/ThreadPool.hpp"
#include "SmppSMInterface.h"
#include "logger/Logger.h"
#include "SmppCommandQueue.h"
#include "SmppChannelRegistrator.h"

namespace scag{
namespace transport{
namespace smpp{

class SmppSocketManager:public SmppSMInterface{
public:
  SmppSocketManager(SmppChannelRegistrator* argReg,SmppCommandQueue* argQueue):
    reg(argReg),queue(argQueue)
  {
    acc=new SmeAcceptor(this);
    acc->Init("0.0.0.0",8001);
    conn=new SmscConnector(this);
    tp.startTask(acc);
    log=smsc::logger::Logger::getInstance("smpp.sm");
  }
  SmscConnectorAdmin* getSmscConnectorAdmin()
  {
    return conn;
  }
  void registerSocket(SmppSocket* sock);
  void unregisterSocket(SmppSocket* sock);
  void shutdown();
protected:
  enum{MaxSocketsPerThread=16};
  Array<SmppReader*> readers;
  Array<SmppWriter*> writers;
  sync::Mutex mtx;

  SmeAcceptor* acc;
  SmscConnector* conn;

  SmppChannelRegistrator* reg;
  SmppCommandQueue* queue;

  smsc::logger::Logger* log;

  thr::ThreadPool tp;
};


}//smpp
}//transport
}//scag


#endif
