#ifndef SMSC_ADMIN_SERVICE_ADMIN_SOCKET_MANAGER
#define SMSC_ADMIN_SERVICE_ADMIN_SOCKET_MANAGER

#include <admin/AdminException.h>
#include <admin/service/ServiceSocketListener.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

class _AdminSocketManager
{
public:
	static void start(const char * const host,
										const in_port_t port)
		throw (AdminException);
	
	static void stop() throw ();

	static void WaitFor() throw (AdminException)
	{
		if (listener == 0)
		{
			throw AdminException("Service Socket Listener is not started");
		}
		listener->WaitFor();
	}

protected:
	static ServiceSocketListener * listener;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_ADMIN_SOCKET_MANAGER

