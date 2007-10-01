#ifndef SCAG_TRANSPORT_MMS_ACCEPTOR
#define SCAG_TRANSPORT_MMS_ACCEPTOR


#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"

namespace scag { namespace transport { namespace mms {

using smsc::core::threads::Thread;
using smsc::core::network::Socket;
using smsc::logger::Logger;

class MmsManagerImpl;

class MmsAcceptor : public Thread {
public:
  MmsAcceptor(MmsManagerImpl& m, bool _is_vasp);

  virtual int Execute();
  virtual const char* taskName();

  void shutdown();
  void init(const char *host, int port);

protected:
  MmsManagerImpl &manager;    
  Socket master_socket;
  Logger *logger;

private:
  bool is_stopped;  
  bool is_vasp;
  const char* task_name; 

};

}//mms
}//transport
}//scag 

#endif
