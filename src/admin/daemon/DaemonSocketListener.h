#ifndef SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER
#define SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER

#include <admin/util/SocketListener.h>
#include <admin/daemon/DaemonCommandDispatcher.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::util::SocketListener;

typedef SocketListener<DaemonCommandDispatcher> DaemonSocketListener;

}
}
}
#endif //ifndef SMSC_ADMIN_DAEMON_DAEMON_SOCKET_LISTENER
