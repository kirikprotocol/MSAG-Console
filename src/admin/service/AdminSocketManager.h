#ifndef SMSC_ADMIN_SERVICE_ADMIN_SOCKET_MANAGER
#define SMSC_ADMIN_SERVICE_ADMIN_SOCKET_MANAGER

#include <admin/AdminException.h>
#include <admin/service/ServiceSocketListener.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

class AdminSocketManager
{
public:
	static void start(const char * const host,
										const in_port_t port,
										const char * const debugCategory = "smsc.admin.service.SocketListener")
		throw (AdminException);
	
	static void stop() throw ();

protected:
	static ServiceSocketListener * listener;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_ADMIN_SOCKET_MANAGER

