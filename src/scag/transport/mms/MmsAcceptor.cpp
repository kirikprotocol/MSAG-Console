#include "util/Exception.hpp"
#include "MmsAcceptor.h"
#include "MmsContext.h"
#include "Managers.h"

namespace scag { namespace transport { namespace mms {

using smsc::util::Exception;

MmsAcceptor::MmsAcceptor(MmsManagerImpl& m, bool _is_vasp) : manager(m), is_vasp(_is_vasp) {
  task_name = is_vasp ? "VASP_Acceptor" : "RS_Acceptor";
}

int MmsAcceptor::Execute() {
  Socket *user_socket;

  smsc_log_debug(logger, "started");

  for (;;) {
      manager.scags.fitQueueLimit();

      user_socket = master_socket.Accept();

      if (is_stopped)
          break;

      if (!user_socket) {
          smsc_log_error(logger, "failed to accept, error: %s", strerror(errno));
          break;
      }
      unsigned int tid = manager.scags.getNewTransactionId();
      MmsContext *cx = new MmsContext(user_socket, is_vasp, tid);
      smsc_log_info(logger, "accepted: context %p, socket %p", cx, user_socket);
      manager.readers.process(cx);
  }

  if (user_socket)
      delete user_socket;

  smsc_log_debug(logger, "quit");

  return is_stopped == false;
}

const char* MmsAcceptor::taskName() {
  return task_name;
}

void MmsAcceptor::shutdown() {
  is_stopped = true;
  master_socket.Close();
  manager.scags.looseQueueLimit();
  WaitFor();
}

void MmsAcceptor::init(const char *host, int port) {
  is_stopped = false;
  logger = is_vasp ? Logger::getInstance("mms.vasp") : logger = Logger::getInstance("mms.rs");

  try {
    if (master_socket.InitServer(host, port, 0, 0) == -1) {          
      smsc_log_error(logger, "failed to init master socket");
      throw Exception("Socket::InitServer() failed");
    }
    if (master_socket.StartServer() == -1) {
      smsc_log_error(logger, "failed to start master socket");
      throw Exception("Socket::StartServer() failed");
    }
  }
  catch(...) {
    throw;
  }
  Start();
}

}//mms
}//transport
}//scag

