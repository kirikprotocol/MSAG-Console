#include "AdminSocketManager.h"

#include <admin/util/ShutdownableList.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::util::ShutdownableList;

ServiceSocketListener * AdminSocketManager::listener = 0;

void AdminSocketManager::start(const char * const host,
															 const in_port_t port)
	throw (AdminException)
{
	if (listener != 0)
	{
		throw AdminException("Socket listener already started");
	}
	listener = new ServiceSocketListener("smsc.admin.service.SocketListener");
	listener->init(host, port);
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
	if (listener != 0)
	{
		delete listener;
	}
}

}
}
}
