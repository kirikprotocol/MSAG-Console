#ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
#define SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER

#include <admin/AdminException.h>
#include <admin/service/ServiceCommandHandler.h>
#include <core/threads/ThreadPool.hpp>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

class ServiceSocketListener {
public:
	ServiceSocketListener(unsigned int port,
												ServiceCommandHandler * commandHandler)
			throw(AdminException&);
	void run();
	void shutdown(void);

protected:

private:
	unsigned int port;
	int sock;
	smsc::core::threads::ThreadPool pool;
	log4cpp::Category &logger;
	ServiceCommandHandler * handler;
	bool isShutdownSignaled;
};

}
}
}

#endif // ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
