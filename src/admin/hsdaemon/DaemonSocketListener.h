#ifndef SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER
#define SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER

#include <admin/util/SocketListener.h>
#include <admin/hsdaemon/DaemonCommandDispatcher.h>

namespace smsc {
namespace admin {
namespace hsdaemon {

using smsc::admin::util::SocketListener;

typedef SocketListener<DaemonCommandDispatcher> DaemonSocketListener;

}
}
}
#endif //ifndef SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER
