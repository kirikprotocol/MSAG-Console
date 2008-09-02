#ifndef __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPSOCKETMANAGER2_H__

#include "SmeAcceptor2.h"
#include "scag/transport/smpp/base/SmppChannelRegistrator2.h"
#include "scag/transport/smpp/base/SmppCommandQueue2.h"
#include "SmppReader2.h"
#include "scag/transport/smpp/base/SmppSMInterface2.h"
#include "SmppWriter2.h"
#include "SmscConnector2.h"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "scag/config/base/ConfigManager2.h"

namespace scag2 {
namespace transport {
namespace smpp{

    using namespace scag2::config;

class SmppSocketManager:public SmppSMInterface{
public:
  SmppSocketManager(SmppChannelRegistrator* argReg,SmppCommandQueue* argQueue):
    reg(argReg),queue(argQueue)
  {
    acc=new SmeAcceptor(this);
    //acc->Init("0.0.0.0",8001);
    acc->Init(
              ConfigManager::Instance().getConfig()->getString("smpp.host"),
              ConfigManager::Instance().getConfig()->getInt("smpp.port")
    );
    conn=new SmscConnector(this);
    conn->Init(
               ConfigManager::Instance().getConfig()->getString("smpp.host")
    );
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
