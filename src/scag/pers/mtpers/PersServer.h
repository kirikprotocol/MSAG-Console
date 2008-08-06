#ifndef __SCAG_PERS_PVGW_PVGWACCEPTOR_H__
#define __SCAG_PERS_PVGW_PVGWACCEPTOR_H__

#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "ConnectionContext.h"
#include "IOTaskManager.h"

namespace scag { namespace mtpers { 

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

//class PersServer : public Thread {
class PersServer {
public:
    PersServer(IOTaskManager& _iomanager);
    void shutdown();
    void init(const char *host, int port);
    virtual int Execute();
    virtual const char* taskName();

private:
  IOTaskManager& iomanager_;
  Socket masterSocket_;
  Logger *logger;
  bool isStopping_;  
};

}//mtpers
}//scag

#endif

