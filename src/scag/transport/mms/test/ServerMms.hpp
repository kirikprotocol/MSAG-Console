#ifndef __SCAG_TRANSPORT_MMS_SERVERMMS_HPP__
#define __SCAG_TRANSPORT_MMS_SERVERMMS_HPP__

#include "ClientMms.h"

#include "core/threads/Thread.hpp"

#include <string>
#include <cstring>
#include <string.h>
#include <memory>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/utsname.h>
  
namespace scag { 
namespace transport { 
namespace mms {
namespace test {

using smsc::core::network::Socket;
using smsc::logger::Logger;

class ServerMms: public ClientMms {

public:
  ServerMms(string _host, int _port, string ep_id, bool _is_vasp = true):ClientMms(_host, _port, ep_id, is_vasp) {
    logger = Logger::getInstance("mms.server");
  }
  virtual ~ServerMms() {
    smsc_log_debug(logger, "Stop Server");
  }
  bool start();

protected:
  virtual void work(Socket* sock, size_t con_number);
  void sendResp(Socket* sock, const MmsCommand& cmd);
  void processErrorCommand(Socket* sock, const MmsCommand& command);
};

using smsc::core::threads::Thread;

class MmsThread: public Thread {
public:
  MmsThread(ClientMms* _mms_point, const char* _task_name) {
    mms_point = _mms_point;
    task_name = _task_name;
  }

  virtual int Execute() {
    mms_point->start();
    return 1;
  }
  const char* taskName() {
    return task_name;
  }
private:
  ClientMms* mms_point;
  const char* task_name;
};

}//test
}//mms
}//transport
}//scag

#endif
