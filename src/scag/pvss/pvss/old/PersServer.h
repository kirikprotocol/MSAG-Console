#ifndef _SCAG_PVSS_SERVER_PERSSERVER_H_
#define _SCAG_PVSS_SERVER_PERSSERVER_H_

#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "SyncContext.h"
#include "ReaderTaskManager.h"
#include "WriterTaskManager.h"
#include "PersProtocol.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;

class PersServer {
public:
  //PersServer(ReaderTaskManager& readers, WriterTaskManager& writers, PersProtocol& protocol, bool perfCounterOn = false, int perfCounterPeriod = 10);
  PersServer(core::server::ServerCore& server, ReaderTaskManager& readers, WriterTaskManager& writers, PersProtocol& protocol, bool perfCounterOn = false, int perfCounterPeriod = 10);
  void stop();
  bool isStopped();
  void init(const char *host, int port);
  virtual int Execute();
  virtual const char* taskName();

private:
  ReaderTaskManager& readers_;
  core::server::ServerCore& pvssServer_;
  WriterTaskManager& writers_;
  PersProtocol& protocol_;
  Socket masterSocket_;
  Logger *logger;
  bool isStopping_;  
  bool perfCounterOn_;
  int perfCounterPeriod_;
  Mutex mutex_;
};

}//pvss
}//scag2

#endif

