#include "AdminSocketManager.h"

#include <admin/util/ShutdownableList.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::util::ShutdownableList;

ServiceSocketListener AdminSocketManager::listener("smsc.admin.service.SocketListener");

void AdminSocketManager::start(const char * const host,
															 const in_port_t port)
	throw (AdminException)
{
	listener.init(host, port);
	listener.Start();
}

void AdminSocketManager::stop()
	throw ()
{
	try
	{
		ShutdownableList::shutdown();
	}
	catch (...)
	{
		// ignoring all exceptions
	}
}

}
}
}
