#ifndef SMSC_ADMIN_DAEMON_DAEMONSHUTDOWNHANDLER
#define SMSC_ADMIN_DAEMON_DAEMONSHUTDOWNHANDLER

#include <admin/util/SignalHandler.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::util::SignalHandler;

class DaemonShutdownHandler : public SignalHandler
{
public:
  DaemonShutdownHandler(DaemonSocketListener & daemon_socket_listener)
    : listener(daemon_socket_listener)
  {
  }

  virtual void handleSignal() 
    throw()
  {
    fprintf(stderr, "Stop signal received\n");
    listener.shutdown();
    fprintf(stderr, "Daemon shutdow...\n");
  }

private:
  DaemonSocketListener &listener;
};

}
}
}

#endif //ifndef SMSC_ADMIN_DAEMON_DAEMONSHUTDOWNHANDLER
