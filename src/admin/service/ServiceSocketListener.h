#ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
#define SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER

#include <admin/service/ServiceCommandDispatcher.h>
#include <admin/util/SocketListener.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::util::SocketListener;
using smsc::admin::service::ServiceCommandDispatcher;

typedef SocketListener<ServiceCommandDispatcher> ServiceSocketListener;

}
}
}

#endif // ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
