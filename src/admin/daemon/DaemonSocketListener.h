#ifndef SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER
#define SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER

#include <admin/util/SocketListener.h>
#include <admin/daemon/DaemonCommandDispatcher.h>
#include <admin/daemon/DaemonCommandHandler.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::util::SocketListener;

typedef SocketListener<DaemonCommandDispatcher, DaemonCommandHandler> DaemonSocketListener;

}
}
}
#endif //ifndef SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER
