#ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
#define SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER

/*#include <admin/AdminException.h>
#include <admin/service/CommandDispatcher.h>
#include <admin/service/ServiceCommandHandler.h>
#include <admin/util/SocketListener.h>
#include <core/threads/ThreadPool.hpp>
#include <util/Logger.h>
*/
#include <admin/service/CommandDispatcher.h>
#include <admin/service/ServiceCommandHandler.h>
#include <admin/util/SocketListener.h>
namespace smsc {
namespace admin {
namespace service {

//using smsc::admin::AdminException;
using smsc::admin::util::SocketListener;

typedef SocketListener<smsc::admin::service::CommandDispatcher, ServiceCommandHandler> ServiceSocketListener;
/*class ServiceSocketListener {
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
};*/

}
}
}

#endif // ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
