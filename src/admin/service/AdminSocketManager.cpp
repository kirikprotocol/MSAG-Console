#include "AdminSocketManager.h"

namespace smsc {
namespace admin {
namespace service {

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
	if (listener != 0)
	{
    listener->shutdown();
		delete listener;
    listener = 0;
	}
}

}
}
}
