#include "AdminSocketManager.h"

#include <admin/util/ShutdownableList.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::util::ShutdownableList;

ServiceSocketListener * AdminSocketManager::listener = 0;

void AdminSocketManager::start(const char * const host,
															 const in_port_t port,
															 const char * const debugCategory = "smsc.admin.service.SocketListener")
	throw (AdminException)
{
	if (listener != 0)
		throw AdminException("Admin socket managaer already started");
	listener = new ServiceSocketListener(host, port, debugCategory);
	listener->Start();
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
	delete listener;
	listener = 0;
}

}
}
}
